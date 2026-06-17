# 06 - Calling Conventions

## x86 32-bit

| Convention | this/args | Stack cleanup | Typical use |
|------------|-----------|---------------|-------------|
| `__cdecl` | All args on stack | Caller | C functions, variadic |
| `__stdcall` | All args on stack | Callee | Win32 APIs |
| `__fastcall` | ECX, EDX, then stack | Callee | Compiler optimization |
| `__thiscall` | ECX=this, args on stack | Callee | MSVC member functions |

## x64

`RCX`, `RDX`, `R8`, `R9` for first four integer/pointer args; XMM0-3 for floats. Caller cleans.

## ARM64

X0-X7 for args. Stack may be used for extras. Link register `X30`/`LR` holds return address.

## Detection in Assembly

| Convention | Prologue tells you |
|------------|------------------|
| `__thiscall` | `MOV ESI, ECX` saves `this` |
| `__stdcall` | `RET 0x8` etc. |
| `__cdecl` | Plain `RET`, caller pops |
