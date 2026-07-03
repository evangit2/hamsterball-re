# "Magnet repulsion physics from one point"

**CT Entry ID:** 406

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(centerX, 4)
alloc(centerY, 4)
alloc(centerZ, 4)

centerX:
  dd (float)0.0
centerY:
  dd (float)0.0
centerZ:
  dd (float)0.0

newmem:
  push eax
  push ecx

  mov eax, [esi+0x1A4]
  test eax, eax
  jz done

  // Push AWAY from center
  fld dword ptr [esi+0x164]
  fsub dword ptr [centerX]
  fmul dword ptr [repelForce]
  fadd dword ptr [eax+0xCA4]
  fstp dword ptr [eax+0xCA4]

  fld dword ptr [esi+0x16C]
  fsub dword ptr [centerZ]
  fmul dword ptr [repelForce]
  fadd dword ptr [eax+0xCAC]
  fstp dword ptr [eax+0xCAC]

done:
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

repelForce:
  dd (float)0.0005

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)
dealloc(centerX)
dealloc(centerY)
dealloc(centerZ)

```
