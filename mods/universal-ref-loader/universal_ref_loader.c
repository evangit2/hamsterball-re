/*
 * universal_ref_loader.c — Hamsterball DLL Mod v3
 *
 * Hooks the vtable[33] factory dispatch in Scene_CreateDynamicObjects.
 * When the original Board factory doesn't recognize a ref name, tries all
 * other Arena factories in sequence. Features:
 *
 *   - JIT mesh injection: loads meshes from disk into board slots on demand
 *   - Clone-on-return for static-mesh objects (WATERWHEEL, SWIRL, BRIDGE-base)
 *     via Level_CloneTree (0x466060) for multi-instance support
 *   - Difficulty gate bypass (temporarily sets App+0x23C = 1)
 *   - Board slot safety checks (prevents crashes from unloaded meshes)
 *
 * Hook point: 0x0040C4BA (CALL [EAX+0x84] → CALL universal_factory)
 *   Original: FF 90 84 00 00 00  (6 bytes)
 *   Patched:  E8 XX XX XX XX 90  (CALL rel32 + NOP)
 *
 * All addresses verified via GhidraMCP decompilation + vtable reads (June 2026).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll universal_ref_loader.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ============================================================
 * Game constants (all verified via GhidraMCP)
 * ============================================================ */

#define HOOK_CALL_SITE     0x0040C4BA
#define HOOK_CALL_SITE_LEN 6

#define LEVEL_CLONE_TREE   0x00466060
#define MESHWORLD_CTOR    0x00461510
#define COLLISION_CTOR    0x00465080
#define OPERATOR_NEW      0x004BA57B
#define APP_SINGLETON      0x004FD680
#define APP_DIFFICULTY    0x23C
#define APP_D3D_DEVICE    0x174
#define BOARD_APP_PTR     0x878

/* Arena factory addresses (verified via vtable[33] reads) */
#define FACTORY_MASTER      0x004121D0
#define FACTORY_TOWER       0x0040D7C0
#define FACTORY_IMPOSSIBLE  0x00417FE0
#define FACTORY_EXPERT      0x0040E250
#define FACTORY_UP          0x004117B0
#define FACTORY_DIZZY       0x0040A5F0
#define FACTORY_BEGINNER    0x0040A550
#define FACTORY_WOBBLY      0x0040AD80
#define FACTORY_NEON        0x00416910
#define FACTORY_ODD         0x0040EC40
#define FACTORY_TOOB        0x0040FB30
#define FACTORY_GLASS       0x0040F420
#define FACTORY_SKY         0x00410AD0

/* ============================================================
 * Types
 * ============================================================ */

typedef void (__thiscall *FactoryFunc)(void* board, char* refName,
    void** outObj, void** outCol, int* refEntry);

typedef void* (__thiscall *CloneTreeFn)(void* mesh, int board);
typedef void* (__thiscall *MeshWorldCtorFn)(void* mem, int d3dDevice, const char* path);
typedef void* (__thiscall *CollisionCtorFn)(void* mem, int sourceMesh);
typedef void* (__cdecl *OperatorNewFn)(unsigned int size);

/* Factory entry: function pointer + board offsets that must be non-NULL */
typedef struct {
    FactoryFunc func;
    int slots[14];  /* board+0x4xxx offsets READ by this factory, 0 = end */
    const char* name;
} SafeFactory;

/* Mesh database entry: maps a board slot to its mesh file path */
typedef struct {
    int boardOffset;        /* e.g. 0x436C */
    const char* meshPath;   /* e.g. "Levels\\Level3-WaterWheel" */
    int needsCollision;     /* 1 = also create CollisionLevel at next slot */
    int collisionOffset;    /* where to store the CollisionLevel (e.g. 0x4370) */
} MeshEntry;

/* Cached loaded mesh */
typedef struct {
    int boardOffset;     /* which slot this was injected into */
    void* mesh;          /* loaded MeshWorld* */
    void* collision;     /* loaded CollisionLevel* (or NULL) */
} CachedMesh;

/* ============================================================
 * Globals
 * ============================================================ */

static BYTE  g_origBytes[6];
static BOOL  g_hooked = FALSE;
static CloneTreeFn      g_cloneTree     = (CloneTreeFn)LEVEL_CLONE_TREE;
static MeshWorldCtorFn  g_meshWorldCtor = (MeshWorldCtorFn)MESHWORLD_CTOR;
static CollisionCtorFn  g_collisionCtor = (CollisionCtorFn)COLLISION_CTOR;
static OperatorNewFn    g_operatorNew   = (OperatorNewFn)OPERATOR_NEW;

/* ============================================================
 * Mesh database
 *
 * Maps board slots to mesh file paths. When a factory needs a slot
 * that's NULL, we load the mesh from disk and inject it temporarily.
 * Meshes are cached so we only load each one once.
 *
 * Paths verified from decompiled board constructors:
 *   BoardLevel3_ctor (Dizzy): Level3-Tipper, Level3-WaterWheel, Level3-Swirl, Level3-Gluebie
 *   BoardLevel5_ctor (Tower): Level4-Catapult, Level4-Drawbridge, Level4-Mace, Level4-Windmill, Level4-Turret
 *   BoardLevel2_ctor (Intermediate): Level2-Bridge
 *   etc.
 * ============================================================ */

static const MeshEntry g_meshDB[] = {
    /* Dizzy Arena meshes */
    { 0x436C, "Levels\\Level3-Tipper",      1, 0x4370 },  /* Tipper mesh + collision */
    { 0x4394, "Levels\\Level3-Tipper",      1, 0x4398 },  /* Master also uses Tipper at different slot */
    { 0x4BA8, "Levels\\Level3-WaterWheel",  1, 0x4BAC },  /* WaterWheel mesh + collision */
    { 0x4BC4, "Levels\\Level3-Swirl",       1, 0x4BC8 },  /* Swirl mesh + collision */
    { 0x4374, "Levels\\Level3-Gluebie",      0, 0 },        /* Gluebie mesh only */
    { 0x607C, "Levels\\Level3-Gluebie",      0, 0 },        /* Master also stores Gluebie at 0x607C */

    /* Tower Arena meshes */
    { 0x436C, "Levels\\Level4-Catapult",     0, 0 },        /* Catapult mesh (Tower uses 0x436C too) */
    { 0x4370, "Levels\\Level4-Drawbridge",   0, 0 },        /* Drawbridge mesh */
    { 0x4378, "Levels\\Level4-Mace",         0, 0 },        /* Mace mesh */
    { 0x437C, "Levels\\Level4-Windmill",     0, 0 },        /* Windmill mesh */
    { 0x43B4, "Levels\\Level4-Turret",       0, 0 },        /* Turret mesh */

    /* Impossible Arena meshes */
    /* LOOPER uses 0x436C, GEAR uses 0x4370, BIGGEAR uses 0x4374, ROTATOR uses 0x4378, PENDULUM uses 0x437C */
    { 0x436C, "Levels\\LevelImpossible-Looper",   0, 0 },
    { 0x4370, "Levels\\LevelImpossible-Gear",     0, 0 },
    { 0x4374, "Levels\\LevelImpossible-BigGear",  0, 0 },
    { 0x4378, "Levels\\LevelImpossible-Rotator",   0, 0 },
    { 0x437C, "Levels\\LevelImpossible-Pendulum", 0, 0 },

    /* Up Arena meshes */
    { 0x4784, "Levels\\LevelUp-Lifter",        0, 0 },
    { 0x4788, "Levels\\LevelUp-SpeedCylinder", 0, 0 },
    { 0x478C, "Levels\\LevelUp-Button",        0, 0 },

    /* Expert Arena — Bonk self-loads, but Bridge/Judge/Bell have mesh deps in some paths */
    { 0x4378, "Levels\\Level5-Bridge",         0, 0 },   /* Expert Bridge uses 0x4378 */

    /* Master Arena meshes (different slots than Dizzy) */
    { 0x5410, "Levels\\Level10-Bridge1",       0, 0 },   /* BreakBridge1 */
    { 0x5414, "Levels\\Level10-Bridge2",       0, 0 },   /* BreakBridge2 */
    { 0x5420, "Levels\\Level9-PopCylinder1",   0, 0 },   /* PopCylinder */
    { 0x5840, "Levels\\Level8-BlockDawg1",     0, 0 },   /* BlockDawg1 */
    { 0x5844, "Levels\\Level8-BlockDawg2",     0, 0 },   /* BlockDawg2 */
    { 0x5848, "Levels\\Level4-Catapult",       0, 0 },   /* Catapult (same mesh, different slot) */

    /* Toob/Odd/Glass Arena meshes */
    { 0x47E0, "Levels\\Level8-Spinny",         0, 0 },   /* Spinny */
};

#define NUM_MESH_ENTRIES (sizeof(g_meshDB) / sizeof(g_meshDB[0]))

/* Mesh cache — stores loaded meshes so we only load each file once */
#define MAX_CACHED_MESHES 32
static CachedMesh g_meshCache[MAX_CACHED_MESHES];
static int g_meshCacheCount = 0;

/* ============================================================
 * Factory table
 *
 * Each entry lists ALL board+0x4xxx offsets the factory READS.
 * If any slot is NULL, the factory is skipped — UNLESS we can
 * JIT-load the mesh for that slot from the mesh database.
 *
 * Factories that don't read any slots (self-loading) have empty lists.
 * ============================================================ */

static SafeFactory g_factories[] = {
    /* Expert Arena: BONK,FAN,SAWBLADE,BRIDGE,JUDGE,BELL — all self-loading or procedural */
    { (FactoryFunc)FACTORY_EXPERT,     {0}, "Expert" },

    /* Wobbly Arena: SMASHER1,SMASHER2 — configuring only, no mesh reads */
    { (FactoryFunc)FACTORY_WOBBLY,     {0}, "Wobbly" },

    /* Master Arena: BRIDGE,TIPPER,BONK,BBRIDGE1-2,POPCYLINDER,BLOCKDAWG1-2,CATAPULT,GLUEBIE */
    { (FactoryFunc)FACTORY_MASTER,     {0x436C, 0x4370, 0x4394, 0x4398,
                                       0x5410, 0x5414, 0x5420,
                                       0x5840, 0x5844, 0x5848, 0x607C, 0}, "Master" },

    /* Tower Arena: CATAPULT,MACE,DRAWBRIDGE,WINDMILL,TRAPDOOR,CHOMPER,TURRET */
    /* Note: 0x4374 is MeshNode (YellowLink), 0x4390 is MeshNode (Chomper) — not MeshWorld */
    { (FactoryFunc)FACTORY_TOWER,      {0x436C, 0x4370, 0x4378, 0x437C, 0x43B4, 0}, "Tower" },

    /* Impossible Arena: LOOPER,GEAR,BIGGEAR,ROTATOR,PENDULUM */
    { (FactoryFunc)FACTORY_IMPOSSIBLE, {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Impossible" },

    /* Up Arena: LIFTER,SPEEDCYLINDER,TIMEBUTTON */
    { (FactoryFunc)FACTORY_UP,         {0x4784, 0x4788, 0x478C, 0}, "Up" },

    /* Dizzy Arena: TIPPER,WATERWHEEL,SWIRL,GLUEBIE */
    { (FactoryFunc)FACTORY_DIZZY,      {0x436C, 0x4370, 0x4374,
                                       0x4BA8, 0x4BAC, 0x4BC4, 0x4BC8, 0}, "Dizzy" },

    /* Beginner Arena: BRIDGE (base, static return) */
    { (FactoryFunc)FACTORY_BEGINNER,   {0x436C, 0x4370, 0}, "Beginner" },

    /* Neon Arena */
    { (FactoryFunc)FACTORY_NEON,       {0x4374, 0x4378, 0x437C, 0x4380, 0x4384, 0x4388, 0}, "Neon" },

    /* Odd Arena */
    { (FactoryFunc)FACTORY_ODD,        {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Odd" },

    /* Toob Arena */
    { (FactoryFunc)FACTORY_TOOB,       {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Toob" },

    /* Glass Arena */
    { (FactoryFunc)FACTORY_GLASS,      {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Glass" },

    /* Sky Arena */
    { (FactoryFunc)FACTORY_SKY,        {0x436C, 0x4374, 0x4378, 0x437C, 0x4380,
                                       0x438C, 0x4390, 0}, "Sky" },
};

#define NUM_FACTORIES (sizeof(g_factories) / sizeof(g_factories[0]))

/* ============================================================
 * Static-mesh detection (objects that return board slot pointer directly)
 * ============================================================ */

static int is_static_mesh_object(const char* refName)
{
    if (_strnicmp(refName, "WATERWHEEL", 10) == 0) return 1;
    if (_strnicmp(refName, "SWIRL", 5) == 0) return 1;
    if (_strnicmp(refName, "BRIDGE", 6) == 0) return 1;
    return 0;
}

/* ============================================================
 * Difficulty-gated object detection
 * ============================================================ */

static int is_difficulty_gated(const char* refName)
{
    if (_strnicmp(refName, "TIPPER",     6) == 0) return 1;
    if (_strnicmp(refName, "BONK",       4) == 0) return 1;
    if (_strnicmp(refName, "BLOCKDAWG",  9) == 0) return 1;
    if (_strnicmp(refName, "GLUEBIE",    7) == 0) return 1;
    if (_strnicmp(refName, "FAN",        3) == 0) return 1;
    if (_strnicmp(refName, "SAWBLADE",   8) == 0) return 1;
    if (_strnicmp(refName, "MACE",       4) == 0) return 1;
    return 0;
}

/* ============================================================
 * Mesh database lookup: find mesh path for a given board offset
 * Returns the MeshEntry* or NULL if not in database.
 *
 * Note: Some board offsets map to different mesh files depending on
 * context (e.g. 0x436C = Level3-Tipper on Dizzy, Level4-Catapult on Tower,
 * LevelImpossible-Looper on Impossible). We try each matching entry.
 * ============================================================ */

static const MeshEntry* find_mesh_for_slot(int boardOffset, int index)
{
    int i;
    int count = 0;
    for (i = 0; i < (int)NUM_MESH_ENTRIES; i++) {
        if (g_meshDB[i].boardOffset == boardOffset) {
            if (count == index) return &g_meshDB[i];
            count++;
        }
    }
    return NULL;
}

/* ============================================================
 * JIT mesh loading: load a mesh from disk into the cache
 *
 * Uses MeshWorld_ctor (0x461510) which:
 *   1. Allocates 0x10D0 bytes (we call operator_new first)
 *   2. Calls MeshWorld_ctor(mem, d3dDevice, path)
 *
 * For meshes that also need CollisionLevel, we:
 *   3. Allocate another 0x10D0 bytes
 *   4. Call CollisionLevel_ctorWithLevel(mem, sourceMesh)
 *
 * D3D device comes from *(board+0x878) + 0x174 (App+0x174)
 * ============================================================ */

static CachedMesh* jit_load_mesh(void* board, const MeshEntry* entry)
{
    int app, d3dDevice;
    void* meshMem = NULL;
    void* collisionMem = NULL;
    CachedMesh* cached = NULL;

    if (g_meshCacheCount >= MAX_CACHED_MESHES) return NULL;

    app = *(int*)((char*)board + BOARD_APP_PTR);
    if (!app) return NULL;

    d3dDevice = *(int*)((char*)app + APP_D3D_DEVICE);
    if (!d3dDevice) return NULL;

    /* Allocate and construct MeshWorld */
    meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return NULL;

    meshMem = g_meshWorldCtor(meshMem, d3dDevice, entry->meshPath);
    if (!meshMem) return NULL;

    /* Optionally create CollisionLevel */
    if (entry->needsCollision && entry->collisionOffset) {
        collisionMem = g_operatorNew(0x10D0);
        if (collisionMem) {
            collisionMem = g_collisionCtor(collisionMem, (int)meshMem);
        }
    }

    /* Store in cache */
    cached = &g_meshCache[g_meshCacheCount++];
    cached->boardOffset = entry->boardOffset;
    cached->mesh = meshMem;
    cached->collision = collisionMem;

    return cached;
}

/* ============================================================
 * Find or load a cached mesh for a board slot
 * ============================================================ */

static CachedMesh* get_or_load_mesh(void* board, int boardOffset)
{
    int i, tryIndex;

    /* Check cache first — look for any cached mesh for this slot */
    for (i = 0; i < g_meshCacheCount; i++) {
        if (g_meshCache[i].boardOffset == boardOffset && g_meshCache[i].mesh) {
            return &g_meshCache[i];
        }
    }

    /* Not in cache — try loading from mesh database */
    for (tryIndex = 0; ; tryIndex++) {
        const MeshEntry* entry = find_mesh_for_slot(boardOffset, tryIndex);
        if (!entry) break;

        CachedMesh* cached = jit_load_mesh(board, entry);
        if (cached) return cached;
    }

    return NULL;  /* No mesh available for this slot */
}

/* ============================================================
 * JIT slot injection: ensure all factory slots are non-NULL
 *
 * For each required slot:
 *   - If already non-NULL, skip (level constructor loaded it)
 *   - If NULL, try JIT-loading from mesh database
 *   - Inject loaded mesh into the slot
 *
 * Returns the number of slots that were JIT-injected (for later restore).
 * Saves original values in the provided arrays.
 * ============================================================ */

#define MAX_INJECTED_SLOTS 16

typedef struct {
    int boardOffset;
    void* originalValue;      /* what was there before (NULL if empty) */
    void* injectedMesh;       /* mesh we injected (NULL if we didn't inject) */
    void* injectedCollision;  /* collision we injected (or NULL) */
    int collisionOffset;      /* where collision was injected (0 = nowhere) */
} InjectedSlot;

static int ensure_slots_loaded(void* board, const SafeFactory* sf,
                                 InjectedSlot* injected, int maxInjected)
{
    int j;
    int injectCount = 0;

    for (j = 0; sf->slots[j] != 0; j++) {
        int offset = sf->slots[j];
        void* currentVal = *(void**)((char*)board + offset);

        if (currentVal != NULL) continue;  /* already loaded */

        if (injectCount >= maxInjected) break;

        /* Try to JIT-load this slot */
        CachedMesh* cached = get_or_load_mesh(board, offset);
        if (!cached) continue;  /* can't load — will be caught by safety check */

        /* Save original (NULL) and inject */
        injected[injectCount].boardOffset = offset;
        injected[injectCount].originalValue = currentVal;  /* NULL */
        injected[injectCount].injectedMesh = cached->mesh;
        injected[injectCount].injectedCollision = cached->collision;
        injected[injectCount].collisionOffset = 0;

        /* Inject mesh */
        *(void**)((char*)board + offset) = cached->mesh;

        /* Inject collision if available and the next slot in the list needs it */
        if (cached->collision) {
            /* Find if the next slot in the factory's list is the collision offset */
            int nextOffset = sf->slots[j + 1];
            if (nextOffset != 0) {
                void* nextVal = *(void**)((char*)board + nextOffset);
                if (nextVal == NULL) {
                    *(void**)((char*)board + nextOffset) = cached->collision;
                    injected[injectCount].collisionOffset = nextOffset;
                    j++;  /* skip the collision slot, it's now filled */
                }
            }
        }

        injectCount++;
    }

    return injectCount;
}

/* Restore injected slots to their original values */
static void restore_injected_slots(void* board, InjectedSlot* injected, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        *(void**)((char*)board + injected[i].boardOffset) = injected[i].originalValue;
        if (injected[i].collisionOffset) {
            /* Restore collision slot — it was NULL before we injected */
            void* origCol = NULL;  /* we only inject into NULL slots */
            *(void**)((char*)board + injected[i].collisionOffset) = origCol;
        }
    }
}

/* ============================================================
 * Safety check: verify all required board slots are non-NULL
 * (after JIT injection attempt)
 * ============================================================ */

static int factory_slots_safe(void* board, const SafeFactory* sf)
{
    int j;
    for (j = 0; sf->slots[j] != 0; j++) {
        void* slot_val = *(void**)((char*)board + sf->slots[j]);
        if (slot_val == NULL) return 0;
    }
    return 1;
}

/* ============================================================
 * Logging — writes ref load results to Z:\tmp\ref_loader_log.txt
 * ============================================================ */

static void log_ref(const char* refName, const char* result, const char* factory)
{
    /* Use OutputDebugString — doesn't interfere with rendering/input */
    char buf[256];
    /* Simple string concatenation (no snprintf to keep DLL small) */
    lstrcpyA(buf, "REFLOAD\t");
    lstrcatA(buf, refName);
    lstrcatA(buf, "\t");
    lstrcatA(buf, result);
    lstrcatA(buf, "\t");
    lstrcatA(buf, factory ? factory : "(null)");
    lstrcatA(buf, "\r\n");
    OutputDebugStringA(buf);
}

static void log_sep(const char* msg)
{
    /* No-op */
}

/* ============================================================
 * Universal Factory Dispatch
 *
 * Called instead of the original vtable[33] dispatch.
 * The calling convention is __thiscall:
 *   ECX = board (this pointer)
 *   Stack: refName, &outObj, &outCol, refEntry
 *
 * Flow:
 *   1. Call the original factory (from board's vtable[33])
 *   2. If it returned an object, clone if static-mesh, return
 *   3. If not, try each Arena factory in order:
 *      a. JIT-load any missing mesh slots from disk
 *      b. Safety-check all slots are non-NULL
 *      c. For difficulty-gated objects, temporarily set App+0x23C = 1
 *      d. Call the factory
 *      e. Restore board slots and difficulty
 *   4. If a factory returned an object, clone if static-mesh, return
 *   5. Return NULL if nothing handled it
 * ============================================================ */

static void __thiscall universal_factory(
    void* board, char* refName,
    void** outObj, void** outCol, int* refEntry)
{
    void** vtable;
    FactoryFunc original;
    int i;
    int savedDiff = 0;
    int needDiffBypass = 0;
    InjectedSlot injected[MAX_INJECTED_SLOTS];
    int injectCount = 0;

    /* Initialize outputs */
    *outObj = NULL;
    *outCol = NULL;

    /* Step 1: Try the original factory first */
    vtable = *(void***)board;
    original = (FactoryFunc)vtable[33]; /* offset 0x84 / 4 = slot 33 */

    original(board, refName, outObj, outCol, refEntry);

    if (*outObj != NULL) {
        /* Original factory handled it — clone if static-mesh */
        if (is_static_mesh_object(refName)) {
            *outObj = g_cloneTree(*outObj, (int)board);
        }
        log_ref(refName, "OK_ORIG", "original");
        return;
    }

    /* Step 2: Original factory didn't handle it — try all Arena factories */
    needDiffBypass = is_difficulty_gated(refName);

    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app) {
            savedDiff = *(int*)((char*)app + APP_DIFFICULTY);
            *(int*)((char*)app + APP_DIFFICULTY) = 1; /* force Normal */
        }
    }

    for (i = 0; i < (int)NUM_FACTORIES; i++) {
        SafeFactory* sf = &g_factories[i];

        /* Skip if this is the same as the original factory */
        if (sf->func == original)
            continue;

        /* JIT load any missing mesh slots */
        injectCount = ensure_slots_loaded(board, sf, injected, MAX_INJECTED_SLOTS);

        /* Safety check: verify all required board slots are non-NULL */
        if (!factory_slots_safe(board, sf)) {
            /* Restore anything we injected */
            if (injectCount > 0)
                restore_injected_slots(board, injected, injectCount);
            continue;
        }

        /* Clear outputs before calling */
        *outObj = NULL;
        *outCol = NULL;

        /* Call the factory */
        sf->func(board, refName, outObj, outCol, refEntry);

        /* Restore injected slots */
        if (injectCount > 0)
            restore_injected_slots(board, injected, injectCount);

        if (*outObj != NULL) {
            /* This factory handled it — clone if static-mesh */
            if (is_static_mesh_object(refName)) {
                *outObj = g_cloneTree(*outObj, (int)board);
            }

            /* Restore difficulty if we changed it */
            if (needDiffBypass) {
                int* app = *(int**)((char*)board + BOARD_APP_PTR);
                if (app)
                    *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
            }
            log_ref(refName, "OK_JIT", sf->name);
            return;
        }
    }

    /* No factory handled this ref — return NULL */
    *outObj = NULL;
    *outCol = NULL;

    log_ref(refName, "FAIL", "none");

    /* Restore difficulty if we changed it */
    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app)
            *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
    }
}

/* ============================================================
 * Hook installation
 * ============================================================ */

static void install_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    BYTE patch[6];
    DWORD rel32;
    DWORD oldProt;

    memcpy(g_origBytes, callSite, 6);

    rel32 = (DWORD)universal_factory - (DWORD)(callSite + 5);

    patch[0] = 0xE8;
    *(DWORD*)(patch + 1) = rel32;
    patch[5] = 0x90;

    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(callSite, patch, 6);
    VirtualProtect(callSite, 6, oldProt, &oldProt);

    FlushInstructionCache(GetCurrentProcess(), callSite, 6);
    g_hooked = TRUE;
}

static void remove_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    DWORD oldProt;

    if (!g_hooked) return;

    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(callSite, g_origBytes, 6);
    VirtualProtect(callSite, 6, oldProt, &oldProt);

    FlushInstructionCache(GetCurrentProcess(), callSite, 6);
    g_hooked = FALSE;
}

/* ============================================================
 * BASS Proxy (v3 lazy loader pattern)
 * ============================================================ */

static HMODULE g_realBass = NULL;
static BOOL g_bassTriedLoad = FALSE;

static void load_real_bass(void)
{
    g_bassTriedLoad = TRUE;
    g_realBass = LoadLibraryA("bass_real.dll");
}

#define DEFINE_BASS_FORWARDED(name, ret_type, params, args, stub_ret) \
    typedef ret_type (__stdcall *name##_t) params; \
    static name##_t real_##name = NULL; \
    __declspec(dllexport) ret_type __stdcall name params { \
        if (!g_bassTriedLoad) load_real_bass(); \
        if (g_realBass && !real_##name) \
            real_##name = (name##_t)GetProcAddress(g_realBass, #name); \
        if (real_##name) return real_##name args; \
        return stub_ret; \
    }

DEFINE_BASS_FORWARDED(BASS_Init,              int,     (int a, int b, int c, HWND d, void* e), (a,b,c,d,e), 0)
DEFINE_BASS_FORWARDED(BASS_Free,              int,     (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_GetVersion,        DWORD,   (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicLoad,          void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f), 0)
DEFINE_BASS_FORWARDED(BASS_StreamCreateFile,  void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e), (a,b,c,d,e), 0)
DEFINE_BASS_FORWARDED(BASS_SampleLoad,        void*,   (void* a, void* b, DWORD c, DWORD d, DWORD e, DWORD f), (a,b,c,d,e,f), 0)
DEFINE_BASS_FORWARDED(BASS_SetConfig,         int,     (DWORD a, DWORD b), (a,b), 0)
DEFINE_BASS_FORWARDED(BASS_GetConfig,         DWORD,   (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_SetVolume,         int,     (float a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_GetVolume,         float,   (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelPlay,        int,     (DWORD a, int b), (a,b), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelStop,       int,     (DWORD a), (a), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttribute, int,    (DWORD a, DWORD b, float c), (a,b,c), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelGetAttribute, int,    (DWORD a, DWORD b, float* c), (a,b,c), 0)
DEFINE_BASS_FORWARDED(BASS_SampleCreate,      void*,   (DWORD a, DWORD b, DWORD c, DWORD d, DWORD e), (a,b,c,d,e), 0)

/* ============================================================
 * DLL Entry Point
 * ============================================================ */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        install_hook();
        break;

    case DLL_PROCESS_DETACH:
        remove_hook();
        if (g_realBass) {
            FreeLibrary(g_realBass);
            g_realBass = NULL;
        }
        break;
    }

    return TRUE;
}
