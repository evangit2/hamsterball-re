# "Zero Gravity (enable No Fall Damage for this)"

**CT Entry ID:** 401

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)

newmem:
  push eax

  mov eax, [esi+0x1A4]
  cmp eax, 0
  je done
  // Zero out Y velocity (gravity)
  mov dword ptr [eax+0xCA8], 0

done:
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)

```
