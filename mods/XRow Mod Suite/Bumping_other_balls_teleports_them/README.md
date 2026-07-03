# "Bumping other balls teleports them"

**CT Entry ID:** 343

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)

newmem:
  cmp dword ptr [esi+0x18], 0
  jne skip

  // Compute direction from ball1 to ball2
  fld dword ptr [edi+0x164]  // ball2.x
  fsub dword ptr [esi+0x164] // ball2.x - ball1.x
  fmul dword ptr [Hamsterball.exe+CF3E8]  // * 6.0
  fadd dword ptr [edi+0x164]
  fstp dword ptr [edi+0x164]

  fld dword ptr [edi+0x16C]  // ball2.z
  fsub dword ptr [esi+0x16C] // ball2.z - ball1.z
  fmul dword ptr [Hamsterball.exe+CF3E8]  // * 6.0
  fadd dword ptr [edi+0x16C]
  fstp dword ptr [edi+0x16C]

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
