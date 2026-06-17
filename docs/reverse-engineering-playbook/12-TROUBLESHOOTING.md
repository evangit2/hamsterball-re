# 12 - Troubleshooting Common Dead Ends

## Decompilation Looks Nonsensical

- Check that the binary base is correct.
- Check that you are analyzing the right code (thumb vs ARM, 16-bit vs 32-bit).
- Look at raw disassembly alongside decompilation.

## Function Has No Xrefs

- It may be reached through a vtable.
- It may be an export or callback.
- It may be dead code.

## Strings Do Not Match Behavior

- Strings may be old or unused.
- The same string may have multiple encoding blocks.
- Look for the code that references the string, not the string itself.

## Tool Hangs

- Don't background long-running native tools.
- Use `timeout`.
- For servers, use Hermes `background=true` with health checks.

## Dynamic Analysis Crashes

- Verify calling convention.
- Verify stack alignment.
- Check for anti-debug (TLS callbacks, timing checks, debug registers).
