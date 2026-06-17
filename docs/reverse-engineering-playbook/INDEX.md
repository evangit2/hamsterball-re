# Reverse Engineering Playbook

A program-agnostic guide for analyzing compiled binaries. The Hamsterball project is used as a running example, but the methodology applies to any compiled program (games, applications, malware, firmware).

## Quick Start

| # | File | What it covers |
|---|------|----------------|
| 1 | [00-MINDSET.md](00-MINDSET.md) | How to approach an unknown binary |
| 2 | [01-TARGETS-AND-TOOLS.md](01-TARGETS-AND-TOOLS.md) | PE, ELF, Mach-O; static vs dynamic tools |
| 3 | [02-BINARY-RECON.md](02-BINARY-RECON.md) | Strings, imports, exports, sections, entropy |
| 4 | [03-GHIDRA-SETUP.md](03-GHIDRA-SETUP.md) | Import and auto-analyze any binary |
| 5 | [04-NAMING-STRATEGY.md](04-NAMING-STRATEGY.md) | How to name unknown functions and data |
| 6 | [05-DECOMPILATION.md](05-DECOMPILATION.md) | Reading and cleaning decompiled output without distorting it |
| 7 | [06-CALLING-CONVENTIONS.md](06-CALLING-CONVENTIONS.md) | cdecl, stdcall, fastcall, thiscall, x64, ARM64 |
| 8 | [07-MEMORY-LAYOUT.md](07-MEMORY-LAYOUT.md) | Recovering structs and globals |
| 9 | [08-OOP-PATTERNS.md](08-OOP-PATTERNS.md) | Vtables, RTTI, constructors, virtual dispatch |
| 10 | [09-DYNAMIC-ANALYSIS.md](09-DYNAMIC-ANALYSIS.md) | Debuggers, hooking, tracing |
| 11 | [10-SUBSYSTEM-PATTERNS.md](10-SUBSYSTEM-PATTERNS.md) | Rendering, input, audio, file formats |
| 12 | [11-VERIFICATION.md](11-VERIFICATION.md) | Proving your findings are right |
| 13 | [12-TROUBLESHOOTING.md](12-TROUBLESHOOTING.md) | Common dead ends and how to escape |
| 14 | [case-studies/hamsterball.md](case-studies/hamsterball.md) | How this playbook was applied to Hamsterball |

## Running Example

Throughout this playbook, Hamsterball.exe (PE32, MSVC 2003, D3D8 + DInput8 + BASS) illustrates each step. See the case study for the full project-specific details.
