# d3d8_proxy_logger — MinGW cross-compile Makefile
# Builds d3d8.dll that intercepts Direct3DCreate8, logs all calls, forwards to real d3d8

CXX = i686-w64-mingw32-g++
SRC_DIR = src
BUILD_DIR = .

SOURCES = \
    $(SRC_DIR)/d3d8_proxy_helpers.cpp \
    $(SRC_DIR)/d3d8_proxy_d3d8.cpp \
    $(SRC_DIR)/d3d8_proxy_device.cpp \
    $(SRC_DIR)/d3d8_proxy_resources.cpp

DEF_FILE = $(SRC_DIR)/d3d8_proxy.def
TARGET = $(BUILD_DIR)/d3d8.dll

# MinGW headers provide d3d8.h, but dsound.h needs mmeapi.h (see memory)
# -ld3d8 links against real d3d8.lib (forwarding)
# -static avoids MinGW runtime DLL dependencies
CXXFLAGS = -Wall -Wextra -O2
LDFLAGS = -shared -static-libgcc -static-libstdc++ -static
LIBS = -ld3d8 -luser32 -lgdi32

.PHONY: all clean info

all: $(TARGET)

$(TARGET): $(SOURCES) $(DEF_FILE)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(DEF_FILE) $(LIBS)
	@echo "=== Build: $@ ==="
	@i686-w64-mingw32-objdump -p $@ | grep -A1 "Export Name Table\|Direct3DCreate8"
	@ls -lh $@

info: $(TARGET)
	@echo "=== DLL Exports ==="
	@i686-w64-mingw32-objdump -p $(TARGET) | grep Direct3DCreate8
	@echo ""
	@echo "=== File Info ==="
	@file $(TARGET)
	@echo ""
	@echo "=== Dependencies ==="
	@i686-w64-mingw32-objdump -p $(TARGET) | grep "DLL Name"

clean:
	rm -f $(BUILD_DIR)/d3d8.dll
	rm -f $(BUILD_DIR)/hb_d3d8_trace.jsonl
