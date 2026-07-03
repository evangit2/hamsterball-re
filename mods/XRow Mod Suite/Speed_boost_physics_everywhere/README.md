# "Speed boost physics everywhere"

**CT Entry ID:** 398

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(globalBoostTimer, 4)
alloc(globalBoostInterval, 4)
alloc(processedThisFrame, 4)
alloc(BoostMult, 4)
registersymbol(globalBoostTimer)
registersymbol(globalBoostInterval)
registersymbol(BoostMult)

globalBoostTimer:
  dd 0
globalBoostInterval:
  dd 60
processedThisFrame:
  dd 0
BoostMult:
  dd (float)3.0

newmem:
  push eax
  push ecx
  push edx

  mov eax, [esi+0x18]
  cmp eax, 0
  je check_boost
  cmp eax, 1
  je check_boost
  cmp eax, 2
  je check_boost
  cmp eax, 3
  je check_boost
  cmp eax, -1
  je check_boost
  jmp done

check_boost:
  cmp dword ptr [processedThisFrame], 1
  je apply_boost

  cmp dword ptr [globalBoostTimer], 0
  jle trigger_now
  dec dword ptr [globalBoostTimer]
  jmp done

trigger_now:
  mov dword ptr [processedThisFrame], 1
  mov eax, [globalBoostInterval]
  mov [globalBoostTimer], eax

apply_boost:
  push eax
  mov eax, [esi+0x1A4]
  test eax, eax
  jz skip_boost
  fld dword ptr [eax+0xCA4]
  fmul dword ptr [BoostMult]
  fstp dword ptr [eax+0xCA4]
  fld dword ptr [eax+0xCAC]
  fmul dword ptr [BoostMult]
  fstp dword ptr [eax+0xCAC]
skip_boost:
  pop eax
  // Reset after boosting - only one entity per trigger
  mov dword ptr [processedThisFrame], 0

done:
  pop edx
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
unregistersymbol(globalBoostTimer)
unregistersymbol(globalBoostInterval)
unregistersymbol(BoostMult)
dealloc(newmem)
dealloc(globalBoostTimer)
dealloc(globalBoostInterval)
dealloc(processedThisFrame)
dealloc(BoostMult)

```
