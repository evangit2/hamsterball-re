# 00 - RE Mindset

Reverse engineering is evidence-based reconstruction.

## Core Principles

1. **The binary is the ground truth.** Tools and docs are hypotheses.
2. **Never trust a name without evidence.** Even a pretty function name is just a label.
3. **Work from the known to the unknown.** Start with strings, imports, and entry points; move toward internal logic.
4. **Cross-reference everything.** A struct offset needs at least two decompilations before it is trustworthy.
5. **Write as you go.** A session without notes is a session you will repeat.

## When You Are Stuck

- Look for the constructor. It tells you field sizes and default values.
- Look for the destructor. It tells you nested objects and cleanup order.
- Look for the update loop. It tells you which fields actually matter.
- Look for string comparisons. They reveal state machines and object types.
- Look for magic numbers. `0x3F800000` is 1.0f; `0x40400000` is 3.0f.

## Reproducibility

Record:
- Binary MD5/SHA256
- Tool versions (Ghidra, plugin, OS)
- Exact commands run
- Failed hypotheses, not just successes
