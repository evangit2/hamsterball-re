# 03 - Ghidra Setup

## Import Any Binary

```bash
/opt/ghidra/support/analyzeHeadless /path/to/project ProjectName -import /path/to/binary -overwrite
```

## Auto-Analysis

Run all default analyzers first. For MSVC binaries, enable:
- Windows PE x86 Propagate External Parameters
- Apply Function Signature Data
- Create Address Tables

## Project Hygiene

- Name the project after the binary version.
- Keep an untampered copy of the original.
- Export renames to git regularly.
- Use commit messages like `analysis: rename rendering functions @ 0x45xxxx`.

## Headless Decompilation

Useful for batch work:

```bash
/opt/ghidra/support/analyzeHeadless /path/to/project ProjectName -scriptPath ./scripts -postScript DecompileByAddress.java 0x1234 /tmp/out.c
```
