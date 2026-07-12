# "Aggressive camera rotation (Dizzy Warning)"

**CT Entry ID:** 403

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(camAngle, 4)

camAngle:
  dd (float)0.0

newmem:
  push eax
  push ecx

  mov ecx, [esi+0x14]
  test ecx, ecx
  jz done

  fld dword ptr [camAngle]
  fadd dword ptr [rotSpeed]
  fst dword ptr [camAngle]

  // 0x29BC = 135 + sin(angle) * range
  fsin
  fmul dword ptr [range]
  fadd dword ptr [baseBC]
  fstp dword ptr [ecx+0x29BC]

  // 0x29C0 = 1100 + cos(angle) * range
  fld dword ptr [camAngle]
  fcos
  fmul dword ptr [range]
  fadd dword ptr [baseC0]
  fstp dword ptr [ecx+0x29C0]

done:
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

rotSpeed:
  dd (float)0.02
baseBC:
  dd (float)135.0
baseC0:
  dd (float)1100.0
range:
  dd (float)200.0

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)
dealloc(camAngle)

```
