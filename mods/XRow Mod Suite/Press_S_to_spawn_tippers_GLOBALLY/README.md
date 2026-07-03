# "Press S to spawn tippers GLOBALLY"

**CT Entry ID:** 54

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

alloc(SpawnCode, 4096)
alloc(SpawnTipper, 4)
alloc(saved_board, 4)
alloc(out_obj, 4)
alloc(temp_mesh, 4)
alloc(temp_submesh, 4)
alloc(temp_subobj, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_20, 4)

registersymbol(SpawnTipper)

SpawnTipper:
  dd 0

saved_board:
  dd 0

out_obj:
  dd 0

temp_mesh:
  dd 0

temp_submesh:
  dd 0

temp_subobj:
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
label(skip_post_spawn)
label(done_spawn)

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
  cmp dword [SpawnTipper], 0
  je original_code

  cmp dword [esi+0x18], 0
  jne original_code

  mov dword [SpawnTipper], 0

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

  push 004D07E8
  push edx
  mov ecx, ebx
  call 00461510

  test eax, eax
  jz done_spawn

  mov [temp_mesh], eax

  push 0x10D0
  call 004BA57B
  add esp, 4

  test eax, eax
  jz done_spawn

  mov edx, [temp_mesh]
  push edx
  mov ecx, eax
  call 00465080

  test eax, eax
  jz done_spawn

  mov [temp_submesh], eax

  push 0x1104
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
  call 00437960

  test eax, eax
  jz done_spawn

  mov [out_obj], eax

  mov edx, [playerX]
  mov [eax+0x10D8], edx
  mov edx, [playerY]
  mov [eax+0x10DC], edx
  mov edx, [playerZ]
  mov [eax+0x10E0], edx

  push 0x10D0
  call 004BA57B
  add esp, 4

  test eax, eax
  jz done_spawn

  mov [temp_subobj], eax

  mov edx, [temp_submesh]
  push edx
  mov ecx, eax
  call 004661A0

  mov eax, [temp_subobj]
  test eax, eax
  jz done_spawn

  mov edx, [out_obj]
  push edx
  mov ecx, eax
  call 00465200

  mov eax, [out_obj]
  mov edx, [temp_subobj]
  mov [eax+0x10D4], edx

  push eax
  mov ecx, [saved_board]
  add ecx, 0x2578
  call 00453810

  mov eax, [out_obj]
  push eax
  mov ecx, [saved_board]
  add ecx, 0xCD4
  call 00453810

  mov edx, [saved_board]
  mov edx, [edx+0x8AC]
  test edx, edx
  jz skip_post_spawn
  mov edx, [edx+0x480]
  test edx, edx
  jz skip_post_spawn

  mov eax, [out_obj]
  push eax
  lea ecx, [edx+0x1C]
  call 00453810

skip_post_spawn:
  mov dword [temp_mesh], 0
  mov dword [temp_submesh], 0
  mov dword [temp_subobj], 0

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
dealloc(SpawnTipper)
dealloc(saved_board)
dealloc(out_obj)
dealloc(temp_mesh)
dealloc(temp_submesh)
dealloc(temp_subobj)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_20)
unregistersymbol(SpawnTipper)

```
