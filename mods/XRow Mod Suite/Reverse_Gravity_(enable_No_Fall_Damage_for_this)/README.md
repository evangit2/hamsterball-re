# "Reverse Gravity (enable No Fall Damage for this)"

**CT Entry ID:** 402

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
  // Reverse gravity - multiply Y velocity by -1.0
  fld dword ptr [eax+0xCA8]
  fmul dword ptr [gravReverse]
  fstp dword ptr [eax+0xCA8]

done:
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

gravReverse:
  dd (float)-1.0

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)

```
