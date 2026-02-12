CC      := clang
CFLAGS  := -O2 -fPIC -Wall -Wextra -fno-omit-frame-pointer -g
LDFLAGS := -shared -ldl -pthread

TARGET  := liboffsan.so
SRC     := liboffsan.c hooks.c
HDR     := liboffsan.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
