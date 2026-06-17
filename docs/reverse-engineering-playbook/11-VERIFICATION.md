# 11 - Verification

## Every Claim Needs Evidence

| Claim | Required Evidence |
|-------|-------------------|
| Function name | Decompilation or direct xrefs |
| Struct size | Constructor `operator_new(size)` |
| Field offset | Raw C statement `(this + N)` or `(this)[N*4]` |
| Field meaning | Observed values and read/write context |
| Calling convention | Prologue/disassembly |
| Vtable slot | Parsed vtable array at known address |

## Falsification

Actively look for evidence against your hypothesis:
- Search other functions for the same offset.
- Check if a setter's field is ever read.
- Check if a flag is overwritten every frame.

## Peer-Review Checklist

Before committing a finding:
1. Can another agent reproduce the address from the doc?
2. Is the binary hash stated?
3. Are confidence markers applied to every claim?
4. Are wrong guesses removed rather than left commented out?
