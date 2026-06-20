/*
 * Function: GameLevel_ctor (MISNAMED — actually a Spinner/Obstacle constructor)
 * Address: 0x004351F0
 * Signature: void * __thiscall GameLevel_ctor(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, void *param_5)
 * Parameters:
 *   this: void* — the obstacle object being constructed (Spinner-type, vtable = PTR_Spinner_DeletingDtor_004D53F8)
 *   param_1: undefined4 — stored at this+0x10D0 (parent Scene pointer)
 *   param_2: undefined4 — stored at this+0x10D8 (position X or rotation)
 *   param_3: undefined4 — stored at this+0x10DC (position Y or rotation)
 *   param_4: undefined4 — stored at this+0x10E0 (position Z or rotation)
 *   param_5: void* — passed to Stands_ctor as parent object (used to access App→MeshWorld via +0x878→+0x5C8)
 *
 * Description:
 * Constructor for spinner/obstacle game level objects. Despite Ghidra's name
 * "GameLevel_ctor", the vtable assignment (PTR_Spinner_DeletingDtor at 0x4D53F8)
 * reveals this is actually a Spinner-family constructor. There is a separate
 * Spinner_Level_ctor at 0x4396F0 with a different signature.
 *
 * Steps:
 *   1. Calls Stands_ctor(this, param_5) — inherits from Stands base class
 *   2. Sets vtable to PTR_Spinner_DeletingDtor (0x4D53F8) — identifies as Spinner
 *   3. Initializes AthenaList at +0x1108 (for sub-objects)
 *   4. Stores parent Scene pointer at +0x10D0
 *   5. Stores position/rotation data at +0x10D8/+0x10DC/+0x10E0
 *   6. Creates a CollisionLevel (0x10D0 bytes) via operator_new + CollisionLevel_ctorWithLevel
 *   7. Copies field +0x434 to CollisionLevel+0x434 (arena/difficulty info)
 *   8. Sets obstacle state: +0x10E4=1 (active), +0x10E8=0x41200000 (float 10.0, radius?),
 *      +0x10EC=0x41200000 (float 10.0), +0x1100=0x43160000 (float 150.0, rotation speed?)
 *   9. Gets a sound channel from App→Scene+0x10D0→+0x878→+0x4E0 and stores at +0x1520
 *   10. Initializes visibility flags at +0x1104 and +0x1105 to 0
 *
 * Called 8 times from Scene_SetupLevel7 (Neon Race) — once per spinner obstacle placed.
 * Also called from CreateWobbly1 — used for Wobbly Race obstacles.
 *
 * Struct offsets:
 *   +0x10D0: parent Scene pointer
 *   +0x10D4: CollisionLevel pointer (owned, 0x10D0 bytes)
 *   +0x10D8/+0x10DC/+0x10E0: position/rotation
 *   +0x10E4: active state (1)
 *   +0x10E8/+0x10EC: radius/size (10.0f each)
 *   +0x1100: rotation speed? (150.0f)
 *   +0x1108: AthenaList (sub-objects)
 *   +0x1520: sound channel ID
 *
 * Cross-references:
 *   8 calls from Scene_SetupLevel7 (0x40F4A1, 0x40F52F, 0x40F696, 0x40F742, 0x40F7E5, 0x40F888, 0x40F92B)
 *   1 call from CreateWobbly1 (0x4154DF)
 *
 * Name verification: Ghidra labels this "GameLevel_ctor" but the vtable (PTR_Spinner_DeletingDtor)
 * proves it's a Spinner-type constructor. This is NOT the same as Spinner_Level_ctor (0x4396F0)
 * which has a different signature with float param_5.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
