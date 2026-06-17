# Agent Knowledge Package - Hamsterball Reverse Engineering

This folder is a self-contained bootstrapping guide for any future agent that needs to reproduce or continue the Hamsterball RE work.

Start here and read the numbered sections in order. Each file is designed to be actionable on its own.

| # | File | Purpose |
|---|------|---------|
| 1 | [01-BOOTSTRAP.md](01-BOOTSTRAP.md) | Set up workspace, acquire assets, run first binary analysis |
| 2 | [02-GHIDRA-SETUP.md](02-GHIDRA-SETUP.md) | Import binary, start GhidraMCP headless, verify server |
| 3 | [03-RENAME-RESTORE.md](03-RENAME-RESTORE.md) | Import the 975+ function renames from FUNCTION_MAP.md |
| 4 | [04-DECOMP-WORKFLOW.md](04-DECOMP-WORKFLOW.md) | How to decompile, clean, and verify code faithfully |
| 5 | [05-STRUCT-VERIFICATION.md](05-STRUCT-VERIFICATION.md) | Methodology for confirming struct offsets |
| 6 | [06-MODDING-PATTERNS.md](06-MODDING-PATTERNS.md) | Common game RE patterns (dead code, vtables, level gating) |
| 7 | [07-REIMPL-LESSONS.md](07-REIMPL-LESSONS.md) | All lessons learned from reimplementation attempts |
| 8 | [08-TROUBLESHOOTING.md](08-TROUBLESHOOTING.md) | Known failure modes and workarounds |
| 9 | [scripts/](scripts/) | Reusable Python helpers |
| 10 | [templates/](templates/) | Copy-paste decompile/verify script templates |

## External Authorities

- The repo's `docs/FUNCTION_MAP.md` is the source of truth for named functions.
- The repo's `analysis/ghidra/structs/*.h` contain C struct definitions exported from Ghidra.
- `docs/STRUCTS_AND_TYPES.md` is the human-facing struct reference (but verify offsets with raw decompilation before trusting).

## First Command Checklist

```bash
cd ~/hamsterball-re
file originals/installed/extracted/Hamsterball.exe
python3 docs/agent-knowledge/scripts/check_server.py
```
