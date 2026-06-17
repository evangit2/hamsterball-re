# 06 - Common Reverse-Engineering Patterns

## 1. Dead Code Detection

A setter is dead code if the field it writes is never read by the main update/render loop, or if it is overwritten every frame by a computed value.

**Method:**
1. Decompile the setter.
2. Find the main update function for that object.
3. Search all decompilations for reads of the same offset.
4. If zero reads → dead value.

Example: `Ball_SetSpeed` (historical misname) writes `CollisionMesh+0xC64` and `+0xC98`, but `Ball_Update` never reads them. The function is actually `CollisionMesh_SetFriction` and the fields are overwritten by physics.

## 2. Nested Object Verification

If a constructor does `operator_new(size) + SubObject_ctor(...)` and stores the result at `this + N`, that field is a pointer to a nested object.

**Pointer chain template:**
```cpp
void* ball = ...;
void** collisionMeshPtr = (void**)((DWORD)ball + 0x1A4);
void* collisionMesh = *collisionMeshPtr;
float* trueVelY = (float*)((DWORD)collisionMesh + 0xCA8);
```

## 3. Vtable-Based Function Discovery

Constructors always assign the vtable first. Parse vtable slots as DWORDs to find virtual method addresses.

```cpp
// Ball vtable at 0x4CF3A0
// [0] Ball_dtor      @ 0x4027F0
// [1] Ball_InitPhysicsDefaults @ 0x405100
// [2] probably Update  @ ...
```

A function reached only through vtable dispatch has **no direct CALLs**. Hook the function start directly or patch the vtable slot.

## 4. Object Spawning Level-Gating Pattern

Objects fall into two categories:

**Self-loading:** Constructor calls `MeshWorld_ctor(..., "hardcoded_path")`.
- Can be added to any level by patching `app+0x23C` mode gate.
- Examples: `Bonk`, `Bumper/Rebound`.

**Scene-dependent:** Constructor receives a pre-loaded mesh pointer from a `BoardLevel*_ctor`.
- Requires both the mode gate patch AND the level constructor to load the sub-mesh.
- Examples: `Tipper`, `Gluebie`, `BlockDawg`, `Catapult`, `BreakBridge`, `PopCylinder`.

## 5. Calling Game Functions from Injected Code

Most game member functions use `__thiscall`:
- `this` in ECX
- Remaining args on stack

```cpp
typedef void* (__thiscall *Ball_ctor_fn)(void* thisPtr, int scene);
auto BallCtor = (Ball_ctor_fn)0x40AFE0;
void* ball = BallCtor(allocatedMem, (int)scene);
```

Calling via `__cdecl` (all args on stack) will crash because ECX contains garbage.

## 6. The CRT Heap Trap

Never call the game's `operator_new` / `malloc` / `_free` from an injected DLL — the VS2003 Small Block Heap critical sections can deadlock or corrupt.

**Fix:** Read the private CRT heap handle and use `HeapAlloc`/`HeapFree` directly:

```cpp
HANDLE hCrtHeap = *(HANDLE*)0x005369C0;  // image base 0x400000 + RVA 0x1369C0
void* mem = HeapAlloc(hCrtHeap, 0, 0xC98);
```

For ASLR safety, compute the runtime address: `GetModuleHandle(NULL) + RVA`.

## 7. AI as Vtable Override

AI objects reuse the same struct as the player; behavior differences come from config flags and a vtable thunk:

1. Level loader writes AI config fields (e.g., `ball[0x31D] = 1` for `is_8ball`).
2. `Ball_Update` checks the flag and calls `Ball_ApplyForce` vtable slot with AI-computed input.
3. Look for string comparisons (`__stricmp`) against names like `BADBALL`, `CHASE`, `HOME`, `SIZE`, `SPINDISTANCE`.
