# "Press S to spawn drawbridge GLOBALLY"

**CT Entry ID:** 85

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

// =====================================================
// Global Drawbridge Spawner (Tower Race Drawbridges)
// =====================================================
// Spawns Tower Race drawbridges at Player 1's position in ANY level.
// Uses JIT mesh injection: saves board+0x4378, writes drawbridge mesh,
// calls Spinner_Level_ctor, restores board+0x4378.
//
// FIX v2: board+0x4378 is a REUSED SLOT across levels:
//   Tower: 0x4378 = Level4-Mace mesh (already loaded by board ctor)
//   Beginner: 0x4378 = 0 (unused) — old script worked here
//   Other races: 0x4378 may hold other level data — crash
// Old script wrote mesh to 0x4378 permanently (never restored),
// corrupting the board's mesh slot for other objects.
// Fix: save/restore 0x4378 around the ctor call (JIT injection).
// =====================================================

alloc(SpawnCode, 8192)
alloc(SpawnDrawbridge, 4)
alloc(saved_board, 4)
alloc(playerX, 4)
alloc(playerY, 4)
alloc(playerZ, 4)
alloc(float_30, 4)
alloc(float_20_sub, 4)
alloc(temp_mesh, 4)
alloc(drawbridge_array, 68)
alloc(last_board, 4)
alloc(saved_slot_4378, 4)

registersymbol(SpawnDrawbridge)

SpawnDrawbridge:
 dd 0

saved_board:
 dd 0

playerX:
 dd 0

playerY:
 dd 0

playerZ:
 dd 0

float_30:
 dd 0x41F00000  // 30.0

float_20_sub:
 dd 0xC1A00000  // -20.0

temp_mesh:
 dd 0

drawbridge_array:
 dd 0
 dd 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

last_board:
 dd 0

saved_slot_4378:
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

 mov eax, [esi+0x14]
 test eax, eax
 jz no_clear
 cmp eax, [last_board]
 je no_clear
 mov [last_board], eax
 mov dword [drawbridge_array], 0

no_clear:
 mov eax, [drawbridge_array]
 test eax, eax
 jz save_pos
 pushad
 xor ebx, ebx

// === PROXIMITY LOOP: Update each drawbridge ===
prox_loop:
 cmp ebx, [drawbridge_array]
 jae prox_done
 mov eax, [drawbridge_array+4+ebx*4]
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

 // Call vtable[0x2C] (update/animation, __fastcall, ECX=this)
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
 fld dword [esi+0x164]
 fadd dword [float_30]
 fstp dword [playerX]
 fld dword [esi+0x168]
 fadd dword [float_20_sub]
 fstp dword [playerY]
 mov eax, [esi+0x16C]
 mov [playerZ], eax

check_spawn:
 cmp dword [SpawnDrawbridge], 0
 je original_code
 cmp dword [esi+0x18], 0
 jne original_code
 mov dword [SpawnDrawbridge], 0

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

// === JIT MESH INJECTION ===
// 1. Load drawbridge mesh into temp_mesh (separate allocation)
// 2. Save board+0x4378 (may contain another level's mesh)
// 3. Write temp_mesh to board+0x4378
// 4. Call Spinner_Level_ctor (reads board+0x4378 internally)
// 5. Restore board+0x4378 to original value

do_mesh:
 // Always create a fresh MeshWorld for the drawbridge mesh
 push 0x10D0
 call 004BA57B
 add esp, 4
 test eax, eax
 jz done_spawn

 mov edx, [saved_board]
 mov edx, [edx+0x878]
 mov edx, [edx+0x174]
 push 004D099C           // "Levels\Level4-Drawbridge"
 push edx                // D3D device
 mov ecx, eax            // this = new MeshWorld
 call 00461510           // MeshWorld_ctor
 test eax, eax
 jz done_spawn
 mov [temp_mesh], eax

 // Allocate drawbridge object (0x10FC bytes)
 push 0x10FC
 call 004BA57B
 add esp, 4
 test eax, eax
 jz done_spawn
 mov edi, eax

 // === SAVE board+0x4378 ===
 mov edx, [saved_board]
 mov ecx, [edx+0x4378]     // save original value
 mov [saved_slot_4378], ecx

 // === INJECT drawbridge mesh into board+0x4378 ===
 mov ecx, [temp_mesh]
 mov [edx+0x4378], ecx

 // Spinner_Level_ctor(this, board, X, Y, Z, rotation)
 // __thiscall: ECX=this, 5 stack params, RET 0x14
 // Push order (right-to-left): rotation, Z, Y, X, board
 push 0                  // param_5 = rotation (0)
 mov edx, [playerZ]
 push edx                // param_4 = Z
 mov edx, [playerY]
 push edx                // param_3 = Y
 mov edx, [playerX]
 push edx                // param_2 = X
 mov edx, [saved_board]
 push edx                // param_1 = board
 mov ecx, edi            // this = new drawbridge
 call 004396F0           // Spinner_Level_ctor

 // === RESTORE board+0x4378 ===
 mov edx, [saved_board]
 mov ecx, [saved_slot_4378]
 mov [edx+0x4378], ecx   // restore original value

 test eax, eax
 jz done_spawn
 mov edi, eax

 // Register in board+0x2578 (active objects list)
 push edi
 mov ecx, [saved_board]
 add ecx, 0x2578
 call 00453810

 // Register in board+0xCD4 (collision objects list)
 push edi
 mov ecx, [saved_board]
 add ecx, 0xCD4
 call 00453810

 // Register collision level in board+0x10EC
 mov eax, [edi+0x10F4]
 test eax, eax
 jz skip_col
 push eax
 mov ecx, [saved_board]
 add ecx, 0x10EC
 call 00453810

 // Register collision level in board+0x8B0+0x18
 mov edx, [saved_board]
 mov edx, [edx+0x8B0]
 test edx, edx
 jz skip_col
 mov eax, [edi+0x10F4]
 push eax
 lea ecx, [edx+0x18]
 call 00453810

skip_col:
 // Register in render list (board+0x8AC→+0x480→+0x1C)
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
 // Add to our tracking array (max 16)
 mov eax, [drawbridge_array]
 cmp eax, 16
 jae done_spawn
 mov [drawbridge_array+4+eax*4], edi
 inc dword [drawbridge_array]

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
dealloc(SpawnDrawbridge)
dealloc(saved_board)
dealloc(playerX)
dealloc(playerY)
dealloc(playerZ)
dealloc(float_30)
dealloc(float_20_sub)
dealloc(temp_mesh)
dealloc(drawbridge_array)
dealloc(last_board)
dealloc(saved_slot_4378)
unregistersymbol(SpawnDrawbridge)

```
