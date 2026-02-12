LLVM_CONFIG := llvm-config

CXX := clang++
CXXFLAGS := -fPIC $(shell $(LLVM_CONFIG) --cxxflags) -g -w
LDFLAGS := -shared $(shell $(LLVM_CONFIG) --ldflags) $(shell $(LLVM_CONFIG) --system-libs)
LIBS    := $(shell $(LLVM_CONFIG) --libs core)

SRC     := OffSanPass.cpp
TARGET  := OffSanPass.so

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
