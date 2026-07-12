# "Ball Collision Mods                                                       -->"

**CT Entry ID:** 345

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{
  Ball Collision Mods

  ---------------------

  Mode 1 (FREEZE TAG):
  Overwrites physics impulse - 2 balls freeze on collision, one gets
  unfrozen when bumped by another ball

  Mode 2 (STRONG COLLISIONS):  Adds to physics impulse - balls get strong force
  on collision

  To toggle modes, change the value of "Mode" below:
    Mode dd 1 = FREEZE TAG
    Mode dd 2 = STRONG COLLISIONS
}

[ENABLE]
alloc(newmem, 1000)
alloc(PushForce, 4)
alloc(Mode, 4)

Mode:
  dd 2 // Modes: 1 for FREEZE TAG, 2 for STRONG COLLISIONS

PushForce:
  dd 41200000

label(returnhere)
label(skip)
label(do_freeze)

newmem:
  mov eax, [edi+0x1A4]
  test eax, eax
  jz skip

  cmp [Mode], 1
  je do_freeze

  fld dword ptr [esp+0x20]
  fmul dword ptr [PushForce]
  fadd dword ptr [eax+0xCA4]
  fstp dword ptr [eax+0xCA4]

  fld dword ptr [esp+0x28]
  fmul dword ptr [PushForce]
  fadd dword ptr [eax+0xCAC]
  fstp dword ptr [eax+0xCAC]
  jmp skip

do_freeze:
  fld dword ptr [esp+0x20]
  fmul dword ptr [PushForce]
  fstp dword ptr [eax+0xCA4]

  fld dword ptr [esp+0x28]
  fmul dword ptr [PushForce]
  fstp dword ptr [eax+0xCAC]

  mov dword ptr [eax+0xCA8], 0

skip:
  push 0x3F800000
  jmp returnhere

aobscanmodule(HookSpot, Hamsterball.exe, 68 00 00 80 3F D9 E0 83 EC 0C)
registersymbol(HookSpot)

HookSpot:
  jmp newmem
returnhere:

[DISABLE]
HookSpot:
  db 68 00 00 80 3F
unregistersymbol(HookSpot)
dealloc(newmem)
dealloc(PushForce)
dealloc(Mode)

```
