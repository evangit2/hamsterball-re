# "Bumping other balls launches them upwards"

**CT Entry ID:** 344

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)

newmem:
  cmp dword ptr [esi+0x18], 0
  jne skip
  mov eax, [edi+0x1A4]
  test eax, eax
  jz skip
  mov dword ptr [eax+0xCA8], 41A00000  // Y impulse
  mov dword ptr [eax+0xCA4], 0           // X = 0
  mov dword ptr [eax+0xCAC], 0           // Z = 0
skip:
  mov eax, [esi+0x1A4]
  jmp Hamsterball.exe+6DC6

"Hamsterball.exe"+6DC0:
  jmp newmem
  nop

[DISABLE]
"Hamsterball.exe"+6DC0:
  db 8B 86 A4 01 00 00
dealloc(newmem)

```
