# "Players 1-4 automatically respawn when being far enough"

**CT Entry ID:** 354

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(MaxDist, 4)
MaxDist:
  dd (float)640000.0

// When balls exceed 800 X and Z units, they respawn
// back at the arena

newmem:
  cmp dword ptr [esi+0x18], 3
  jg original
  cmp dword ptr [esi+0x18], 0
  jl original

  fld dword ptr [esi+0x164]
  fmul st(0), st(0)
  fld dword ptr [esi+0x16C]
  fmul st(0), st(0)
  faddp
  fcomp dword ptr [MaxDist]
  fnstsw ax
  test ah, 0x01
  jnz original

  push ecx
  mov ecx, esi
  call 00405190
  pop ecx

original:
  mov eax, [esi+0x0c5c]
  jmp Hamsterball.exe+5E28

"Hamsterball.exe"+5E22:
  jmp newmem
  nop

[DISABLE]
"Hamsterball.exe"+5E22:
  db 8B 86 5C 0C 00 00
dealloc(newmem)
dealloc(MaxDist)

```
