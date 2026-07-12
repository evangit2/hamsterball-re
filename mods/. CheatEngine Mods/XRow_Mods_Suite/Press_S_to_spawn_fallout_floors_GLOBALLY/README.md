# "Press S to spawn fallout floors GLOBALLY"

**CT Entry ID:** 81

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnFallout, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_20_sub, 4)
alloc(temp_mesh, 4)
alloc(fallout_array, 68)
alloc(last_board, 4)

registersymbol(SpawnFallout)

SpawnFallout:
  dd 0

saved_board:
  dd 0

playerX:
  dd 0

playerY:
  dd 0

playerZ:
  dd 0

float_20_sub:
  dd 0xC1A00000

temp_mesh:
  dd 0

fallout_array:
  dd 0
  dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

last_board:
  dd 0

label(original_code)
label(save_pos)
label(check_spawn)
label(done_spawn)
label(no_clear)
label(do_mesh)
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
  mov [last_board], eax
  mov dword [fallout_array], 0

no_clear:
  mov eax, [fallout_array]
  test eax, eax
  jz save_pos
  pushad
  xor ebx, ebx

prox_loop:
  cmp ebx, [fallout_array]
  jae prox_done
  mov eax, [fallout_array+4+ebx*4]
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
  fadd dword [float_20_sub]
  fstp dword [playerY]
  mov eax, [esi+0x16C]
  mov [playerZ], eax

check_spawn:
  cmp dword [SpawnFallout], 0
  je original_code
  cmp dword [esi+0x18], 0
  jne original_code
  mov dword [SpawnFallout], 0

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

do_mesh:
  push 0x10D0
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn

  mov edx, [saved_board]
  mov edx, [edx+0x878]
  mov edx, [edx+0x174]
  push 004D0E0C
  push edx
  mov ecx, eax
  call 00461510
  test eax, eax
  jz done_spawn
  mov [temp_mesh], eax

  push 0x10E8
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
  call 0043BBC0

  test eax, eax
  jz done_spawn
  mov edi, eax

  push edi
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  push edi
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov eax, [edi+0x10D4]
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
  mov eax, [edi+0x10D4]
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
  mov eax, [fallout_array]
  cmp eax, 16
  jae done_spawn
  mov [fallout_array+4+eax*4], edi
  inc dword [fallout_array]

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
dealloc(SpawnFallout)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_20_sub)
dealloc(temp_mesh)
dealloc(fallout_array)
dealloc(last_board)
unregistersymbol(SpawnFallout)

```
