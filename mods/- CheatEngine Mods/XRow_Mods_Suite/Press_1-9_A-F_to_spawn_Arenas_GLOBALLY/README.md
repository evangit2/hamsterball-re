# "Press 1-9, A-F to spawn Arenas GLOBALLY"

**CT Entry ID:** 121

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]
// ============================================================
//  Global Arena Spawners - All 15 Arenas
// ============================================================

alloc(SpawnCode, 8192)
alloc(SpawnArena1, 4)
alloc(SpawnArena2, 4)
alloc(SpawnArena3, 4)
alloc(SpawnArena4, 4)
alloc(SpawnArena5, 4)
alloc(SpawnArena6, 4)
alloc(SpawnArena7, 4)
alloc(SpawnArena8, 4)
alloc(SpawnArena9, 4)
alloc(SpawnArena10, 4)
alloc(SpawnArena11, 4)
alloc(SpawnArena12, 4)
alloc(SpawnArena13, 4)
alloc(SpawnArena14, 4)
alloc(SpawnArena15, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_250, 4)
alloc(float_500, 4)
alloc(temp_mesh, 4)
alloc(mesh_select, 4)
alloc(arena_array, 36)
alloc(last_board, 4)

registersymbol(SpawnArena1)
registersymbol(SpawnArena2)
registersymbol(SpawnArena3)
registersymbol(SpawnArena4)
registersymbol(SpawnArena5)
registersymbol(SpawnArena6)
registersymbol(SpawnArena7)
registersymbol(SpawnArena8)
registersymbol(SpawnArena9)
registersymbol(SpawnArena10)
registersymbol(SpawnArena11)
registersymbol(SpawnArena12)
registersymbol(SpawnArena13)
registersymbol(SpawnArena14)
registersymbol(SpawnArena15)

SpawnArena1:
  dd 0
SpawnArena2:
  dd 0
SpawnArena3:
  dd 0
SpawnArena4:
  dd 0
SpawnArena5:
  dd 0
SpawnArena6:
  dd 0
SpawnArena7:
  dd 0
SpawnArena8:
  dd 0
SpawnArena9:
  dd 0
SpawnArena10:
  dd 0
SpawnArena11:
  dd 0
SpawnArena12:
  dd 0
SpawnArena13:
  dd 0
SpawnArena14:
  dd 0
SpawnArena15:
  dd 0

saved_board:
  dd 0

playerX:
  dd 0
playerY:
  dd 0
playerZ:
  dd 0

float_250:
  dd 0xC1A00000

float_500:
  dd 0x428C0000

temp_mesh:
  dd 0

mesh_select:
  dd 0

arena_array:
  dd 0
  dd 0 0 0 0 0 0 0 0

last_board:
  dd 0

label(original_code)
label(save_pos)
label(check_spawn)
label(do_spawn)
label(done_spawn)
label(no_clear)
label(clear_loop)
label(clear_next)
label(clear_done)
label(skip_col)
label(skip_render)

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
  mov eax, [arena_array+4+edx*4]
  test eax, eax
  jz clear_next
  mov dword [eax+0x10D0], 0
clear_next:
  inc edx
  jmp clear_loop
clear_done:
  mov dword [arena_array], 0
  pop eax
  mov [last_board], eax
  jmp save_pos

no_clear:

save_pos:
  cmp dword [esi+0x18], 0
  jne check_spawn

  fld dword [esi+0x164]
  fadd dword [float_500]
  fstp dword [playerX]
  fld dword [esi+0x168]
  fadd dword [float_250]
  fstp dword [playerY]
  fld dword [esi+0x16C]
  fadd dword [float_500]
  fstp dword [playerZ]

check_spawn:
  cmp dword [esi+0x18], 0
  jne original_code

  cmp dword [SpawnArena1], 0
  je check_a2
  mov dword [SpawnArena1], 0
  mov dword [mesh_select], 004CFF98
  jmp do_spawn
check_a2:
  cmp dword [SpawnArena2], 0
  je check_a3
  mov dword [SpawnArena2], 0
  mov dword [mesh_select], 004CFFAC
  jmp do_spawn
check_a3:
  cmp dword [SpawnArena3], 0
  je check_a4
  mov dword [SpawnArena3], 0
  mov dword [mesh_select], 004CFFC4
  jmp do_spawn
check_a4:
  cmp dword [SpawnArena4], 0
  je check_a5
  mov dword [SpawnArena4], 0
  mov dword [mesh_select], 004CFFF4
  jmp do_spawn
check_a5:
  cmp dword [SpawnArena5], 0
  je check_a6
  mov dword [SpawnArena5], 0
  mov dword [mesh_select], 004D0008
  jmp do_spawn
check_a6:
  cmp dword [SpawnArena6], 0
  je check_a7
  mov dword [SpawnArena6], 0
  mov dword [mesh_select], 004D001C
  jmp do_spawn
check_a7:
  cmp dword [SpawnArena7], 0
  je check_a8
  mov dword [SpawnArena7], 0
  mov dword [mesh_select], 004D0130
  jmp do_spawn
check_a8:
  cmp dword [SpawnArena8], 0
  je check_a9
  mov dword [SpawnArena8], 0
  mov dword [mesh_select], 004D002C
  jmp do_spawn
check_a9:
  cmp dword [SpawnArena9], 0
  je check_a10
  mov dword [SpawnArena9], 0
  mov dword [mesh_select], 004D0040
  jmp do_spawn
check_a10:
  cmp dword [SpawnArena10], 0
  je check_a11
  mov dword [SpawnArena10], 0
  mov dword [mesh_select], 004D0054
  jmp do_spawn
check_a11:
  cmp dword [SpawnArena11], 0
  je check_a12
  mov dword [SpawnArena11], 0
  mov dword [mesh_select], 004D0068
  jmp do_spawn
check_a12:
  cmp dword [SpawnArena12], 0
  je check_a13
  mov dword [SpawnArena12], 0
  mov dword [mesh_select], 004D0194
  jmp do_spawn
check_a13:
  cmp dword [SpawnArena13], 0
  je check_a14
  mov dword [SpawnArena13], 0
  mov dword [mesh_select], 004D007C
  jmp do_spawn
check_a14:
  cmp dword [SpawnArena14], 0
  je check_a15
  mov dword [SpawnArena14], 0
  mov dword [mesh_select], 004D00A0
  jmp do_spawn
check_a15:
  cmp dword [SpawnArena15], 0
  je original_code
  mov dword [SpawnArena15], 0
  mov dword [mesh_select], 004D0218

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

  push 0x10D0
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
  mov eax, [arena_array]
  cmp eax, 8
  jae done_spawn
  mov [arena_array+4+eax*4], edi
  inc dword [arena_array]

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
dealloc(SpawnArena1)
dealloc(SpawnArena2)
dealloc(SpawnArena3)
dealloc(SpawnArena4)
dealloc(SpawnArena5)
dealloc(SpawnArena6)
dealloc(SpawnArena7)
dealloc(SpawnArena8)
dealloc(SpawnArena9)
dealloc(SpawnArena10)
dealloc(SpawnArena11)
dealloc(SpawnArena12)
dealloc(SpawnArena13)
dealloc(SpawnArena14)
dealloc(SpawnArena15)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_250)
dealloc(float_500)
dealloc(temp_mesh)
dealloc(mesh_select)
dealloc(arena_array)
dealloc(last_board)
unregistersymbol(SpawnArena1)
unregistersymbol(SpawnArena2)
unregistersymbol(SpawnArena3)
unregistersymbol(SpawnArena4)
unregistersymbol(SpawnArena5)
unregistersymbol(SpawnArena6)
unregistersymbol(SpawnArena7)
unregistersymbol(SpawnArena8)
unregistersymbol(SpawnArena9)
unregistersymbol(SpawnArena10)
unregistersymbol(SpawnArena11)
unregistersymbol(SpawnArena12)
unregistersymbol(SpawnArena13)
unregistersymbol(SpawnArena14)
unregistersymbol(SpawnArena15)

```
