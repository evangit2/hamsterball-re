# "Double 360 Camera Rotation (Cinematic)"

**CT Entry ID:** 20

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

  // Increase angle continuously
  fld dword ptr [camAngle]
  fadd dword ptr [rotSpeed]
  fst dword ptr [camAngle]

  // No wrap needed - sin/cos handle any value

  // 0x29BC = base + cos(angle) * range
  fld st(0)
  fmul dword ptr [degToRad]
  fcos
  fmul dword ptr [range]
  fadd dword ptr [baseBC]
  fstp dword ptr [ecx+0x29BC]

  // 0x29C0 = base + sin(angle) * range
  fmul dword ptr [degToRad]
  fsin
  fmul dword ptr [range]
  fadd dword ptr [baseC0]
  fstp dword ptr [ecx+0x29C0]

done:
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

rotSpeed:
  dd (float)0.2
degToRad:
  dd (float)0.0174533
baseBC:
  dd (float)135.0
baseC0:
  dd (float)1100.0
range:
  dd (float)300.0

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)
dealloc(camAngle)

```
