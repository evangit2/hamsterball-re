# "Press S to spawn strong bump popcylinders GLOBALLY"

**CT Entry ID:** 208

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

// =====================================================
// Global Bumper Spawner v19 — Fixed Speed 3.0
// =====================================================

alloc(BumperCode, 4096)
alloc(SpawnBumper2, 4)
alloc(saved_board, 4)
alloc(bumper_array, 68)
alloc(bumper_cooldown, 68)
alloc(our_mesh, 4)
alloc(temp_mesh, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_y_offset, 4)
alloc(float_x_offset, 4)
alloc(last_board, 4)
alloc(bumper_radius_sq, 4)
alloc(float_bump_speed, 4)

registersymbol(SpawnBumper2)

SpawnBumper2:
  dd 0
saved_board:
  dd 0
bumper_array:
  dd 0
  dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
bumper_cooldown:
  dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
our_mesh:
  dd 0
temp_mesh:
  dd 0
playerX:
  dd 0
playerY:
  dd 0
playerZ:
  dd 0
float_y_offset:
  dd C0A00000
float_x_offset:
  dd 42480000
last_board:
  dd 0
bumper_radius_sq:
  dd 457A0000
float_bump_speed:
  dd 3EBD70A4

label(original_code)
label(save_pos)
label(check_spawn)
label(do_spawn)
label(done_spawn)
label(no_clear)
label(skip_col)
label(skip_render)
label(check_prox)
label(apply_bump)
label(next_bumper)
label(bump_done)

BumperCode:
  mov eax, [esi+14]
  test eax, eax
  jz original_code
  cmp eax, [last_board]
  je no_clear
  mov [last_board], eax
  mov dword [bumper_array], 0

no_clear:
  mov eax, [bumper_array]
  test eax, eax
  jz save_pos

  pushad
  xor ebx, ebx

check_prox:
  cmp ebx, [bumper_array]
  jae bump_done

  cmp dword [bumper_cooldown+ebx*4], 0
  jle check_dist
  dec dword [bumper_cooldown+ebx*4]
  jmp next_bumper

check_dist:
  mov ecx, [bumper_array+4+ebx*4]
  test ecx, ecx
  jz next_bumper

  fld dword [ecx+10D4]
  fsub dword [esi+164]
  fmul st(0), st(0)
  fld dword [ecx+10DC]
  fsub dword [esi+16C]
  fmul st(0), st(0)
  faddp
  fld dword [bumper_radius_sq]
  fcomip st(1)
  fstp st(0)
  jb next_bumper

apply_bump:
  mov dword [bumper_cooldown+ebx*4], 30

  mov eax, [esi+1A4]
  test eax, eax
  jz next_bumper

  fld dword [esi+164]
  fsub dword [ecx+10D4]
  fmul dword [float_bump_speed]
  fstp dword [eax+CA4]
  fld dword [esi+16C]
  fsub dword [ecx+10DC]
  fmul dword [float_bump_speed]
  fstp dword [eax+CAC]
  mov dword [eax+CA8], 0

next_bumper:
  inc ebx
  jmp check_prox

bump_done:
  popad

save_pos:
  cmp dword [esi+18], 0
  jne check_spawn

  fld dword [esi+164]
  fadd dword [float_x_offset]
  fstp dword [playerX]
  fld dword [esi+168]
  fadd dword [float_y_offset]
  fstp dword [playerY]
  mov eax, [esi+16C]
  mov [playerZ], eax

check_spawn:
  cmp dword [SpawnBumper2], 0
  je original_code
  cmp dword [esi+18], 0
  jne original_code
  mov dword [SpawnBumper2], 0

do_spawn:
  pushad
  mov eax, [esi+14]
  mov [saved_board], eax
  test eax, eax
  jz done_spawn
  mov edx, [eax+878]
  test edx, edx
  jz done_spawn
  mov edx, [edx+174]
  test edx, edx
  jz done_spawn

  push 10D0
  call 004BA57B
  add esp, 4
  test eax, eax
  jz done_spawn

  mov edx, [saved_board]
  mov edx, [edx+878]
  mov edx, [edx+174]
  push 004D0F40
  push edx
  mov ecx, eax
  call 00461510
  test eax, eax
  jz done_spawn
  mov [temp_mesh], eax

  push 10D0
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
  call 00436EE0
  test eax, eax
  jz done_spawn
  mov edi, eax

  push edi
  mov ecx, [saved_board]
  add ecx, 2578
  call 00453810

  push edi
  mov ecx, [saved_board]
  add ecx, CD4
  call 00453810

  mov eax, [edi+10E0]
  test eax, eax
  jz skip_col
  push eax
  mov ecx, [saved_board]
  add ecx, 10EC
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+8B0]
  test edx, edx
  jz skip_col
  mov eax, [edi+10E0]
  push eax
  lea ecx, [edx+18]
  call 00453810

skip_col:
  mov edx, [saved_board]
  mov edx, [edx+8AC]
  test edx, edx
  jz skip_render
  mov edx, [edx+480]
  test edx, edx
  jz skip_render
  push edi
  lea ecx, [edx+1C]
  call 00453810

skip_render:
  mov eax, [bumper_array]
  cmp eax, 16
  jae done_spawn
  mov [bumper_array+4+eax*4], edi
  inc dword [bumper_array]

done_spawn:
  popad
  mov dword [temp_mesh], 0

original_code:
  mov eax, [esi+0c5c]
  jmp 00405E28

00405E22:
  jmp BumperCode
  nop

[DISABLE]

00405E22:
  db 8B 86 5C 0C 00 00

dealloc(BumperCode)
dealloc(SpawnBumper2)
dealloc(saved_board)
dealloc(bumper_array)
dealloc(bumper_cooldown)
dealloc(our_mesh)
dealloc(temp_mesh)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_y_offset)
dealloc(float_x_offset)
dealloc(last_board)
dealloc(bumper_radius_sq)
dealloc(float_bump_speed)
unregistersymbol(SpawnBumper2)

```
