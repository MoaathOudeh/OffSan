# OffSan
OffSan is a tool made to detect out of bounds vulnerabilities on the stack. It is built using LLVM (C++) and clang (C). This tool is part of my master's thesis.

## How To Compile OffSan
OffSan is easy to compile. All you need to do is run the `compile.sh` file!\
*Note: OffSan was tested against LLVM14 and CLANG14. If something doesn't work maybe try to use these versions*

## How To Run OffSan
OffSan doesn't run by itself. You need to insert it in the compilation process.\
lets assume we have a file called `code.c` that we want to compile. to compile it with OffSan we need to do the following:
```bash
ROOT_DIR="/path/to/OffSan"
RUNTIME_DIR="$ROOT_DIR/runtime"
PASS_SO="$ROOT_DIR/OffSanPass.so"
clang code.c -g -O0 -fno-omit-frame-pointer \
        -fpass-plugin="$PASS_SO" \
        -L"$RUNTIME_DIR" -loffsan \
        -Wl,-rpath,'$ORIGIN/runtime' \
        -o code.bin
```
Now code.bin is ready to be run!

OffSan has some flags that can be set:\
`OFFSAN_DEBUG`: if this is set to 1 we can see every init and check that OffSan is doing.\
`OFFSAN_CONTINUE`: is this is set to 1 we can continue execution even if an out of bounds was found.

## Tests
in the `tests` folder you can find 50 tests that show the effectivness of OffSan. to compile them, use the code shown above.
