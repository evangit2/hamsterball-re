# 09 - Dynamic Analysis

## Debugger Uses

- Set breakpoint on suspected function, observe args.
- Watch memory writes to structs.
- Capture call stack for crashes.
- Trace API calls (CreateFile, LoadLibrary, etc.).

## Hooking

| Method | Best For |
|--------|----------|
| DLL injection | Long-term interception |
| IAT hooking | Replacing imported APIs |
| Inline hooking | Replacing internal functions |
| Frida | Scriptable tracing |

## Safe Hook Checklist

- Match calling convention exactly.
- Preserve registers not used by your hook.
- Do not call CRT allocators from injected threads if the target uses a private heap.
- Log before changing state.
