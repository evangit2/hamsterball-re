# "Beginner Race bumper speed"

**CT Entry ID:** 1

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{ Game   : Hamsterball.exe
  Version:
  Date   : 2026-05-25
  Author : XRow

  Bumper Speed Modifier - Beginner Race
}

[ENABLE]

aobscanmodule(VelocityHook,Hamsterball.exe,8B 54 24 10 8B 44 24 14 8B 4C 24 18 89 17)
alloc(newmem,$1000)
alloc(SpeedMult,4)

SpeedMult:
  dd (float)4.5 // Bumper Speed

label(code)
label(return)

newmem:
  // At this point: [esp+10]=X, [esp+14]=Y, [esp+18]=Z
  // We need to modify them BEFORE they're loaded into registers

  sub esp,10
  // Store X and Z on our new stack space
  fld dword ptr [esp+20]     // X (was esp+10, now esp+20 due to sub esp,10)
  fmul dword ptr [SpeedMult]
  fstp dword ptr [esp+20]

  fld dword ptr [esp+28]     // Z (was esp+18, now esp+28)
  fmul dword ptr [SpeedMult]
  fstp dword ptr [esp+28]

  add esp,10

code:
  mov edx,[esp+10]
  mov eax,[esp+14]
  mov ecx,[esp+18]
  mov [edi],edx
  mov [edi+04],eax
  mov [edi+08],ecx
  jmp return

VelocityHook:
  jmp newmem
  nop
return:
registersymbol(VelocityHook)

[DISABLE]

VelocityHook:
  db 8B 54 24 10 8B 44 24 14 8B 4C 24 18 89 17 89 47 04 89 4F 08

unregistersymbol(VelocityHook)
dealloc(newmem)
dealloc(SpeedMult)

```
