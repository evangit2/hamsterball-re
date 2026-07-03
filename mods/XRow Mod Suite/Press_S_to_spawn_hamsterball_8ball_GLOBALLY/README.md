# "Press S to spawn hamsterball 8ball GLOBALLY"

**CT Entry ID:** 161

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(renderPatch, 256)
alloc(SpawnCode, 4096)
alloc(SpawnHB8ball, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(ballCount, 4)
alloc(spawnNow, 4)

registersymbol(SpawnHB8ball)

SpawnHB8ball:
  dd 0

playerX:
  dd 0
playerY:
  dd 0
playerZ:
  dd 0
ballCount:
  dd 0
spawnNow:
  dd 0

renderPatch:
  cmp dword ptr [ebp+754], 14
  jne check_dizzy
  mov ecx, [edx+264]
  jmp render_continue
check_dizzy:
  cmp dword ptr [ebp+754], 12
  jne check_bell
  mov ecx, [edx+274]
  jmp render_continue
check_bell:
  cmp dword ptr [ebp+754], 11
  jne check_funball
  mov ecx, [edx+270]
  jmp render_continue
check_funball:
  cmp dword ptr [ebp+754], 10
  jne use_default
  mov ecx, [edx+26C]
  jmp render_continue
use_default:
  mov ecx, [edx+268]
render_continue:
  mov eax, [ecx]
  push ebx
  lea edx, [esp+14]
  push edx
  jmp 00402F82

label(original_code)

SpawnCode:
  cmp dword ptr [esi+18], 0
  jne original_code
  mov eax, [esi+164]
  mov [playerX], eax
  mov eax, [esi+168]
  mov [playerY], eax
  mov eax, [esi+16C]
  mov [playerZ], eax

  cmp dword ptr [SpawnHB8ball], 0
  je original_code
  cmp dword ptr [esi+18], 0
  jne original_code

  mov dword ptr [SpawnHB8ball], 0
  mov dword ptr [spawnNow], 1

  push ecx
  push edx
  push ebx

  mov ecx, [esi+14]
  mov edx, [ecx+29d8]
  mov [ballCount], edx

  mov ecx, [esi+14]
  call 0040BCA0

  mov ecx, [esi+14]
  mov edx, [ecx+29d8]
  cmp edx, [ballCount]
  jle no_new

  mov ebx, [ecx+2DE0]
  test ebx, ebx
  jz no_new
  mov ebx, [ebx+edx*4-4]
  test ebx, ebx
  jz no_new

  cmp dword ptr [ebx+18], -1
  jne no_new

  mov eax, [playerX]
  mov [ebx+164], eax
  mov eax, [playerY]
  mov [ebx+168], eax
  mov eax, [playerZ]
  mov [ebx+16C], eax
  mov dword ptr [ebx+170], 0
  mov dword ptr [ebx+174], 0
  mov dword ptr [ebx+178], 0

  mov dword ptr [ebx+754], 9
  mov dword ptr [ebx+284], 41D00000
  mov dword ptr [ebx+C6C], 49742400
  mov dword ptr [ebx+C70], 49742400
  mov dword ptr [ebx+C7C], 0

  mov dword ptr [ebx+188], 40800000   // master speed = 4.0
  mov eax, [ebx+1A4]
  test eax, eax
  jz no_new
  mov dword ptr [eax+C70], 40C00000   // max speed = 6.0
  mov dword ptr [eax+C68], 3F800000

no_new:
  mov dword ptr [spawnNow], 0
  pop ebx
  pop edx
  pop ecx

original_code:
  mov eax, [esi+0c5c]
  jmp 00405E28

alloc(spawnHook, 1000)
spawnHook:
  cmp dword ptr [spawnNow], 0
  je normal_check
  cmp dword ptr [spawnNow], 2
  je stop_spawn
  inc dword ptr [spawnNow]
  jmp 0040BD2D
stop_spawn:
  jmp 0040BEFE
normal_check:
  push 7
  push 004CF74C
  push dword ptr [edi]
  call 004C7677
  add esp, C
  test eax, eax
  jnz 0040BEFE
  jmp 0040BD2D

00402F74:
  jmp renderPatch
  nop

0040BD27:
  jmp spawnHook
  nop

00405E22:
  jmp SpawnCode
  nop

[DISABLE]
0040BD27:
  db 0F 85 D1 01 00 00
00405E22:
  db 8B 86 5C 0C 00 00

unregistersymbol(SpawnHB8ball)
dealloc(SpawnCode)
dealloc(spawnHook)
dealloc(SpawnHB8ball)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(ballCount)
dealloc(spawnNow)

```
