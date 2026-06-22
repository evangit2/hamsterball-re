# Ball Collision System Analysis

## Verdict: Collisions Are ASYMMETRIC Per-Event (Definitively Proven via Decompilation)

Each collision event generates a collision entry in **only ONE ball's** collision
list. The detecting ball processes the collision (push-apart, velocity exchange)
and writes velocity changes to BOTH balls' physics objects — but only the
detecting ball gets a collision entry. The other ball does not receive an entry
for this event.

Both balls independently detect the same contact across separate `Ball_Update`
calls (which run sequentially within `Scene_UpdateBallsAndState`), but each
detection is a separate event at a different tick. This is why runtime dumps
show entries at different timestamps with unequal counts (e.g., 4 player
entries vs 7 eight-ball entries).

---

## How the Collision System Works (Full Call Chain)

### 1. Scene_UpdateBallsAndState (0x41B540)

Iterates the ball list at `Scene+0x29D4` and calls `vtable[4]` (`Ball_Update`)
for each ball sequentially:

```c
while (ball = AthenaList_Next(scene+0x29D4)) {
    Scene_SetCamera(scene, ball, 1);
    (*ball->vtable[4])();  // Ball_Update
}
```

**No ball-ball collision detection happens here.** It just calls each ball's
update function. Collision detection happens INSIDE each ball's `Ball_Update`.

### 2. Ball_Update (0x405E00) — Collision Setup

Each ball's `Ball_Update` creates its own collision detection objects:

#### Phase A: Create SpatialTree node (for wall/floor collisions)

```asm
00406898: PUSH 0x20              ; allocate 32 bytes
0040689a: CALL operator_new
004068bc: CALL SpatialTree_ctor   ; 0x463330 — vtable = 0x4D9038
004068c5: MOV [EAX+0x10], 0xa    ; type = 10 (spatial)
004068cc: MOV EDX, [ESI+0x278]   ; ball+0x278 = geometry data
004068d2: MOV [EAX+0xc], EDX     ; node+0xc = geometry
004068db: MOV [ESP+0x1c], EAX    ; save SpatialTree ptr → [ESP+0x1c]
```

If `ball+0x80c > 0`, the SpatialTree is added to the collision mesh:
```asm
004068e9: MOV ECX, [ESI+0x1a4]   ; ECX = collision_mesh
004068f1: PUSH EAX               ; push SpatialTree
004068f2: CALL [EDX+0x14]        ; vtable[5] = CollisionMesh_AddTriangle
```

`CollisionMesh_AddTriangle` (0x456120) appends to `collision_mesh+0x430`:
```c
void CollisionMesh_AddTriangle(void *this, int node) {
    AthenaList_Append(this + 0x430, node);
    *(void **)(node + 8) = this;  // back-pointer to mesh
}
```

#### Phase B: Create CollisionNode (for ball-ball collisions)

```asm
004068f5: PUSH 0x14              ; allocate 20 bytes
004068f7: CALL operator_new
0040690f: MOV ECX, [ESI+0x14]   ; ECX = Scene (ball+0x14)
00406912: ADD ECX, 0x29d4        ; ECX = Scene+0x29D4 (ball list)
00406918: PUSH ECX
0040691b: CALL CollisionNode_ctor ; 0x466CF0
0040692c: MOV [ESP+0x18], EAX    ; save CollisionNode ptr → [ESP+0x18]
```

`CollisionNode_ctor` stores the ball list reference:
```c
void CollisionNode_ctor(void *this, undefined4 param_1) {
    CollisionNode_BaseInit(this, param_1);
    // vtable = 0x4D9128
    *(this + 0x0c) = 0x3dcccccd;  // radius scale
    *(this + 0x10) = 0x3dcccccd;
}

void CollisionNode_BaseInit(void *this, undefined4 param_1) {
    *this = &vtable_0x4DA65C;  // base vtable
    *(this + 4) = param_1;       // ← CollisionNode+0x04 = Scene+0x29D4 (ball list!)
}
```

If `ball+0x324 == 0` (not in special state), the CollisionNode is also added
to the collision mesh via `vtable[5]`.

#### Phase C: Run collision detection (vtable[1])

```asm
004069d5: MOV ECX, [ESI+0x1a4]   ; ECX = collision_mesh (this)
004069e4: MOV EDX, [ECX]         ; EDX = vtable
004069e7: CALL [EDX+0x4]         ; vtable[1] = Ball_AdvancePositionOrCollision
```

`Ball_AdvancePositionOrCollision` (0x4564C0) calls `vtable[7]` internally,
which does the actual collision detection.

### 3. CollisionMesh vtable[7] (0x456890) — Collision Detection Engine

This function iterates all nodes in `collision_mesh+0x430` (the SpatialTree
and CollisionNode added above) and tests each against the ball's movement:

```c
void CollisionMesh_Detect(void *this, float *out_pos, float *in_pos,
                          float *in_dir, float *in_scale, ...) {
    do {
        Material_Init(&local_material);  // zero 0x68-byte struct
        
        // Iterate all nodes in +0x430 list
        for each node in AthenaList(this+0x430):
            node->vtable[2](this, aabb, &local_material);  // collision test
        
        if (local_material.type_field == 0) break;  // no collision
        
        *param_7 = 1;  // collision flag
        
        // Call resolution function
        node->vtable[3](this, &result_pos, this, &in_dir, &local_material);
        
        // Create entry and append to collision list
        entry = operator_new(0x68);
        Material_Init(entry);
        Material_Copy(entry, &local_material);  // copy all fields
        AthenaList_Append(this + 0x18, entry);  // ← ONLY to THIS mesh's list!
        
    } while (iteration_count < this+0xc60);
}
```

**Critical**: `AthenaList_Append(this + 0x18, entry)` appends to **only this
ball's** collision list. There is no second `AthenaList_Append` call to the
other ball's list.

### 4. CollisionNode vtable[2] (0x467030) — Ball-Ball Collision Test

This is the function that tests THIS ball against ALL other balls:

```c
void CollisionNode_Test(int *this, int collision_mesh, float *aabb, int material) {
    int ball_count = AthenaList_GetSize(this[1]);  // this+0x04 = ball list
    
    for (int i = 0; i < ball_count; i++) {
        int *other_ball = AthenaList_Get(this[1], i);
        
        // Self-collision skip: compare mesh IDs
        if (*(collision_mesh + 0x10) != *(other_ball->collision_mesh + 0x10)) {
            // Get other ball position and radius
            float ox = other_ball[0x164];  // pos X
            float oy = other_ball[0x168];  // pos Y
            float oz = other_ball[0x16c];  // pos Z
            float r  = other_ball[0x284];  // radius
            
            // AABB overlap test
            if (aabb_overlap(aabb, sphere(ox, oy, oz, r))) {
                // Detailed collision test
                char hit = this->vtable[4](aabb, ..., &collision_t);
                
                if (hit && (material->type == 0 || collision_t < material->t)) {
                    material->t = collision_t;
                    material->subtype = 4;          // +0x04 = 4
                    material->source_ref = this;     // +0x64 = CollisionNode ptr
                    material->other_ball = other_ball; // +0x0C = other ball ptr
                    material->material_ptr = 0;       // +0x10 = NULL
                    // +0x14 = contact point (XYZ)
                    // +0x20 = normal (XYZ, negated)
                    // +0x58 = previous point
                }
            }
        }
    }
}
```

**Key**: The `+0x64` field (`source_ref`) is set to `this` — the CollisionNode
pointer. This CollisionNode was created by THIS ball's `Ball_Update`, so only
this ball will match the `+0x64` check in the collision processing loop.

### 5. CollisionNode vtable[3] (0x467360) — Ball-Ball Collision Resolution

This function is called AFTER vtable[2] detects a collision. It finalizes the
entry and applies velocity changes to BOTH balls:

```c
void CollisionNode_Resolve(int this, float *out_pos, int collision_mesh,
                          int dir, undefined4 *material) {
    // Set entry type to BALL-BALL
    *material = 1;  // +0x00 = 1 (ball-ball type!)
    
    // Compute reflection/deflection
    float angle = asin(...);
    material[0x0b] = 1.0 - 2.0 * angle * angle;  // +0x2C
    
    // Copy relative velocity into entry
    material[0x0c] = current_vel.x;  // +0x30 = rel_vel X
    material[0x0d] = current_vel.y;  // +0x34 = rel_vel Y
    material[0x0e] = current_vel.z;  // +0x38 = rel_vel Z
    
    // Get other ball's collision mesh
    int other_ball = material[3];              // +0x0C = other_ball
    int other_mesh = *(other_ball + 0x1a4);    // other_ball+0x1A4
    
    // Write velocity to THIS ball's mesh
    *(collision_mesh + 0xca4) = push_vel.x * factor_a;
    *(collision_mesh + 0xca8) = push_vel.y * factor_a;
    *(collision_mesh + 0xcac) = push_vel.z * factor_a;
    
    // Write velocity to OTHER ball's mesh
    *(other_mesh + 0xca4) = push_vel2.x * factor_b;
    *(other_mesh + 0xca8) = push_vel2.y * factor_b;
    *(other_mesh + 0xcac) = push_vel2.z * factor_b;
}
```

**This is the critical finding**: The resolution function writes velocity changes
to BOTH balls' collision meshes (`+0xCA4/+0xCA8/+0xCAC`), but only ONE entry
is created (in the detecting ball's list).

### 6. Ball_Update Collision Processing Loop

After `vtable[1]` returns, `Ball_Update` iterates the collision entries:

```c
// Clear old entries from +0x430 list
collision_mesh->vtable[6]();  // AthenaList_Free(+0x430)

// Iterate entries in +0x18 list (populated by vtable[1])
for each entry in AthenaList(collision_mesh + 0x18):
    
    // Phase 1: Wall collision (type == 2)
    if (entry->type == 2 && entry->source_ref == spatialtree_node) {
        // Update camera focus point
    }
    
    // Phase 2: Ball-ball trajectory (type == 1, source_ref == spatialtree)
    // DEAD CODE: ball-ball entries have source_ref == CollisionNode, 
    //            never == SpatialTree. This never fires for ball-ball.
    if (entry->type == 1 && entry->source_ref == spatialtree_node) {
        Ball_ApplyTrajectory(ball);
        // camera/sound stuff
    }
    
    // Phase 3: Ball-ball physics (type == 1, source_ref == collisionnode)
    if (entry->type == 1 && entry->source_ref == collisionnode) {
        other_ball = entry->other_ball;  // +0x0C
        // Push-apart: compute distance, apply separation
        // Velocity exchange: read both balls' positions
        // Sound: play collision sound if rel_vel > threshold
        // Camera: call vtable[8] for camera focus
    }
```

#### Disasm Verification of +0x64 Check

The decompiler incorrectly mapped `unaff_EBP` and `unaff_ESI` to the collision
checks. The actual disassembly shows:

```asm
; EBP = current collision entry (piVar16)
; ESI = ball pointer (set at 00405e20: MOV ESI,ECX)
; [ESP+0x1c] = SpatialTree node (set at 004068db)
; [ESP+0x18] = CollisionNode (set at 0040692c)

; Phase 1 (type == 2, wall):
00406b80: CMP [EBP], 0x2              ; entry->type == 2?
00406b8a: CMP [EBP+0x64], EDX         ; entry+0x64 == [ESP+0x1c] (SpatialTree)?

; Phase 2 (type == 1, trajectory — DEAD CODE for ball-ball):
00406bd3: CMP [EBP], 0x1              ; entry->type == 1?
00406be1: CMP [EBP+0x64], EDX         ; entry+0x64 == [ESP+0x1c] (SpatialTree)?

; Phase 3 (type == 1, physics):
00406c79: CMP [EBP], 0x1              ; entry->type == 1?
00406c87: CMP [EBP+0x64], ECX         ; entry+0x64 == [ESP+0x18] (CollisionNode)?
00406c90: MOV EDI, [EBP+0xc]          ; other_ball = entry+0x0C
```

**Ghidra decompiler error**: The decompiler mapped `[ESP+0x18]` (loaded into
ECX at 0x00406c83) to `unaff_ESI`. This is wrong — `ESI` holds the ball pointer
(set at `MOV ESI,ECX` in the prologue), but `ECX` at 0x00406c83 is loaded from
`[ESP+0x18]` (the CollisionNode). The actual comparison is `entry+0x64 ==
CollisionNode`, not `entry+0x64 == ball`.

---

## Collision Entry Struct (0x68 bytes)

| Offset | Type | Name | Set By | Evidence |
|--------|------|------|--------|----------|
| +0x00 | int | **type** | vtable[3] (0x467360): `*material = 1` | `0x00000001` for ball-ball, `0x00000005` for floor (set in Ball_AdvancePositionOrCollision: `*puVar7 = 5`) |
| +0x04 | int | **subtype** | vtable[2] (0x467030): `*(param_4+4) = 4` | Always 4 for ball-ball, 1 for wall, 0 for floor |
| +0x08 | float | **collision_t** | vtable[2]: `*(param_4+8) = fStack_8c` | Distance ratio along movement ray |
| +0x0C | ptr | **other_ball** | vtable[2]: `*(param_4+0xc) = iVar3` (ball from list) | Verified in dump: points to other ball's address |
| +0x10 | ptr | **material_ptr** | vtable[2]: `*(param_4+0x10) = 0` | NULL for ball-ball, set for wall entries |
| +0x14 | float[3] | **contact_point** | vtable[2]: `Vec3_CopyUnlessEqual(param_4+0x14, ...)` | World-space XYZ of contact |
| +0x20 | float[3] | **normal** | vtable[2]: `Vec3_CopyUnlessEqual(param_4+0x20, ...)` | Contact normal, negated (points away from other ball) |
| +0x2C | float | **deflection** | vtable[3]: `material[0x0b] = fVar11` | Angle-based deflection factor |
| +0x30 | float[3] | **rel_vel** | vtable[3]: `material[0xc..0xe]` | Relative velocity at contact point |
| +0x3C–0x50 | — | **unused** | Material_Init zeroes | Always zero for ball-ball |
| +0x54 | float | **penetration** | Not set for ball-ball | Only used for type==5 (floor) |
| +0x58 | float[3] | **prev_point** | vtable[2]: `Vec3_CopyUnlessEqual(param_4+0x58, ...)` | Previous contact point |
| +0x64 | ptr | **source_ref** | vtable[2]: `*(param_4+100) = param_1` (CollisionNode) | Ownership token: which CollisionNode created this entry |
| +0x68–0x7C | — | **trailing data** | Not part of Material struct (0x68 bytes) | Appears in dump as adjacent heap data |

**Note**: The entry struct is 0x68 bytes (allocated by `operator_new(0x68)`),
NOT 0x80 bytes. The dump shows 0x80 bytes because the logging code reads
past the allocation boundary. Fields +0x68–0x7C are adjacent heap data, not
part of the entry struct.

---

## CollisionMesh vtable

Vtable at `0x4D8E10` (set by `CollisionMesh_ctor` at 0x456D80):

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| 0 | 0x00 | 0x456870 | Mesh dtor |
| 1 | 0x04 | 0x4564C0 | Ball_AdvancePositionOrCollision — move + detect collisions |
| 2 | 0x08 | 0x456280 | Unknown |
| 3 | 0x0C | 0x456CD0 | Ball_InitBattleMode — battle mode setup |
| 4 | 0x10 | 0x456110 | Unknown |
| 5 | 0x14 | 0x456120 | CollisionMesh_AddTriangle — add node to +0x430 list |
| 6 | 0x18 | 0x456140 | AthenaList_Free(+0x430) — clear node list |
| 7 | 0x1C | 0x456890 | Collision detection engine — test nodes, create entries |
| 8 | 0x20 | 0x457A20 | Unknown |

## CollisionNode vtable

Vtable at `0x4D9128` (set by `CollisionNode_ctor` at 0x466CF0):

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| 0 | 0x00 | 0x466D50 | dtor |
| 1 | 0x04 | 0x466CD0 | Unknown |
| 2 | 0x08 | 0x467030 | **Ball-ball collision test** — iterate ball list, AABB test |
| 3 | 0x0C | 0x467360 | **Ball-ball collision resolve** — set type=1, exchange velocities |
| 4 | 0x10 | 0x466D70 | Sphere-ray intersection test |
| 5 | 0x14 | 0x466F50 | Unknown |

## SpatialTree vtable

Vtable at `0x4D9038` (set by `SpatialTree_ctor` at 0x463330):

| Slot | Offset | Address | Function |
|------|--------|---------|----------|
| 2 | 0x08 | 0x463880 | SpatialTree_ForEach — iterate children, call their vtable[8] |
| 6 | 0x18 | 0x463500 | Unknown (called at 0x00406895 via vtable) |

---

## Collision List Location

| Offset from Ball | Offset from CollisionMesh | Field |
|------------------|---------------------------|-------|
| Ball + 0x1A4 | — | `collision_mesh_ptr` (pointer to CollisionMesh) |
| — | collision_mesh + 0x10 | `mesh_id` (unique ID for self-collision skip) |
| — | collision_mesh + 0x18 | `entry_list` (AthenaList of collision entries) |
| — | collision_mesh + 0x1C | `entry_count` (AthenaList count) |
| — | collision_mesh + 0x424 | `entry_arr` (AthenaList data pointer) |
| — | collision_mesh + 0x430 | `node_list` (AthenaList of SpatialTree/CollisionNode) |
| — | collision_mesh + 0xC60 | `max_iterations` (collision detection loop limit) |
| — | collision_mesh + 0xC74 | `collision_time` (accumulated collision time) |
| — | collision_mesh + 0xCA4 | `velocity` (XYZ float, written by resolution) |
| — | collision_mesh + 0xCA8 | `velocity_y` |
| — | collision_mesh + 0xCAC | `velocity_z` |

**Note**: In the decompiler, `param_1[0x69]` means `*(int*)(ball + 0x69*4)` =
`*(int*)(ball + 0x1A4)`. The memory note "Ball + 0x69 (DWORD)" was using a
different indexing convention. The correct offset is **ball + 0x1A4** (byte offset).

---

## Stale Entry Problem

The AthenaList at `collision_mesh+0x18` is NOT cleared between frames for
entries that persist. Old collision entries remain with stale `+0x0C` pointers.
These stale entries have invalid `playerID` values at `other_ball + 0x18`.

### Fix

```c
int other_id = *(int*)((char*)other_ball + 0x18);
if (other_id != 0 && other_id != -1) continue;  // skip stale
```

---

## Modding Implications

### 1. Hook ALL Balls, Not Just the Player

Since each ball creates entries only in its own list, hooking only the player's
`Ball_Update` misses ~64% of collision events (based on the 4:7 ratio observed
in the dump). Hook `0x405E00` and process entries for every ball:

```c
void __fastcall Hooked_BallUpdate(Ball* ball, void* edx) {
    Original_BallUpdate(ball, edx);

    CollisionMesh* mesh = *(CollisionMesh**)((char*)ball + 0x1A4);
    int count = *(int*)((char*)mesh + 0x1C);
    if (count <= 0) return;

    void** entries = *(void***)((char*)mesh + 0x424);
    if (!entries || !*entries) return;

    for (int i = 0; i < count; i++) {
        DWORD* e = (DWORD*)entries[0][i];
        if (!e || IsBadReadPtr(e, 0x68)) continue;
        if (e[0] != 1) continue;  // ball-ball only (type at +0x00)

        DWORD other_ptr = e[3];  // +0x0C
        if (other_ptr <= 0x10000) continue;
        if (IsBadReadPtr((void*)other_ptr, 0x20)) continue;

        int other_id = *(int*)((char*)other_ptr + 0x18);
        if (other_id != 0 && other_id != -1) continue;

        mod->onBallBump(ball, (Ball*)other_ptr);
    }
}
```

### 2. Fire onBallBump for Both Participants

Since only the detecting ball has the entry, the other ball won't see this
collision event. For full coverage (e.g., if you need both balls to react),
fire the callback for both:

```c
mod->onBallBump(ball, (Ball*)other_ptr);      // detecting ball
mod->onBallBump((Ball*)other_ptr, ball);       // other ball (no entry, but was hit)
```

### 3. Contact Deduplication

The collision list persists entries across frames while balls remain in contact.
Use a per-ball touching set to fire `onBallBump` only on new contacts:

```c
static DWORD touching[MAX_BALLS][16] = {0};
```

### 4. Velocity Fields Are Shared

The resolution function writes to BOTH balls' velocity fields
(`collision_mesh+0xCA4/+0xCA8/+0xCAC`). If you modify one ball's velocity
after collision, the other ball may have already been affected. Read velocities
AFTER `Original_BallUpdate` returns to see post-collision values.

### 5. Phase 2 (Ball_ApplyTrajectory) Is Dead Code

The `if (type==1 && source_ref==SpatialTree)` check at 0x00406be1 never fires
for ball-ball entries because they have `source_ref == CollisionNode`. If you
need trajectory effects on ball-ball collisions, you must add them yourself
in the hook — the game's built-in trajectory code only runs for wall collisions.

### 6. Asymmetry Affects Detection Frequency

The 8-ball receives ~1.8x more collision entries than the player because:
- The 8-ball's `Ball_Update` runs after the player's in the iteration order
- By the time the 8-ball processes, the player may have already pushed it
- The 8-ball then detects the ongoing contact and creates its own entry
- The player, on its next update, may find the balls have separated

This means collision events are biased toward whichever ball runs second in
the `Scene_UpdateBallsAndState` iteration.

---

## Summary of Decompilation Chain

```
Scene_UpdateBallsAndState (0x41B540)
  └→ Ball_Update (0x405E00) [for each ball]
       ├→ Create SpatialTree node → [ESP+0x1c]
       ├→ Create CollisionNode (with Scene+0x29D4 ball list) → [ESP+0x18]
       ├→ CollisionMesh_AddTriangle (vtable[5]) — add both nodes to +0x430
       ├→ Ball_AdvancePositionOrCollision (vtable[1] = 0x4564C0)
       │    └→ CollisionMesh_Detect (vtable[7] = 0x456890)
       │         ├→ For each node in +0x430:
       │         │    ├→ SpatialTree_ForEach (vtable[2] = 0x463880) — wall/floor test
       │         │    │    └→ Recursively test child nodes (CollisionFace)
       │         │    └→ CollisionNode_Test (vtable[2] = 0x467030) — ball-ball test
       │         │         └→ For each ball in Scene+0x29D4:
       │         │              ├→ Skip self (compare mesh+0x10 IDs)
       │         │              ├→ AABB overlap test
       │         │              └→ On hit: set entry fields (+0x04, +0x08, +0x0C, +0x64)
       │         ├→ If collision detected:
       │         │    └→ CollisionNode_Resolve (vtable[3] = 0x467360)
       │         │         ├→ Set entry type = 1 (+0x00)
       │         │         ├→ Set rel_vel (+0x30)
       │         │         ├→ Write velocity to THIS mesh (+0xCA4/+0xCA8/+0xCAC)
       │         │         └→ Write velocity to OTHER mesh (+0xCA4/+0xCA8/+0xCAC)
       │         └→ Create 0x68-byte entry, Material_Copy, append to +0x18
       │              ⚠ ONLY to THIS ball's list — NOT the other ball's list
       ├→ AthenaList_Free(+0x430) (vtable[6]) — clear node list
       └→ Iterate entries in +0x18:
            ├→ Phase 1: type==2 && +0x64==SpatialTree → wall (camera focus)
            ├→ Phase 2: type==1 && +0x64==SpatialTree → DEAD CODE for ball-ball
            └→ Phase 3: type==1 && +0x64==CollisionNode → push-apart physics
```

---

*Document created from decompiled source analysis of Hamsterball.exe via GhidraMCP.
All function addresses, vtable layouts, and field offsets verified through
disassembly cross-referencing. The Ghidra decompiler's `unaff_EBP`/`unaff_ESI`
mapping was corrected against raw x86 disassembly.*
