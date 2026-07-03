# "Random Speed boost physics everywhere"

**CT Entry ID:** 399

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 2000)
alloc(globalBoostTimer, 4)
alloc(globalBoostInterval, 4)
alloc(processedThisFrame, 4)
alloc(randSeed, 4)
registersymbol(globalBoostTimer)
registersymbol(globalBoostInterval)

globalBoostTimer:
  dd 0
globalBoostInterval:
  dd 60
processedThisFrame:
  dd 0
randSeed:
  dd 1234567

randTable:
  dd (float)3.0
  dd (float)4.0
  dd (float)5.0
  dd (float)7.0
  dd (float)9.0
  dd (float)10.0
  dd (float)12.0
  dd (float)15.0

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
  push eax

  push ebx
  mov ebx, [randSeed]
  mov ecx, ebx
  shl ecx, 13
  xor ebx, ecx
  mov ecx, ebx
  shr ecx, 17
  xor ebx, ecx
  mov ecx, ebx
  shl ecx, 5
  xor ebx, ecx
  mov [randSeed], ebx
  mov eax, ebx
  shr eax, 1
  mov ecx, 8
  xor edx, edx
  div ecx
  mov ebx, edx
  lea ecx, [randTable]
  fld dword ptr [ecx+ebx*4]
  pop ebx

  pop eax
  push eax
  fmul dword ptr [eax+0xCA4]
  fstp dword ptr [eax+0xCA4]

  push ebx
  mov ebx, [randSeed]
  mov ecx, ebx
  shl ecx, 13
  xor ebx, ecx
  mov ecx, ebx
  shr ecx, 17
  xor ebx, ecx
  mov ecx, ebx
  shl ecx, 5
  xor ebx, ecx
  mov [randSeed], ebx
  mov eax, ebx
  shr eax, 1
  mov ecx, 8
  xor edx, edx
  div ecx
  mov ebx, edx
  lea ecx, [randTable]
  fld dword ptr [ecx+ebx*4]
  pop ebx

  pop eax
  fmul dword ptr [eax+0xCAC]
  fstp dword ptr [eax+0xCAC]

skip_boost:
  pop eax
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
dealloc(newmem)
dealloc(globalBoostTimer)
dealloc(globalBoostInterval)
dealloc(processedThisFrame)
dealloc(randSeed)

```
