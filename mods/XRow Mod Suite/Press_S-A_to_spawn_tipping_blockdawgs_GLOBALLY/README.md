# "Press S-A to spawn tipping blockdawgs GLOBALLY"

**CT Entry ID:** 455

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
// ============================================================
//  SpawnDawg1 & SpawnDawg2 - Pillar Constructor + Blockdawg
// ============================================================

alloc(SpawnCode, 4096)
alloc(SpawnDawg1, 4)
alloc(SpawnDawg2, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_40_sub, 4)
alloc(temp_mesh, 4)
alloc(mesh_select, 4)
alloc(dawg_array, 36)
alloc(last_board, 4)

registersymbol(SpawnDawg1)
registersymbol(SpawnDawg2)

SpawnDawg1:
  dd 0
SpawnDawg2:
  dd 0

saved_board:
  dd 0

playerX:
  dd 0
playerY:
  dd 0
playerZ:
  dd 0

float_40_sub:
  dd 0xC2200000

temp_mesh:
  dd 0

mesh_select:
  dd 0

dawg_array:
  dd 0
  dd 0 0 0 0 0 0 0 0

last_board:
  dd 0

label(original_code)
label(save_pos)
label(check_b1)
label(check_b2)
label(do_spawn)
label(done_spawn)
label(no_clear)
label(clear_loop)
label(clear_next)
label(clear_done)
label(skip_render)
label(skip_col)
label(prox_loop)
label(prox_next)
label(prox_done)

SpawnCode:
  cmp dword [esi+0x18], 0
  jne original_code

  mov eax, [esi+0x14]
  test eax, eax
  jz original_code
  cmp eax, [last_board]
  je no_clear

  push eax
  xor edx, edx
clear_loop:
  cmp edx, 8
  jae clear_done
  mov eax, [dawg_array+4+edx*4]
  test eax, eax
  jz clear_next
  mov dword [eax+0x10D0], 0
clear_next:
  inc edx
  jmp clear_loop
clear_done:
  mov dword [dawg_array], 0
  pop eax
  mov [last_board], eax

no_clear:
  mov eax, [dawg_array]
  test eax, eax
  jz save_pos
  pushad
  xor ebx, ebx

prox_loop:
  cmp ebx, [dawg_array]
  jae prox_done
  mov eax, [dawg_array+4+ebx*4]
  test eax, eax
  jz prox_next
  mov ecx, [eax]
  test ecx, ecx
  jz prox_next
  cmp dword [esi+0x18], 0
  jne prox_next
  mov edx, [eax+0x10D0]
  cmp edx, [last_board]
  jne prox_next

  mov ecx, eax
  mov edx, [eax]
  call dword [edx+0x2C]

  mov eax, [dawg_array+4+ebx*4]
  cmp dword [eax+0x10EC], 0
  jne prox_next
  mov dword [eax+0x10EC], 1

prox_next:
  inc ebx
  jmp prox_loop
prox_done:
  popad

save_pos:
  cmp dword [esi+0x18], 0
  jne check_b1

  mov eax, [esi+0x164]
  mov [playerX], eax
  fld dword [esi+0x168]
  fadd dword [float_40_sub]
  fstp dword [playerY]
  mov eax, [esi+0x16C]
  mov [playerZ], eax

check_b1:
  cmp dword [SpawnDawg1], 0
  je check_b2
  cmp dword [esi+0x18], 0
  jne check_b2
  mov dword [SpawnDawg1], 0
  mov dword [mesh_select], 004D0DF0
  jmp do_spawn

check_b2:
  cmp dword [SpawnDawg2], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnDawg2], 0
  mov dword [mesh_select], 004D0DD4

do_spawn:
  pushad
  mov eax, [esi+0x14]
  mov [saved_board], eax
  test eax, eax
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

  mov edx, [saved_board]
  mov edx, [edx+0x878]
  mov edx, [edx+0x174]
  push dword [mesh_select]
  push edx
  mov ecx, eax
  call 00461510
  test eax, eax
  jz done_spawn
  mov [temp_mesh], eax

  push 0x1100
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn
  mov edi, eax

  mov edx, [temp_mesh]
  push edx
  mov edx, [playerZ]
  push edx
  mov edx, [playerY]
  push edx
  mov edx, [playerX]
  push edx
  mov edx, [saved_board]
  push edx
  mov ecx, edi
  call 004363F0
  test eax, eax
  jz done_spawn
  mov edi, eax

  mov dword [edi+0x10EC], 1

  push edi
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  push edi
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov eax, [edi+0x10E0]
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
  mov eax, [edi+0x10E0]
  push eax
  lea ecx, [edx+0x18]
  call 00453810

skip_col:
  mov edx, [saved_board]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz skip_render
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_render
  push edi
  lea ecx, [edx+0x1C]
  call 00453810

skip_render:
  mov eax, [dawg_array]
  cmp eax, 8
  jae done_spawn
  mov [dawg_array+4+eax*4], edi
  inc dword [dawg_array]

done_spawn:
  popad
  mov dword [temp_mesh], 0

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
dealloc(SpawnDawg1)
dealloc(SpawnDawg2)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_40_sub)
dealloc(temp_mesh)
dealloc(mesh_select)
dealloc(dawg_array)
dealloc(last_board)
unregistersymbol(SpawnDawg1)
unregistersymbol(SpawnDawg2)

```
