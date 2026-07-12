# "Press S to spawn Tar Signs"

**CT Entry ID:** 385

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(spawnTar, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(tarCount, 4)
alloc(spawnNow, 4)
registersymbol(spawnTar)

newmem:
  cmp dword ptr [esi+0x18], 0
  jne @f
  mov eax, [esi+0x164]
  mov [playerX], eax
  mov eax, [esi+0x168]
  mov [playerY], eax
  mov eax, [esi+0x16C]
  mov [playerZ], eax
@@:
  cmp dword ptr [spawnTar], 0
  je original
  cmp dword ptr [esi+0x18], 0
  jne original

  mov dword ptr [spawnTar], 0
  mov dword ptr [spawnNow], 1

  push ecx
  push edx
  push ebx

  mov ecx, [esi+0x14]
  mov edx, [ecx+0xCD8]
  mov [tarCount], edx

  mov ecx, [esi+0x14]
  call 0040C270

  mov ecx, [esi+0x14]
  mov edx, [ecx+0xCD8]
  cmp edx, [tarCount]
  jle no_new

  mov ebx, ecx
  add ebx, 0xCD4
  mov ebx, [ebx+0x40C]
  test ebx, ebx
  jz no_new
  mov ebx, [ebx+edx*4-4]
  test ebx, ebx
  jz no_new

  // Set both position sets
  mov eax, [playerX]
  mov [ebx+0x10D4], eax
  mov [ebx+0x10E0], eax
  mov eax, [playerY]
  mov [ebx+0x10D8], eax
  mov [ebx+0x10E4], eax
  mov eax, [playerZ]
  mov [ebx+0x10DC], eax
  mov [ebx+0x10E8], eax

no_new:
  mov dword ptr [spawnNow], 0
  pop ebx
  pop edx
  pop ecx

original:
  mov eax, [esi+0x0c5c]
  jmp 00405E28

alloc(spawnHook, 1000)
spawnHook:
  cmp dword ptr [spawnNow], 0
  je normal_check
  cmp dword ptr [spawnNow], 2
  je stop_spawn
  inc dword ptr [spawnNow]
  jmp 0040C2FA

stop_spawn:
  jmp 0040C3E1

normal_check:
  push 0x4
  push 004CF780
  push dword ptr [ebx]
  call 004C7677
  add esp, 0xC
  test eax, eax
  jnz 0040C3E1
  jmp 0040C2FA

0040C2F4:
  jmp spawnHook
  nop

00405E22:
  jmp newmem
  nop

[DISABLE]
00405E22:
  db 8B 86 5C 0C 00 00
0040C2F4:
  db 0F 85 E7 00 00 00
unregistersymbol(spawnTar)
dealloc(newmem)
dealloc(spawnHook)
dealloc(spawnTar)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(tarCount)
dealloc(spawnNow)

```
