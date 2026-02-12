#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;

/* ------------------------------- helpers ----------------------------------- */

static std::pair<llvm::Value *, llvm::Value *>
computeBaseAndOffsetBytes(llvm::IRBuilder<> &B,
                          llvm::Value *Ptr,
                          const llvm::DataLayout &DL)
{
  LLVMContext &C = B.getContext();
  Type *I64 = Type::getInt64Ty(C);

  // default: base = Ptr, offset = 0
  Value *Base = Ptr;
  Value *Off = ConstantInt::get(I64, 0);

  // sign-extend an integer to i64 (preserve negative indices)
  auto sext64 = [&](Value *V) -> Value *
  {
    if (V->getType()->isIntegerTy(64))
      return V;
    return B.CreateSExt(V, I64);
  };

  if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr))
  {
    Value *PO = GEP->getPointerOperand();
    Type *SrcTy = GEP->getSourceElementType();

    auto computePtrDiff = [&]()
    {
      Value *BaseI8 = B.CreateBitCast(PO, Type::getInt8PtrTy(C));
      Value *PtrI8 = B.CreateBitCast(Ptr, Type::getInt8PtrTy(C));
      Off = B.CreateSub(B.CreatePtrToInt(PtrI8, I64),
                        B.CreatePtrToInt(BaseI8, I64));
      Base = PO;
    };

    // Case A: GEP on i8* with one index => byte offset = idx
    if (SrcTy->isIntegerTy(8) && GEP->getNumIndices() == 1)
    {
      Base = PO;
      Value *Idx = GEP->getOperand(1);
      Off = sext64(Idx);
    }
    // Case B: GEP on [N x T]* with indices (0, i) => base=&arr[0], off=i*sizeof(T)
    else if (auto *ArrTy = dyn_cast<ArrayType>(SrcTy))
    {
      if (GEP->getNumIndices() >= 2)
      {
        Base = B.CreateInBoundsGEP(ArrTy, PO,
                                   {ConstantInt::get(I64, 0),
                                    ConstantInt::get(I64, 0)});
        Value *Idx = GEP->getOperand(2);
        uint64_t ElemSz = DL.getTypeAllocSize(ArrTy->getElementType());
        Off = B.CreateMul(sext64(Idx), ConstantInt::get(I64, ElemSz));
      }
      else
      {
        computePtrDiff();
      }
    }
    // Case C: GEP on T* with one index i => off = i*sizeof(T)
    else
    {
      auto *PtrTy = dyn_cast<PointerType>(GEP->getPointerOperandType());
      if (PtrTy && GEP->getNumIndices() == 1)
      {
        Base = PO;
        Value *Idx = GEP->getOperand(1);
        uint64_t ElemSz = DL.getTypeAllocSize(SrcTy);
        Off = B.CreateMul(sext64(Idx), ConstantInt::get(I64, ElemSz));
      }
      else
      {
        computePtrDiff();
      }
    }
  }

  if (Base->getType() != Type::getInt8PtrTy(B.getContext()))
    Base = B.CreateBitCast(Base, Type::getInt8PtrTy(B.getContext()));
  return {Base, Off};
}

static inline ConstantInt *constI64(LLVMContext &C, uint64_t V)
{
  return ConstantInt::get(Type::getInt64Ty(C), V);
}

static inline Value *toI64(IRBuilder<> &B, Value *V, bool signExtend = false)
{
  auto *I64 = Type::getInt64Ty(B.getContext());
  if (V->getType()->isIntegerTy(64))
    return V;
  return signExtend ? B.CreateSExt(V, I64) : B.CreateZExt(V, I64);
}

static void emitInitForAlloca(IRBuilder<> &B,
                              AllocaInst *AI,
                              const DataLayout &DL,
                              FunctionCallee initFn)
{
  LLVMContext &Ctx = B.getContext();
  Type *AllocTy = AI->getAllocatedType();
  Value *TotalSize = nullptr;

  if (AI->isArrayAllocation())
  {
    // Dynamic/VLA: alloca T, iN %count -> T[count]
    Value *Count = AI->getArraySize();
    uint64_t Elem = DL.getTypeAllocSize(AllocTy);
    Value *Elem64 = ConstantInt::get(Type::getInt64Ty(Ctx), Elem);
    Value *Cnt64 = Count->getType()->isIntegerTy(64)
                       ? Count
                       : B.CreateZExt(Count, Type::getInt64Ty(Ctx));
    TotalSize = B.CreateMul(Cnt64, Elem64);
  }
  else if (auto *ArrTy = dyn_cast<ArrayType>(AllocTy))
  {
    // Fixed array: alloca [N x T]
    uint64_t N = ArrTy->getNumElements();
    uint64_t E = DL.getTypeAllocSize(ArrTy->getElementType());
    TotalSize = ConstantInt::get(Type::getInt64Ty(Ctx), N * E);
  }
  else
  {
    // Scalars (pointers, structs, etc.)
    uint64_t S = DL.getTypeAllocSize(AllocTy);
    TotalSize = ConstantInt::get(Type::getInt64Ty(Ctx), S);
  }

  Value *AddrI8 = B.CreateBitCast(AI, Type::getInt8PtrTy(Ctx));
  B.CreateCall(initFn, {AddrI8, TotalSize});
}

static void emitChecked(IRBuilder<> &B,
                        Value *Ptr,
                        Value *AccessSize,
                        const DataLayout &DL,
                        FunctionCallee CheckFn)
{
  auto [BaseI8, Off] = computeBaseAndOffsetBytes(B, Ptr, DL);
  Value *Sz64 = toI64(B, AccessSize, false);
  Value *Off64 = Off->getType()->isIntegerTy(64) ? Off
                                                 : B.CreateSExt(Off, Type::getInt64Ty(B.getContext()));

  B.CreateCall(CheckFn, {BaseI8, Off64, Sz64});
}

static bool isWholeAllocaZeroInit(const MemSetInst &MSI,
                                  const DataLayout &DL)
{
  auto *LenCI = dyn_cast<ConstantInt>(MSI.getLength());
  auto *ValCI = dyn_cast<ConstantInt>(MSI.getValue());
  if (!LenCI || !ValCI || !ValCI->isZero())
    return false;

  uint64_t LenVal = LenCI->getZExtValue();
  Value *Underlying = MSI.getDest()->stripPointerCasts();
  auto *AI = dyn_cast<AllocaInst>(Underlying);
  if (!AI)
    return false;

  uint64_t AllocSz = DL.getTypeAllocSize(AI->getAllocatedType());
  return LenVal == AllocSz;
}

static bool isAggregateMemset(const MemSetInst &MSI, const DataLayout &DL)
{
  auto *LenCI = dyn_cast<ConstantInt>(MSI.getLength());
  if (!LenCI)
    return false;

  Value *DestBase = MSI.getDest()->stripPointerCasts();
  auto *PtrTy = dyn_cast<PointerType>(DestBase->getType());
  if (!PtrTy)
    return false;

  Type *EltTy = PtrTy->getElementType();
  if (!EltTy || (!isa<StructType>(EltTy) && !isa<ArrayType>(EltTy)))
    return false;

  uint64_t AggSize = DL.getTypeStoreSize(EltTy);
  return AggSize == LenCI->getZExtValue();
}

static bool isAggregateMemcpy(const MemTransferInst &MT,
                              const DataLayout &DL)
{
  auto *LenCI = dyn_cast<ConstantInt>(MT.getLength());
  if (!LenCI)
    return false;

  Value *DstBase = MT.getDest()->stripPointerCasts();
  Value *SrcBase = MT.getSource()->stripPointerCasts();
  auto *DstPtrTy = dyn_cast<PointerType>(DstBase->getType());
  auto *SrcPtrTy = dyn_cast<PointerType>(SrcBase->getType());
  if (!DstPtrTy || !SrcPtrTy)
    return false;

  Type *DstEltTy = DstPtrTy->getElementType();
  Type *SrcEltTy = SrcPtrTy->getElementType();
  if (DstEltTy != SrcEltTy || (!isa<StructType>(DstEltTy) && !isa<ArrayType>(DstEltTy)))
    return false;

  uint64_t AggSize = DL.getTypeStoreSize(DstEltTy);
  return AggSize == LenCI->getZExtValue();
}

static void instrumentMainArgs(Function &F,
                               const DataLayout &DL,
                               FunctionCallee initFn)
{
  if (F.getName() != "main" || F.arg_size() < 2)
    return;

  auto ArgIt = F.arg_begin();
  Argument *Argc = &*ArgIt++;
  Argument *Argv = &*ArgIt;

  if (!Argc->getType()->isIntegerTy() ||
      !Argv->getType()->isPointerTy() ||
      !Argv->getType()->getPointerElementType()->isPointerTy())
    return;

  IRBuilder<> EntryB(&*F.getEntryBlock().getFirstInsertionPt());
  LLVMContext &C = F.getContext();
  Type *I64 = Type::getInt64Ty(C);

  Value *ArgvI8 = EntryB.CreateBitCast(Argv, Type::getInt8PtrTy(C));
  Value *Argc64 = EntryB.CreateSExtOrTrunc(Argc, I64);

  Value *ArgcPlusOne = EntryB.CreateAdd(Argc64, constI64(C, 1));
  Value *TotalBytes = EntryB.CreateMul(ArgcPlusOne, constI64(C, DL.getPointerSize()));

  EntryB.CreateCall(initFn, {ArgvI8, TotalBytes});
}

static void instrumentMemIntrinsic(MemIntrinsic &MI,
                                   const DataLayout &DL,
                                   FunctionCallee CheckFn)
{
  IRBuilder<> B(&MI);
  Value *Len = nullptr;

  if (auto *MSI = dyn_cast<MemSetInst>(&MI))
  {

    if (isWholeAllocaZeroInit(*MSI, DL) || isAggregateMemset(*MSI, DL))
      return;

    Len = MSI->getLength();
  }
  else if (auto *MT = dyn_cast<MemTransferInst>(&MI))
  {

    if (isAggregateMemcpy(*MT, DL))
      return;

    Len = MT->getLength();
  }
  else
  {
    return;
  }

  emitChecked(B, MI.getDest(), Len, DL, CheckFn);
}

/* --------------------------------- pass ------------------------------------ */

class InitStackVarPass : public PassInfoMixin<InitStackVarPass>
{
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM)
  {
    if (F.isDeclaration())
      return PreservedAnalyses::all();

    Module *M = F.getParent();
    LLVMContext &C = M->getContext();
    const DataLayout &DL = M->getDataLayout();
    Type *I64 = Type::getInt64Ty(C);
    Type *I8Ptr = Type::getInt8PtrTy(C);

    // Declarations
    FunctionCallee initFn =
        M->getOrInsertFunction("initStackVar",
                               FunctionType::get(Type::getVoidTy(C),
                                                 {I8Ptr, I64}, false));

    FunctionCallee CheckFn =
        M->getOrInsertFunction("checkStackAccess",
                               FunctionType::get(Type::getVoidTy(C),
                                                 {I8Ptr, I64, I64},
                                                 false));

    auto Enter = M->getOrInsertFunction("offsan_frame_enter",
                                        FunctionType::get(Type::getVoidTy(C), {}, false));
    auto Exit = M->getOrInsertFunction("offsan_frame_exit",
                                       FunctionType::get(Type::getVoidTy(C), {}, false));
    IRBuilder<> B(&*F.getEntryBlock().getFirstInsertionPt());
    B.CreateCall(Enter, {});
    instrumentMainArgs(F, DL, initFn);

    for (auto &BB : F)
    {
      for (auto &I : make_early_inc_range(BB))
      {

        // 1) Alloca: insert init *after* the alloca
        if (auto *AI = dyn_cast<AllocaInst>(&I))
        {
          if (Instruction *After = AI->getNextNode())
          {
            IRBuilder<> B(After);
            emitInitForAlloca(B, AI, DL, initFn);
          }
          continue;
        }

        // 2) Loads / Stores: check *before* the access
        if (auto *LI = dyn_cast<LoadInst>(&I))
        {
          IRBuilder<> B(LI);
          uint64_t Sz = DL.getTypeStoreSize(LI->getType());

          emitChecked(B, LI->getPointerOperand(), constI64(C, Sz), DL, CheckFn);
          continue;
        }

        if (auto *SI = dyn_cast<StoreInst>(&I))
        {
          IRBuilder<> B(SI);
          uint64_t Sz = DL.getTypeStoreSize(SI->getValueOperand()->getType());

          emitChecked(B, SI->getPointerOperand(), constI64(C, Sz), DL, CheckFn);
          continue;
        }

        // 3) Mem intrinsics: memset, memcpy, memmove
        if (auto *MI = dyn_cast<MemIntrinsic>(&I))
        {
          instrumentMemIntrinsic(*MI, DL, CheckFn);
          continue;
        }
        if (auto *RI = dyn_cast<ReturnInst>(&I))
        {
          IRBuilder<> RB(RI);
          RB.CreateCall(Exit, {});
          continue;
        }
      }
    }

    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }
};

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
llvmGetPassPluginInfo()
{
  return {
      LLVM_PLUGIN_API_VERSION,
      "InitStackVarPass",
      LLVM_VERSION_STRING,
      [](PassBuilder &PB)
      {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>)
            {
              if (Name == "init-stack")
              {
                FPM.addPass(InitStackVarPass());
                return true;
              }
              return false;
            });
        PB.registerPipelineStartEPCallback(
            [](ModulePassManager &MPM, OptimizationLevel Level)
            {
              FunctionPassManager FPM;
              FPM.addPass(InitStackVarPass());
              // Run our function pass over every function in the module
              MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
            });
      }};
}
