# "Press S to spawn maces GLOBALLY"

**CT Entry ID:** 430

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnMace, 4)
alloc(saved_board, 4)
alloc(out_obj, 4)
alloc(temp_mesh, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(mace_array, 68)
alloc(float_900, 4)
alloc(float_20, 4)
alloc(last_board, 4)

registersymbol(SpawnMace)

SpawnMace:
  dd 0

saved_board:
  dd 0

out_obj:
  dd 0

temp_mesh:
  dd 0

playerX:
  dd 0

playerY:
  dd 0

playerZ:
  dd 0

mace_array:
  dd 0
  dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

float_900:
  dd 0x44610000

float_20:
  dd 0x41A00000

last_board:
  dd 0

label(original_code)
label(save_pos)
label(check_spawn)
label(done_spawn)
label(no_clear)
label(skip_col)
label(prox_loop)
label(prox_next)
label(prox_near)
label(prox_done)

SpawnCode:
  mov eax, [esi+0x14]
  test eax, eax
  jz no_clear
  cmp eax, [last_board]
  je no_clear
  mov [last_board], eax
  mov dword [mace_array], 0

no_clear:
  mov eax, [mace_array]
  test eax, eax
  jz save_pos

  pushad
  xor ebx, ebx

prox_loop:
  cmp ebx, [mace_array]
  jae prox_done

  mov eax, [mace_array+4+ebx*4]
  test eax, eax
  jz prox_next

  mov ecx, [eax]
  test ecx, ecx
  jz prox_next

  cmp byte [eax+0x10F0], 0
  jne prox_next

  fld dword [esi+0x164]
  fsub dword [eax+0x10D8]
  fmul st, st

  fld dword [esi+0x16C]
  fsub dword [eax+0x10E0]
  fmul st, st
  faddp

  fld dword [float_900]
  fcomip st(1)
  fstp st
  jb prox_next

prox_near:
  mov byte [eax+0x10F0], 1
  jmp prox_done

prox_next:
  inc ebx
  jmp prox_loop

prox_done:
  popad

save_pos:
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
  cmp dword [SpawnMace], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnMace], 0

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
  push 004D0974
  push edx
  mov ecx, ebx
  call 00461510
  test eax, eax
  jz done_spawn
  mov [temp_mesh], eax

  push 0x110C
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn
  mov edi, eax

  mov edx, [temp_mesh]
  push edx
  mov edx, [saved_board]
  push edx
  mov ecx, edi
  call 00438750
  test eax, eax
  jz done_spawn
  mov [out_obj], eax
  mov esi, eax

  mov edx, [playerX]
  mov [esi+0x10D8], edx
  mov edx, [playerY]
  mov [esi+0x10DC], edx
  mov edx, [playerZ]
  mov [esi+0x10E0], edx

  sub esp, 68
  mov ebx, esp
  mov ecx, ebx
  call 00457AD0

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
  jz skip_lists
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_lists
  push esi
  lea ecx, [edx+0x1C]
  call 00453810

skip_lists:
  mov eax, [esi+0x10D4]
  test eax, eax
  jz skip_col
  push eax
  mov ecx, [saved_board]
  add ecx, 0x10EC
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8B0]
  test edx, edx
  jz skip_col
  mov eax, [esi+0x10D4]
  push eax
  lea ecx, [edx+0x18]
  call 00453810

skip_col:
  mov ecx, esi
  mov eax, [esi]
  call dword [eax+0x58]

  push ebx
  mov ecx, esi
  mov eax, [esi]
  call dword [eax+0x54]

  mov ecx, ebx
  call 00457A40
  add esp, 68

  mov eax, [out_obj]
  mov ecx, [mace_array]
  cmp ecx, 16
  jae done_spawn
  mov [mace_array+4+ecx*4], eax
  inc dword [mace_array]

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
dealloc(SpawnMace)
dealloc(saved_board)
dealloc(out_obj)
dealloc(temp_mesh)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(mace_array)
dealloc(float_900)
dealloc(float_20)
dealloc(last_board)
unregistersymbol(SpawnMace)

```
