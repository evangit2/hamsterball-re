# "Auto spawn random-sized 8balls everywhere"

**CT Entry ID:** 410

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(spawnTimer, 4)
alloc(spawnBad, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(ballCount, 4)
alloc(spawnNow, 4)
alloc(badExists, 4)
alloc(randSeed, 4)
alloc(spawnCount, 4)
alloc(savedSize, 4)
alloc(spawnHook, 1000)
registersymbol(spawnBad)

spawnTimer:
  dd 0
badExists:
  dd 0
randSeed:
  dd 1234567
spawnCount:
  dd 0
savedSize:
  dd 0

newmem:
  cmp dword ptr [esi+0x18], 0
  jne check_badballs
  mov eax, [esi+0x164]
  mov [playerX], eax
  mov eax, [esi+0x168]
  mov [playerY], eax
  mov eax, [esi+0x16C]
  mov [playerZ], eax
  mov dword ptr [badExists], 0

check_badballs:
  cmp dword ptr [esi+0x18], -1
  jne check_timer
  mov dword ptr [badExists], 1

check_timer:
  cmp dword ptr [esi+0x18], 0
  jne do_spawn
  cmp dword ptr [badExists], 1
  je reset_timer
  inc dword ptr [spawnTimer]
  cmp dword ptr [spawnTimer], 90
  jl do_spawn
  mov dword ptr [spawnTimer], 0
  mov dword ptr [spawnBad], 1
  mov dword ptr [spawnCount], 0
  jmp do_spawn

reset_timer:
  mov dword ptr [spawnTimer], 0

do_spawn:
  cmp dword ptr [spawnBad], 0
  je original
  cmp dword ptr [esi+0x18], 0
  jne original

  push ecx
  push edx
  push ebx

spawn_loop:
  mov dword ptr [spawnNow], 1

  mov ecx, [esi+0x14]
  mov edx, [ecx+0x29d8]
  mov [ballCount], edx
  mov ecx, [esi+0x14]
  call 0040BCA0
  mov ecx, [esi+0x14]
  mov edx, [ecx+0x29d8]
  cmp edx, [ballCount]
  jle no_new
  mov ebx, [ecx+0x2DE0]
  test ebx, ebx
  jz no_new
  mov ebx, [ebx+edx*4-4]
  test ebx, ebx
  jz no_new
  cmp dword ptr [ebx+0x18], -1
  jne no_new

  mov eax, [playerX]
  mov [ebx+0x164], eax
  mov eax, [playerY]
  mov [ebx+0x168], eax
  mov eax, [playerZ]
  mov [ebx+0x16C], eax
  mov dword ptr [ebx+0x170], 0
  mov dword ptr [ebx+0x174], 0
  mov dword ptr [ebx+0x178], 0

  cmp dword ptr [spawnCount], 0
  jne use_saved_size

  mov eax, [randSeed]
  mov ecx, 1103515245
  mul ecx
  add eax, 12345
  mov [randSeed], eax
  mov ecx, 8
  xor edx, edx
  div ecx

  cmp edx, 0
  je size10
  cmp edx, 1
  je size15
  cmp edx, 2
  je size20
  cmp edx, 3
  je size25
  cmp edx, 4
  je size30
  cmp edx, 5
  je size37
  cmp edx, 6
  je size55
  mov dword ptr [ebx+0x284], 0x42B40000
  mov dword ptr [spawnBad], 0
  jmp size_done
size10:
  mov dword ptr [ebx+0x284], 0x41200000
  mov eax, [ebx+0x284]
  mov [savedSize], eax
  cmp dword ptr [spawnCount], 0
  jne @f
  mov dword ptr [spawnCount], 3
@@:
  jmp size_done
size15:
  mov dword ptr [ebx+0x284], 0x41700000
  mov eax, [ebx+0x284]
  mov [savedSize], eax
  cmp dword ptr [spawnCount], 0
  jne @f
  mov dword ptr [spawnCount], 3
@@:
  jmp size_done
size20:
  mov dword ptr [ebx+0x284], 0x41A00000
  mov dword ptr [spawnBad], 0
  jmp size_done
size25:
  mov dword ptr [ebx+0x284], 0x41C80000
  mov dword ptr [spawnBad], 0
  jmp size_done
size30:
  mov dword ptr [ebx+0x284], 0x41F00000
  mov dword ptr [spawnBad], 0
  jmp size_done
size37:
  mov dword ptr [ebx+0x284], 0x42140000
  mov dword ptr [spawnBad], 0
  jmp size_done
size55:
  mov dword ptr [ebx+0x284], 0x425C0000
  mov dword ptr [spawnBad], 0
  jmp size_done

use_saved_size:
  mov eax, [savedSize]
  mov [ebx+0x284], eax

size_done:
  mov dword ptr [ebx+0xC6C], 0x49742400
  mov dword ptr [ebx+0xC70], 0x49742400
  mov dword ptr [ebx+0xC7C], 0

no_new:
  mov dword ptr [spawnNow], 0
  dec dword ptr [spawnCount]
  cmp dword ptr [spawnCount], 0
  jg spawn_loop

  mov dword ptr [spawnBad], 0
  pop ebx
  pop edx
  pop ecx

original:
  mov eax, [esi+0x0c5c]
  jmp 00405E28

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
  push 0x7
  push 004CF74C
  push dword ptr [edi]
  call 004C7677
  add esp, 0xC
  test eax, eax
  jnz 0040BEFE
  jmp 0040BD2D

0040BD27:
  jmp spawnHook
  nop

00405E22:
  jmp newmem
  nop

[DISABLE]
00405E22:
db 8B 86 5C 0C 00 00
0040BD27:
db 0F 85 D1 01 00 00
unregistersymbol(spawnBad)
dealloc(newmem)
dealloc(spawnHook)
dealloc(spawnTimer)
dealloc(spawnBad)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(ballCount)
dealloc(spawnNow)
dealloc(badExists)
dealloc(randSeed)
dealloc(spawnCount)
dealloc(savedSize)

```
