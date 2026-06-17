# 01 - Targets and Tools

## Supported Binary Formats

| Format | Extensions | Typical Tools |
|--------|-----------|---------------|
| PE32/PE32+ | .exe, .dll | pefile, Ghidra, x64dbg |
| ELF | (none), .so | readelf, Ghidra, objdump |
| Mach-O | .app, .dylib | otool, Ghidra, radare2 |
| Raw/Firmware | .bin | binwalk, Ghidra, IDA |

## Analysis Types

**Static analysis:** No code execution. Safe and repeatable.
- File headers, strings, imports/exports, disassembly, decompilation.

**Dynamic analysis:** Code runs under observation.
- Debugging, hooking, memory dumps, API tracing.

## Tool List

| Task | Tool |
|------|------|
| Disassemble/decompile | Ghidra, IDA, radare2, Binary Ninja |
| PE inspection | pefile, Detect It Easy, CFF Explorer |
| Strings | `strings` |
| Hex/entropy | 010 Editor, ImHex, binwalk |
| Debugger | x64dbg, WinDbg, gdb, LLDB |
| API tracing | API Monitor, Process Monitor, frida |
| Network | Wireshark, tcpdump |
