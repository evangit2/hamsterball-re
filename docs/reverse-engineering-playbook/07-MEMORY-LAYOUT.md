# 07 - Recovering Structs and Globals

## Struct Recovery Steps

1. Find constructor. Note `operator_new(size)` — that is the struct size.
2. Watch vtable assignment. First field confirms object type.
3. Watch sub-object allocations. `this + N = operator_new(...) + ctor` means pointer to nested object.
4. Watch default writes. They reveal field types.
5. Find update/render function to see which fields are read.

## Globals

| Location | How to find |
|----------|-------------|
| IAT entries | Import table |
| Singletons | Look for static pointer written once in init |
| CRT heap handle | Trace `operator_new` → `HeapAlloc` handle |

## Offset Confidence

- ✅ Raw decompiled C, 2+ functions
- ⚠️ One function only or comment-only
- ❓ Inferred from size/alignment, no direct evidence
