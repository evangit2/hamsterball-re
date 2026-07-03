# "Press S to spawn static half trapdoors GLOBALLY"

**CT Entry ID:** 60

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnTrapdoor, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_20, 4)

registersymbol(SpawnTrapdoor)

SpawnTrapdoor:
  dd 0

saved_board:
  dd 0

playerX:
  dd 0

playerY:
  dd 0

playerZ:
  dd 0

float_20:
  dd 0x41A00000

label(original_code)
label(check_spawn)
label(done_spawn)
label(skip_col1)
label(skip_col2)

SpawnCode:
  cmp dword [esi+0x18], 0
  jne check_spawn

  mov eax, [esi+0x164]
  mov [playerX], eax
  fld dword [esi+0x168]
  fsub dword [float_20]
  fstp dword [playerY]
  mov eax, [esi+0x16C]
  mov [playerZ], eax

check_spawn:
  cmp dword [SpawnTrapdoor], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnTrapdoor], 0

  pushad
  mov eax, [esi+0x14]
  mov [saved_board], eax
  test eax, eax
  jz done_spawn
  mov edx, [eax+0x8AC]
  test edx, edx
  jz done_spawn
  mov edx, [eax+0x878]
  test edx, edx
  jz done_spawn

  push 0x10F8
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn
  mov ebx, eax

  mov edx, [saved_board]
  push edx
  mov ecx, ebx
  call 00438290

  mov edx, [playerX]
  mov [ebx+0x10E0], edx
  mov edx, [playerY]
  mov [ebx+0x10E4], edx
  mov edx, [playerZ]
  mov [ebx+0x10E8], edx

  push ebx
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  push ebx
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz skip_col1
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_col1
  push ebx
  lea ecx, [edx+0x1C]
  call 00453810

skip_col1:
  mov edi, [ebx+0x10D8]
  test edi, edi
  jz skip_col2
  push edi
  mov ecx, [saved_board]
  add ecx, 0x10EC
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8B0]
  test edx, edx
  jz skip_col2
  push edi
  lea ecx, [edx+0x18]
  call 00453810

skip_col2:
  mov edi, [ebx+0x10DC]
  test edi, edi
  jz done_spawn
  push edi
  mov ecx, [saved_board]
  add ecx, 0x10EC
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8B0]
  test edx, edx
  jz done_spawn
  push edi
  lea ecx, [edx+0x18]
  call 00453810

done_spawn:
  popad

original_code:
  mov eax, [esi+0x0c5c]
  jmp 00405E28

00405E22:
  jmp SpawnCode
  nop

[DISABLE]
00405E22:
  db 8B 86 5C 0C 00 00

dealloc(SpawnCode)
dealloc(SpawnTrapdoor)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_20)
unregistersymbol(SpawnTrapdoor)

```
