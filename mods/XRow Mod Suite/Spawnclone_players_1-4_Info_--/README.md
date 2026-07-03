# "Spawn/clone players 1-4                                                 [] Info -->"

**CT Entry ID:** 418

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{

Press hotkeys to spawn/clone players:

1 = P1
2 = P2
3 = P3
4 = P4

If the original player is not in the level, it will
just spawn Player 1

Numbers in numpad wont work

}

[ENABLE]

alloc(CloneCode, 8192)
alloc(scene_saved, 4)
alloc(spawnP1, 4)
alloc(spawnP2, 4)
alloc(spawnP3, 4)
alloc(spawnP4, 4)
alloc(player_idx, 4)
alloc(src_ball, 4)
alloc(new_ball, 4)
alloc(clone_count, 4)
alloc(clone_ptrs, 32)
alloc(clone_pidx, 32)
alloc(force_buf, 12)
alloc(saved_ball_ptr, 4)
alloc(saved_ctrl_mode, 4)
alloc(tmp_val, 4)

scene_saved:
  dd 0
spawnP1:
  dd 0
spawnP2:
  dd 0
spawnP3:
  dd 0
spawnP4:
  dd 0
player_idx:
  dd 0
src_ball:
  dd 0
new_ball:
  dd 0
clone_count:
  dd 0
clone_ptrs:
  dd 0 0 0 0 0 0 0 0
clone_pidx:
  dd 0 0 0 0 0 0 0 0
force_buf:
  dd 0 0 0
saved_ball_ptr:
  dd 0
saved_ctrl_mode:
  dd 0
tmp_val:
  dd 0

registersymbol(scene_saved)
registersymbol(spawnP1)
registersymbol(spawnP2)
registersymbol(spawnP3)
registersymbol(spawnP4)

label(original_code)
label(check_p2)
label(check_p3)
label(check_p4)
label(done_spawn)
label(spawn_skip)
label(spawn_found)
label(ai_loop)
label(ai_next)
label(ai_done)
label(ai_skip_dead)
label(not_stale)
label(no_respawn_reset)
label(has_target)
label(restore_table)
label(phase2)
label(use_p1_fallback)
label(no_fallback)
label(compact_loop)
label(compact_done)
label(compact_skip)

// PATCH: Chase everywhere - remove Y-check in COMPUTER AI
0042238A:
  db EB 0E

CloneCode:
  mov [scene_saved], ecx

  mov eax, [spawnP1]
  or eax, [spawnP2]
  or eax, [spawnP3]
  or eax, [spawnP4]
  test eax, eax
  jz phase2

  pushad

  cmp [spawnP1], 0
  je check_p2
  mov [spawnP1], 0
  mov [player_idx], 0
  call SpawnClone

check_p2:
  cmp [spawnP2], 0
  je check_p3
  mov [spawnP2], 0
  mov [player_idx], 1
  call SpawnClone

check_p3:
  cmp [spawnP3], 0
  je check_p4
  mov [spawnP3], 0
  mov [player_idx], 2
  call SpawnClone

check_p4:
  cmp [spawnP4], 0
  je done_spawn
  mov [spawnP4], 0
  mov [player_idx], 3
  call SpawnClone

done_spawn:
  popad

phase2:
  cmp [clone_count], 0
  je original_code

  pushad

  xor esi, esi

ai_loop:
  cmp esi, [clone_count]
  jae ai_done

  mov eax, esi
  shl eax, 2
  add eax, clone_ptrs
  mov eax, [eax]
  test eax, eax
  jz ai_next

  mov ecx, [eax+0x14]
  cmp ecx, [scene_saved]
  je not_stale
  mov eax, esi
  shl eax, 2
  add eax, clone_ptrs
  mov dword [eax], 0
  jmp ai_next

not_stale:
  cmp byte [eax+0x768], 0
  je ai_skip_dead

  cmp byte [eax+0x2F8], 0
  je no_respawn_reset
  mov dword [eax+0x808], 0
  mov dword [eax+0x288], 0
  mov byte [eax+0x2F8], 0
  mov byte [eax+0x324], 0
  mov dword [eax+0x170], 0
  mov dword [eax+0x174], 0
  mov dword [eax+0x178], 0

no_respawn_reset:
  mov edx, eax
  mov dword [edx+0xC6C], 0x4B7A5E35
  mov dword [edx+0xC70], 0x4B7A5E35
  mov dword [edx+0xC7C], 0x42480000

  mov ecx, esi
  shl ecx, 2
  add ecx, clone_pidx
  mov edi, [ecx]

  mov ecx, [scene_saved]
  mov ecx, [ecx+0x878]
  test ecx, ecx
  jz ai_skip_dead
  mov edx, edi
  imul edx, edx, 0xA0
  mov ebx, [ecx+edx+0x5DC]
  mov [saved_ball_ptr], ebx

  mov eax, esi
  shl eax, 2
  add eax, clone_ptrs
  mov eax, [eax]
  mov [ecx+edx+0x5DC], eax

  mov edx, edi
  shl edx, 2
  mov ebx, [ecx+edx+0xB28]
  mov [saved_ctrl_mode], ebx
  mov dword [ecx+edx+0xB28], 0x63

  push edi
  mov eax, force_buf
  push eax
  mov ecx, [scene_saved]
  call 004222D0

  mov eax, esi
  shl eax, 2
  add eax, clone_ptrs
  mov eax, [eax]
  cmp dword [eax+0x288], 0
  jne has_target
  mov dword [force_buf], 0
  mov dword [force_buf+4], 0
  mov dword [force_buf+8], 0
  mov dword [eax+0x808], 0
  jmp restore_table

has_target:

restore_table:
  mov ecx, [scene_saved]
  mov ecx, [ecx+0x878]
  mov edx, edi
  imul edx, edx, 0xA0
  mov ebx, [saved_ball_ptr]
  mov [ecx+edx+0x5DC], ebx

  mov edx, edi
  shl edx, 2
  mov ebx, [saved_ctrl_mode]
  mov [ecx+edx+0xB28], ebx

  mov eax, esi
  shl eax, 2
  add eax, clone_ptrs
  mov eax, [eax]
  mov ecx, eax
  mov edx, [eax]
  push dword [force_buf+8]
  push dword [force_buf+4]
  push dword [force_buf]
  call dword [edx+0x14]

ai_skip_dead:
ai_next:
  inc esi
  jmp ai_loop

ai_done:
  pushad
  xor ecx, ecx
  xor edx, edx

compact_loop:
  cmp ecx, [clone_count]
  jae compact_done
  mov eax, ecx
  shl eax, 2
  add eax, clone_ptrs
  mov eax, [eax]
  test eax, eax
  jz compact_skip
  mov ebx, [eax+0x14]
  cmp ebx, [scene_saved]
  jne compact_skip
  mov eax, ecx
  shl eax, 2
  add eax, clone_ptrs
  mov ebx, [eax]
  mov eax, edx
  shl eax, 2
  add eax, clone_ptrs
  mov [eax], ebx
  mov eax, ecx
  shl eax, 2
  add eax, clone_pidx
  mov ebx, [eax]
  mov eax, edx
  shl eax, 2
  add eax, clone_pidx
  mov [eax], ebx
  inc edx
compact_skip:
  inc ecx
  jmp compact_loop

compact_done:
  mov [clone_count], edx
  popad
  popad

original_code:
  push ebx
  push ebp
  push esi
  mov ebx, ecx
  push edi
  jmp 0041B546

SpawnClone:
  mov dword [src_ball], 0

  mov ecx, [scene_saved]
  test ecx, ecx
  jz spawn_skip

  mov ecx, [ecx+0x878]
  test ecx, ecx
  jz spawn_skip

  mov edx, [player_idx]
  imul edx, edx, 0xA0
  mov eax, [ecx+edx+0x5DC]
  test eax, eax
  jz use_p1_fallback

  cmp byte [eax+0x768], 0
  jz use_p1_fallback

  mov [src_ball], eax
  jmp spawn_found

use_p1_fallback:
  mov ecx, [scene_saved]
  mov ecx, [ecx+0x878]
  mov eax, [ecx+0x5DC]
  test eax, eax
  jz no_fallback

  cmp byte [eax+0x768], 0
  jz no_fallback

  mov [src_ball], eax
  jmp spawn_found

no_fallback:
  jmp spawn_skip

spawn_found:
  mov eax, [clone_count]
  cmp eax, 8
  jae spawn_skip

  push 0xC98
  call 004BA57B
  add esp, 4
  test eax, eax
  jz spawn_skip
  mov [new_ball], eax

  mov edi, eax
  xor eax, eax
  mov ecx, 0x326
  cld
  rep stosd

  mov ecx, [new_ball]
  mov edx, [scene_saved]
  push edx
  call 004039E0

  mov ecx, [new_ball]
  mov edx, [ecx]
  call dword [edx+4]

  mov edx, [new_ball]
  mov eax, [player_idx]
  mov [edx+0x18], eax
  mov dword [edx+0x284], 0x41D00000
  mov dword [edx+0x188], 0x40A00000
  mov dword [edx+0x278], 0x3F000000
  mov dword [edx+0x27C], 0x3DCCCCCD
  mov dword [edx+0x1A0], 0x3F866666
  mov dword [edx+0xC6C], 0x4B7A5E35
  mov dword [edx+0xC70], 0x4B7A5E35
  mov dword [edx+0xC7C], 0x42480000
  mov dword [edx+0xC78], 0

  mov eax, [src_ball]
  test eax, eax
  jz spawn_skip

  fld dword [eax+0x164]
  fadd dword [004CF55C]
  fstp dword [edx+0x164]
  fld dword [eax+0x168]
  fadd dword [eax+0x284]
  fstp dword [edx+0x168]
  fld dword [eax+0x16C]
  fadd dword [004CF55C]
  fstp dword [edx+0x16C]

  mov ebx, [eax+0x170]
  mov [edx+0x170], ebx
  mov ebx, [eax+0x174]
  mov [edx+0x174], ebx
  mov ebx, [eax+0x178]
  mov [edx+0x178], ebx

  mov esi, eax
  mov edi, edx
  add esi, 0x1B8
  add edi, 0x1B8
  mov ecx, 0x29
  cld
  rep movsd

  mov esi, eax
  mov edi, [new_ball]
  mov ebx, [esi+0x1BC]
  mov [edi+0x1BC], ebx
  mov ebx, [esi+0x1C0]
  mov [edi+0x1C0], ebx
  mov ebx, [esi+0x1C4]
  mov [edi+0x1C4], ebx
  mov ebx, [esi+0x1C8]
  mov [edi+0x1C8], ebx
  mov ebx, [esi+0x204]
  mov [edi+0x204], ebx

  mov esi, eax
  mov edi, [new_ball]
  mov ebx, [esi+0x2AC]
  mov [edi+0x2AC], ebx
  mov ebx, [esi+0x2B0]
  mov [edi+0x2B0], ebx
  mov ebx, [esi+0x2B4]
  mov [edi+0x2B4], ebx
  mov ebx, [esi+0x2B8]
  mov [edi+0x2B8], ebx

  mov eax, [new_ball]
  mov ebx, [eax+0x164]
  mov [eax+0xC60], ebx
  mov ebx, [eax+0x168]
  mov [eax+0xC64], ebx
  mov ebx, [eax+0x16C]
  mov [eax+0xC68], ebx

  mov edx, [new_ball]
  mov byte [edx+0x768], 1
  mov byte [edx+0x769], 1
  mov byte [edx+0x2F9], 0
  mov byte [edx+0x324], 0
  mov dword [edx+0x808], 0
  mov dword [edx+0x288], 0

  mov ecx, [scene_saved]
  add ecx, 0x29D4
  push edx
  call 00453810

  mov ecx, [scene_saved]
  add ecx, 0x2DEC
  push edx
  call 00453810

  mov eax, [clone_count]
  shl eax, 2
  mov ecx, [new_ball]
  mov [clone_ptrs+eax], ecx
  mov ecx, [player_idx]
  mov [clone_pidx+eax], ecx
  inc dword [clone_count]

spawn_skip:
  ret

0041B540:
  jmp CloneCode
  nop

[DISABLE]

0041B540:
  db 53 55 56 8B D9 57

// Restore original Y-check
0042238A:
  db 74 0E

dealloc(CloneCode)
dealloc(scene_saved)
dealloc(spawnP1)
dealloc(spawnP2)
dealloc(spawnP3)
dealloc(spawnP4)
dealloc(player_idx)
dealloc(src_ball)
dealloc(new_ball)
dealloc(clone_count)
dealloc(clone_ptrs)
dealloc(clone_pidx)
dealloc(force_buf)
dealloc(saved_ball_ptr)
dealloc(saved_ctrl_mode)
dealloc(tmp_val)

unregistersymbol(scene_saved)
unregistersymbol(spawnP1)
unregistersymbol(spawnP2)
unregistersymbol(spawnP3)
unregistersymbol(spawnP4)

```
