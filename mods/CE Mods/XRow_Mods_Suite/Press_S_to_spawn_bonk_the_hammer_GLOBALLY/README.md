# "Press S to spawn bonk the hammer GLOBALLY"

**CT Entry ID:** 75

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnBonk, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_20, 4)
alloc(bonk_array, 68)
alloc(float_900, 4)
alloc(last_board, 4)
alloc(update_timer, 4)
alloc(saved_436C, 4)

registersymbol(SpawnBonk)

SpawnBonk:
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

bonk_array:
  dd 0
  dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

float_900:
  dd 0x44610000

last_board:
  dd 0

update_timer:
  dd 0

saved_436C:
  dd 0

label(original_code)
label(save_pos)
label(check_spawn)
label(done_spawn)
label(no_clear)
label(skip_col)
label(skip_scene)
label(prox_loop)
label(prox_next)
label(prox_near)
label(prox_done)
label(do_update)
label(check_active)
label(restore_436C)

SpawnCode:
  mov eax, [esi+0x14]
  test eax, eax
  jz no_clear
  cmp eax, [last_board]
  je no_clear
  mov [last_board], eax
  mov dword [bonk_array], 0

no_clear:
  mov eax, [bonk_array]
  test eax, eax
  jz save_pos

  pushad
  xor ebx, ebx

prox_loop:
  cmp ebx, [bonk_array]
  jae prox_done

  mov eax, [bonk_array+4+ebx*4]
  test eax, eax
  jz prox_next

  mov ecx, [eax]
  test ecx, ecx
  jz prox_next

  mov edx, [eax+0x10D0]
  cmp edx, [last_board]
  jne prox_next

  cmp byte [eax+0x10FD], 1
  jne check_active

do_update:
  mov edx, [esi+0x14]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz prox_next
  inc dword [update_timer]
  mov edx, [esi+0x164]
  mov [eax+0x1120], edx
  mov edx, [esi+0x168]
  mov [eax+0x1124], edx
  mov edx, [esi+0x16C]
  mov [eax+0x1128], edx
  cmp dword [update_timer], 15
  jl prox_next
  mov dword [update_timer], 0
  push eax
  mov ecx, eax
  mov edx, [eax]
  call dword [edx+0x2C]
  pop eax
  jmp prox_next

check_active:
  cmp byte [eax+0x10FC], 0
  je prox_next

  mov edx, [esi+0x164]
  mov [eax+0x1120], edx
  mov edx, [esi+0x168]
  mov [eax+0x1124], edx
  mov edx, [esi+0x16C]
  mov [eax+0x1128], edx

  fld dword [esi+0x164]
  fsub dword [eax+0x10E0]
  fmul st, st
  fld dword [esi+0x16C]
  fsub dword [eax+0x10E8]
  fmul st, st
  faddp
  fld dword [float_900]
  fcomip st(1)
  fstp st
  jb prox_next

prox_near:
  mov byte [eax+0x10FC], 0
  mov byte [eax+0x10FD], 1
  mov dword [eax+0x1104], 1
  mov dword [eax+0x1138], 0x3F000000
  jmp prox_done

prox_next:
  inc ebx
  jmp prox_loop

prox_done:
  popad

save_pos:
  cmp dword [esi+0x18], 0
  jne check_spawn

  fld dword [esi+0x164]
  fadd dword [float_20]
  fstp dword [playerX]
  mov eax, [esi+0x168]
  mov [playerY], eax
  mov eax, [esi+0x16C]
  mov [playerZ], eax

check_spawn:
  cmp dword [SpawnBonk], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnBonk], 0

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

  mov edx, [eax+0x436C]
  mov [saved_436C], edx

  push 0x1200
  call 004BA57B
  add esp, 4
  test eax, eax
  jz restore_436C

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
  call 00438850
  test eax, eax
  jz restore_436C
  mov edi, eax

  mov edx, [playerX]
  mov [edi+0x10D4], edx
  mov [edi+0x10E0], edx
  mov [edi+0x1108], edx
  mov edx, [playerY]
  mov [edi+0x10D8], edx
  mov [edi+0x10E4], edx
  mov [edi+0x110C], edx
  mov edx, [playerZ]
  mov [edi+0x10DC], edx
  mov [edi+0x10E8], edx
  mov [edi+0x1110], edx

  push edi
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  push edi
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz skip_scene
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_scene
  push edi
  lea ecx, [edx+0x1C]
  call 00453810

skip_scene:
  mov eax, [edi+0x10F8]
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
  mov eax, [edi+0x10F8]
  push eax
  lea ecx, [edx+0x18]
  call 00453810

skip_col:
  mov edx, [saved_board]
  mov [edx+0x436C], edi

  mov eax, edi
  mov ecx, [bonk_array]
  cmp ecx, 16
  jae restore_436C
  mov [bonk_array+4+ecx*4], eax
  inc dword [bonk_array]

restore_436C:
  mov edx, [saved_board]
  mov eax, [saved_436C]
  mov [edx+0x436C], eax

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
dealloc(SpawnBonk)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_20)
dealloc(bonk_array)
dealloc(float_900)
dealloc(last_board)
dealloc(update_timer)
dealloc(saved_436C)
unregistersymbol(SpawnBonk)

```
