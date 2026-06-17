# 05 - Decompilation Without Distortion

## The Goal

Produce readable reference code that preserves the original control flow and memory layout.

## Cleaning Rules

- Rename variables.
- Name constants and struct offsets.
- Add section comments.
- Keep the original `if`-chain order.
- Keep the original calling convention.

## Forbidden Cleanups

- Replacing flat string-compare chains with `switch`.
- Changing `__thiscall` free functions into C++ methods with early returns.
- Factoring repeated pointer arithmetic into accessors unless the original did so.
- Inventing types for unverified offsets.
