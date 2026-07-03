# "Press 1,2,3,4 to spawn neon DFloors 1-4 GLOBALLY"

**CT Entry ID:** 66

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnDFloor1, 4)
alloc(SpawnDFloor2, 4)
alloc(SpawnDFloor3, 4)
alloc(SpawnDFloor4, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(temp_mesh, 4)
alloc(float_20, 4)

registersymbol(SpawnDFloor1)
registersymbol(SpawnDFloor2)
registersymbol(SpawnDFloor3)
registersymbol(SpawnDFloor4)

SpawnDFloor1:
  dd 0

SpawnDFloor2:
  dd 0

SpawnDFloor3:
  dd 0

SpawnDFloor4:
  dd 0

saved_board:
  dd 0

playerX:
  dd 0

playerY:
  dd 0

playerZ:
  dd 0

temp_mesh:
  dd 0

float_20:
  dd 0x41A00000

label(original_code)
label(check_dfloor1)
label(check_dfloor2)
label(check_dfloor3)
label(check_dfloor4)
label(do_spawn)
label(done_spawn)
label(skip_col)

SpawnCode:
  cmp dword [esi+0x18], 0
  jne check_dfloor1

  mov eax, [esi+0x164]
  mov [playerX], eax
  fld dword [esi+0x168]
  fsub dword [float_20]
  fstp dword [playerY]
  mov eax, [esi+0x16C]
  mov [playerZ], eax

check_dfloor1:
  cmp dword [SpawnDFloor1], 0
  je check_dfloor2
  cmp dword [esi+0x18], 0
  jne check_dfloor2
  mov dword [SpawnDFloor1], 0
  push 004D1D80
  jmp do_spawn

check_dfloor2:
  cmp dword [SpawnDFloor2], 0
  je check_dfloor3
  cmp dword [esi+0x18], 0
  jne check_dfloor3
  mov dword [SpawnDFloor2], 0
  push 004D1D64
  jmp do_spawn

check_dfloor3:
  cmp dword [SpawnDFloor3], 0
  je check_dfloor4
  cmp dword [esi+0x18], 0
  jne check_dfloor4
  mov dword [SpawnDFloor3], 0
  push 004D1D48
  jmp do_spawn

check_dfloor4:
  cmp dword [SpawnDFloor4], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnDFloor4], 0
  push 004D1D2C

do_spawn:
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
  mov edx, [edx+0x174]
  test edx, edx
  jz done_spawn

  push 0x10D0
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn
  mov ebx, eax

  mov edx, [saved_board]
  mov edx, [edx+0x878]
  mov edx, [edx+0x174]
  mov ecx, [esp+0x20]
  push ecx
  push edx
  mov ecx, ebx
  call 00461510
  test eax, eax
  jz done_spawn
  mov [temp_mesh], eax

  push 0x1104
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn

  push [temp_mesh]
  sub esp, 0xC
  mov edx, [playerX]
  mov [esp], edx
  mov edx, [playerY]
  mov [esp+4], edx
  mov edx, [playerZ]
  mov [esp+8], edx
  mov edx, [saved_board]
  push edx
  mov ecx, eax
  call 0043E450
  test eax, eax
  jz done_spawn
  mov esi, eax

  push esi
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  push esi
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz skip_col
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_col
  push esi
  lea ecx, [edx+0x1C]
  call 00453810

skip_col:
  mov eax, [esi+0x10E8]
  test eax, eax
  jz done_spawn
  push eax
  mov ecx, [saved_board]
  add ecx, 0x10EC
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8B0]
  test edx, edx
  jz done_spawn
  mov eax, [esi+0x10E8]
  push eax
  lea ecx, [edx+0x18]
  call 00453810

done_spawn:
  popad
  add esp, 4

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
dealloc(SpawnDFloor1)
dealloc(SpawnDFloor2)
dealloc(SpawnDFloor3)
dealloc(SpawnDFloor4)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(temp_mesh)
dealloc(float_20)
unregistersymbol(SpawnDFloor1)
unregistersymbol(SpawnDFloor2)
unregistersymbol(SpawnDFloor3)
unregistersymbol(SpawnDFloor4)

```
