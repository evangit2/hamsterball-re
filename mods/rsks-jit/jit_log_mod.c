/*
 * jit_log_mod.c — Universal Ref Loader v3 with COMPREHENSIVE LOGGING
 *
 * This is a copy of universal_ref_loader.c with extensive logging added.
 * Does NOT modify the original. Logs EVERYTHING to Z:\tmp\ref_loader_log.txt
 *
 * Logs:
 *   - DLL load/unload
 *   - Hook install
 *   - Every factory dispatch (original + alternatives)
 *   - Every ref name seen
 *   - Every board slot check (offset, value before/after)
 *   - Every JIT mesh load (path, success/fail, cache hit/miss)
 *   - Every clone operation
 *   - Every difficulty bypass
 *   - Every safety check pass/fail
 *   - Board pointer, vtable pointer, app pointer
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll jit_log_mod.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>

/* ============================================================
 * LOGGING
 * ============================================================ */

static const char* LOG_PATH = "C:\\tmp\\ref_loader_log.txt";
static HANDLE g_logMutex = NULL;

static void log_init(void)
{
    FILE* f = NULL;
    /* Clear the log at startup */
    if (fopen_s(&f, LOG_PATH, "w") == 0 && f) {
        fprintf(f, "=== Universal Ref Loader LOG — DLL Loaded ===\n");
        fprintf(f, "Timestamp: DLL_PROCESS_ATTACH\n");
        fclose(f);
    }
    g_logMutex = CreateMutexA(NULL, FALSE, "RefLoaderLogMutex");
}

static void log_msg(const char* fmt, ...)
{
    FILE* f = NULL;
    va_list args;

    if (fopen_s(&f, LOG_PATH, "a") != 0 || !f) return;

    if (g_logMutex) WaitForSingleObject(g_logMutex, INFINITE);

    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
    fflush(f);
    fclose(f);

    if (g_logMutex) ReleaseMutex(g_logMutex);
}

/* Hex dump helper */
static void log_hex(const char* label, void* ptr, int len)
{
    int i;
    char buf[1024];
    int pos = 0;
    unsigned char* p = (unsigned char*)ptr;

    pos += sprintf(buf + pos, "%s (ptr=%p, len=%d): ", label, ptr, len);
    for (i = 0; i < len && pos < 900; i++) {
        pos += sprintf(buf + pos, "%02X ", p[i]);
        if ((i + 1) % 16 == 0) pos += sprintf(buf + pos, "\n  ");
    }
    log_msg("%s", buf);
}

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

typedef struct {
    FactoryFunc func;
    int slots[14];
    const char* name;
} SafeFactory;

typedef struct {
    int boardOffset;
    const char* meshPath;
    int needsCollision;
    int collisionOffset;
} MeshEntry;

typedef struct {
    int boardOffset;
    void* mesh;
    void* collision;
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

static int g_refCounter = 0;

/* ============================================================
 * Mesh database
 * ============================================================ */

static const MeshEntry g_meshDB[] = {
    { 0x436C, "Levels\\Level3-Tipper",      1, 0x4370 },
    { 0x4394, "Levels\\Level3-Tipper",      1, 0x4398 },
    { 0x4BA8, "Levels\\Level3-WaterWheel",  1, 0x4BAC },
    { 0x4BC4, "Levels\\Level3-Swirl",       1, 0x4BC8 },
    { 0x4374, "Levels\\Level3-Gluebie",      0, 0 },
    { 0x607C, "Levels\\Level3-Gluebie",      0, 0 },
    { 0x436C, "Levels\\Level4-Catapult",     0, 0 },
    { 0x4370, "Levels\\Level4-Drawbridge",   0, 0 },
    { 0x4378, "Levels\\Level4-Mace",         0, 0 },
    { 0x437C, "Levels\\Level4-Windmill",     0, 0 },
    { 0x43B4, "Levels\\Level4-Turret",       0, 0 },
    { 0x436C, "Levels\\LevelImpossible-Looper",   0, 0 },
    { 0x4370, "Levels\\LevelImpossible-Gear",     0, 0 },
    { 0x4374, "Levels\\LevelImpossible-BigGear",  0, 0 },
    { 0x4378, "Levels\\LevelImpossible-Rotator",   0, 0 },
    { 0x437C, "Levels\\LevelImpossible-Pendulum", 0, 0 },
    { 0x4784, "Levels\\LevelUp-Lifter",        0, 0 },
    { 0x4788, "Levels\\LevelUp-SpeedCylinder", 0, 0 },
    { 0x478C, "Levels\\LevelUp-Button",        0, 0 },
    { 0x4378, "Levels\\Level5-Bridge",         0, 0 },
    { 0x5410, "Levels\\Level10-Bridge1",       0, 0 },
    { 0x5414, "Levels\\Level10-Bridge2",       0, 0 },
    { 0x5420, "Levels\\Level9-PopCylinder1",   0, 0 },
    { 0x5840, "Levels\\Level8-BlockDawg1",     0, 0 },
    { 0x5844, "Levels\\Level8-BlockDawg2",     0, 0 },
    { 0x5848, "Levels\\Level4-Catapult",       0, 0 },
    { 0x47E0, "Levels\\Level8-Spinny",         0, 0 },
};

#define NUM_MESH_ENTRIES (sizeof(g_meshDB) / sizeof(g_meshDB[0]))

#define MAX_CACHED_MESHES 32
static CachedMesh g_meshCache[MAX_CACHED_MESHES];
static int g_meshCacheCount = 0;

/* ============================================================
 * Factory table
 * ============================================================ */

static SafeFactory g_factories[] = {
    { (FactoryFunc)FACTORY_EXPERT,     {0}, "Expert" },
    { (FactoryFunc)FACTORY_WOBBLY,     {0}, "Wobbly" },
    { (FactoryFunc)FACTORY_MASTER,     {0x436C, 0x4370, 0x4394, 0x4398,
                                       0x5410, 0x5414, 0x5420,
                                       0x5840, 0x5844, 0x5848, 0x607C, 0}, "Master" },
    { (FactoryFunc)FACTORY_TOWER,      {0x436C, 0x4370, 0x4378, 0x437C, 0x43B4, 0}, "Tower" },
    { (FactoryFunc)FACTORY_IMPOSSIBLE, {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Impossible" },
    { (FactoryFunc)FACTORY_UP,         {0x4784, 0x4788, 0x478C, 0}, "Up" },
    { (FactoryFunc)FACTORY_DIZZY,      {0x436C, 0x4370, 0x4374,
                                       0x4BA8, 0x4BAC, 0x4BC4, 0x4BC8, 0}, "Dizzy" },
    { (FactoryFunc)FACTORY_BEGINNER,   {0x436C, 0x4370, 0}, "Beginner" },
    { (FactoryFunc)FACTORY_NEON,       {0x4374, 0x4378, 0x437C, 0x4380, 0x4384, 0x4388, 0}, "Neon" },
    { (FactoryFunc)FACTORY_ODD,        {0x436C, 0x4370, 0x4374, 0x4378, 0x437C, 0}, "Odd" },
    { (FactoryFunc)FACTORY_TOOB,       {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Toob" },
    { (FactoryFunc)FACTORY_GLASS,      {0x436C, 0x4370, 0x4374, 0x4378, 0x437C,
                                       0x4380, 0x4384, 0}, "Glass" },
    { (FactoryFunc)FACTORY_SKY,        {0x436C, 0x4374, 0x4378, 0x437C, 0x4380,
                                       0x438C, 0x4390, 0}, "Sky" },
};

#define NUM_FACTORIES (sizeof(g_factories) / sizeof(g_factories[0]))

/* ============================================================
 * Static-mesh detection
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
 * Mesh database lookup
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
 * JIT mesh loading
 * ============================================================ */

static CachedMesh* jit_load_mesh(void* board, const MeshEntry* entry)
{
    int app, d3dDevice;
    void* meshMem = NULL;
    void* collisionMem = NULL;
    CachedMesh* cached = NULL;

    log_msg("  [JIT_LOAD] Attempting to load mesh: path='%s' boardOffset=0x%X collision=%d",
            entry->meshPath, entry->boardOffset, entry->needsCollision);

    if (g_meshCacheCount >= MAX_CACHED_MESHES) {
        log_msg("  [JIT_LOAD] FAILED: mesh cache full (%d/%d)", g_meshCacheCount, MAX_CACHED_MESHES);
        return NULL;
    }

    app = *(int*)((char*)board + BOARD_APP_PTR);
    log_msg("  [JIT_LOAD] board=%p, board+0x878 (App ptr) = 0x%08X", board, app);
    if (!app) {
        log_msg("  [JIT_LOAD] FAILED: App pointer is NULL");
        return NULL;
    }

    d3dDevice = *(int*)((char*)app + APP_D3D_DEVICE);
    log_msg("  [JIT_LOAD] App+0x174 (D3D device) = 0x%08X", d3dDevice);
    if (!d3dDevice) {
        log_msg("  [JIT_LOAD] FAILED: D3D device is NULL");
        return NULL;
    }

    log_msg("  [JIT_LOAD] Allocating 0x10D0 bytes for MeshWorld...");
    meshMem = g_operatorNew(0x10D0);
    log_msg("  [JIT_LOAD] operator_new returned: %p", meshMem);
    if (!meshMem) {
        log_msg("  [JIT_LOAD] FAILED: operator_new returned NULL");
        return NULL;
    }

    log_msg("  [JIT_LOAD] Calling MeshWorld_ctor(mem=%p, d3d=%p, path='%s')...",
            meshMem, (void*)d3dDevice, entry->meshPath);
    meshMem = g_meshWorldCtor(meshMem, d3dDevice, entry->meshPath);
    log_msg("  [JIT_LOAD] MeshWorld_ctor returned: %p", meshMem);
    if (!meshMem) {
        log_msg("  [JIT_LOAD] FAILED: MeshWorld_ctor returned NULL (file not found?)");
        return NULL;
    }

    if (entry->needsCollision && entry->collisionOffset) {
        log_msg("  [JIT_LOAD] Allocating 0x10D0 bytes for CollisionLevel...");
        collisionMem = g_operatorNew(0x10D0);
        log_msg("  [JIT_LOAD] operator_new (collision) returned: %p", collisionMem);
        if (collisionMem) {
            log_msg("  [JIT_LOAD] Calling CollisionLevel_ctorWithLevel(mem=%p, sourceMesh=%p)...",
                    collisionMem, meshMem);
            collisionMem = g_collisionCtor(collisionMem, (int)meshMem);
            log_msg("  [JIT_LOAD] CollisionLevel_ctor returned: %p", collisionMem);
        }
    }

    cached = &g_meshCache[g_meshCacheCount++];
    cached->boardOffset = entry->boardOffset;
    cached->mesh = meshMem;
    cached->collision = collisionMem;

    log_msg("  [JIT_LOAD] SUCCESS: cached mesh #%d (offset=0x%X, mesh=%p, collision=%p)",
            g_meshCacheCount - 1, entry->boardOffset, meshMem, collisionMem);
    return cached;
}

static CachedMesh* get_or_load_mesh(void* board, int boardOffset)
{
    int i, tryIndex;

    log_msg("  [GET_MESH] Looking for mesh for board offset 0x%X", boardOffset);

    /* Check cache first */
    for (i = 0; i < g_meshCacheCount; i++) {
        if (g_meshCache[i].boardOffset == boardOffset && g_meshCache[i].mesh) {
            log_msg("  [GET_MESH] CACHE HIT: found cached mesh #%d (mesh=%p, collision=%p)",
                    i, g_meshCache[i].mesh, g_meshCache[i].collision);
            return &g_meshCache[i];
        }
    }
    log_msg("  [GET_MESH] Cache miss, trying mesh database...");

    for (tryIndex = 0; ; tryIndex++) {
        const MeshEntry* entry = find_mesh_for_slot(boardOffset, tryIndex);
        if (!entry) {
            log_msg("  [GET_MESH] No more mesh entries for offset 0x%X (tried %d)",
                    boardOffset, tryIndex);
            break;
        }
        log_msg("  [GET_MESH] Trying mesh DB entry %d: path='%s' collision=%d",
                tryIndex, entry->meshPath, entry->needsCollision);

        CachedMesh* cached = jit_load_mesh(board, entry);
        if (cached) return cached;
        log_msg("  [GET_MESH] Mesh entry %d failed, trying next...", tryIndex);
    }

    log_msg("  [GET_MESH] FAILED: no mesh available for offset 0x%X", boardOffset);
    return NULL;
}

/* ============================================================
 * JIT slot injection
 * ============================================================ */

#define MAX_INJECTED_SLOTS 16

typedef struct {
    int boardOffset;
    void* originalValue;
    void* injectedMesh;
    void* injectedCollision;
    int collisionOffset;
} InjectedSlot;

static int ensure_slots_loaded(void* board, const SafeFactory* sf,
                                 InjectedSlot* injected, int maxInjected)
{
    int j;
    int injectCount = 0;

    log_msg("  [ENSURE_SLOTS] Factory '%s' requires %d slots:", sf->name, 0);
    /* Count slots first */
    for (j = 0; sf->slots[j] != 0; j++) {}
    log_msg("  [ENSURE_SLOTS] Factory '%s' requires %d slots", sf->name, j);

    for (j = 0; sf->slots[j] != 0; j++) {
        int offset = sf->slots[j];
        void* currentVal = *(void**)((char*)board + offset);

        log_msg("  [ENSURE_SLOTS] Slot +0x%X: current value = %p %s",
                offset, currentVal, currentVal ? "(NON-NULL, pre-loaded)" : "(NULL, needs JIT)");

        if (currentVal != NULL) continue;

        if (injectCount >= maxInjected) {
            log_msg("  [ENSURE_SLOTS] WARNING: max injected slots reached (%d)", maxInjected);
            break;
        }

        CachedMesh* cached = get_or_load_mesh(board, offset);
        if (!cached) {
            log_msg("  [ENSURE_SLOTS] Could not load mesh for slot +0x%X, will fail safety check", offset);
            continue;
        }

        injected[injectCount].boardOffset = offset;
        injected[injectCount].originalValue = currentVal;
        injected[injectCount].injectedMesh = cached->mesh;
        injected[injectCount].injectedCollision = cached->collision;
        injected[injectCount].collisionOffset = 0;

        log_msg("  [ENSURE_SLOTS] INJECTING mesh %p into board+0x%X (was %p)",
                cached->mesh, offset, currentVal);
        *(void**)((char*)board + offset) = cached->mesh;

        if (cached->collision) {
            int nextOffset = sf->slots[j + 1];
            if (nextOffset != 0) {
                void* nextVal = *(void**)((char*)board + nextOffset);
                if (nextVal == NULL) {
                    log_msg("  [ENSURE_SLOTS] INJECTING collision %p into board+0x%X (was NULL)",
                            cached->collision, nextOffset);
                    *(void**)((char*)board + nextOffset) = cached->collision;
                    injected[injectCount].collisionOffset = nextOffset;
                    j++;
                } else {
                    log_msg("  [ENSURE_SLOTS] Collision slot +0x%X already non-NULL (%p), skipping",
                            nextOffset, nextVal);
                }
            }
        }

        injectCount++;
    }

    log_msg("  [ENSURE_SLOTS] Injected %d slots total", injectCount);
    return injectCount;
}

static void restore_injected_slots(void* board, InjectedSlot* injected, int count)
{
    int i;
    log_msg("  [RESTORE] Restoring %d injected slots:", count);
    for (i = 0; i < count; i++) {
        log_msg("  [RESTORE] Slot +0x%X: restoring to %p (was injected with %p)",
                injected[i].boardOffset, injected[i].originalValue, injected[i].injectedMesh);
        *(void**)((char*)board + injected[i].boardOffset) = injected[i].originalValue;
        if (injected[i].collisionOffset) {
            log_msg("  [RESTORE] Collision slot +0x%X: restoring to NULL (was injected with %p)",
                    injected[i].collisionOffset, injected[i].injectedCollision);
            *(void**)((char*)board + injected[i].collisionOffset) = NULL;
        }
    }
}

static int factory_slots_safe(void* board, const SafeFactory* sf)
{
    int j;
    for (j = 0; sf->slots[j] != 0; j++) {
        void* slot_val = *(void**)((char*)board + sf->slots[j]);
        if (slot_val == NULL) {
            log_msg("  [SAFETY_CHECK] FAILED: slot +0x%X is NULL for factory '%s'",
                    sf->slots[j], sf->name);
            return 0;
        }
    }
    log_msg("  [SAFETY_CHECK] PASSED for factory '%s' (all %d slots non-NULL)", sf->name, j);
    return 1;
}

/* ============================================================
 * Universal Factory Dispatch (with LOGGING)
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
    char refNameSafe[128] = {0};

    g_refCounter++;

    /* Safely copy ref name for logging (in case it's not null-terminated) */
    if (refName) {
        int i2;
        for (i2 = 0; i2 < 127 && refName[i2]; i2++)
            refNameSafe[i2] = refName[i2];
        refNameSafe[i2] = 0;
    } else {
        strcpy(refNameSafe, "(NULL)");
    }

    log_msg("========================================");
    log_msg("[REF #%d] board=%p, refName='%s', outObj=%p, outCol=%p, refEntry=%p",
            g_refCounter, board, refNameSafe, outObj, outCol, refEntry);

    /* Log ref entry data if available */
    if (refEntry) {
        int* ri = (int*)refEntry;
        log_msg("[REF #%d] refEntry raw: [0]=%d [1]=0x%08X [2]=0x%08X [3]=0x%08X [4]=0x%08X [5]=0x%08X",
                g_refCounter, ri[0], ri[1], ri[2], ri[3], ri[4], ri[5]);
        /* refEntry[1,2,3] = position x,y,z as floats */
        {
            float* rf = (float*)refEntry;
            log_msg("[REF #%d] refEntry pos: x=%f y=%f z=%f (floats at [1],[2],[3])",
                    g_refCounter, rf[1], rf[2], rf[3]);
        }
    }

    /* Log board info */
    if (board) {
        vtable = *(void***)board;
        log_msg("[REF #%d] board vtable = %p, vtable[33] (factory) = %p",
                g_refCounter, vtable, vtable ? vtable[33] : NULL);

        {
            int* appPtr = *(int**)((char*)board + BOARD_APP_PTR);
            log_msg("[REF #%d] board+0x878 (App ptr) = 0x%08X", g_refCounter, (int)appPtr);
            if (appPtr) {
                int diff = *(int*)((char*)appPtr + APP_DIFFICULTY);
                int d3d = *(int*)((char*)appPtr + APP_D3D_DEVICE);
                int arenaFlag = *(unsigned char*)((char*)appPtr + 0x237);
                log_msg("[REF #%d] App: difficulty(0x23C)=%d, D3D(0x174)=0x%08X, arenaFlag(0x237)=%d",
                        g_refCounter, diff, d3d, arenaFlag);
            }
        }
    }

    /* Initialize outputs */
    *outObj = NULL;
    *outCol = NULL;

    /* Step 1: Try original factory first */
    vtable = *(void***)board;
    original = (FactoryFunc)vtable[33];

    log_msg("[REF #%d] Step 1: Calling ORIGINAL factory at %p ...", g_refCounter, original);

    original(board, refName, outObj, outCol, refEntry);

    log_msg("[REF #%d] Step 1 result: outObj=%p, outCol=%p", g_refCounter, *outObj, *outCol);

    if (*outObj != NULL) {
        log_msg("[REF #%d] ORIGINAL factory HANDLED ref '%s' -> obj=%p", g_refCounter, refNameSafe, *outObj);

        if (is_static_mesh_object(refNameSafe)) {
            log_msg("[REF #%d] CLONE: '%s' is static-mesh, cloning via Level_CloneTree(%p, %p)...",
                    g_refCounter, refNameSafe, *outObj, board);
            *outObj = g_cloneTree(*outObj, (int)board);
            log_msg("[REF #%d] CLONE result: %p", g_refCounter, *outObj);
        }
        log_msg("[REF #%d] DONE (handled by original)", g_refCounter);
        return;
    }

    log_msg("[REF #%d] Original factory did NOT handle ref '%s'. Trying %d Arena factories...",
            g_refCounter, refNameSafe, (int)NUM_FACTORIES);

    /* Step 2: Try all Arena factories */
    needDiffBypass = is_difficulty_gated(refNameSafe);
    log_msg("[REF #%d] Difficulty gated? %s", g_refCounter, needDiffBypass ? "YES" : "NO");

    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app) {
            savedDiff = *(int*)((char*)app + APP_DIFFICULTY);
            log_msg("[REF #%d] DIFFICULTY BYPASS: saving current diff=%d, setting to 1 (Normal)",
                    g_refCounter, savedDiff);
            *(int*)((char*)app + APP_DIFFICULTY) = 1;
        }
    }

    for (i = 0; i < (int)NUM_FACTORIES; i++) {
        SafeFactory* sf = &g_factories[i];

        log_msg("[REF #%d] --- Trying factory %d/%d: '%s' (addr=%p) ---",
                g_refCounter, i + 1, (int)NUM_FACTORIES, sf->name, sf->func);

        if (sf->func == original) {
            log_msg("[REF #%d] SKIP: same as original factory", g_refCounter);
            continue;
        }

        /* Log all slot values before JIT */
        {
            int j2;
            for (j2 = 0; sf->slots[j2] != 0; j2++) {
                void* val = *(void**)((char*)board + sf->slots[j2]);
                log_msg("[REF #%d]   pre-JIT slot +0x%X = %p", g_refCounter, sf->slots[j2], val);
            }
        }

        /* JIT load missing mesh slots */
        injectCount = ensure_slots_loaded(board, sf, injected, MAX_INJECTED_SLOTS);

        /* Log all slot values after JIT */
        {
            int j2;
            for (j2 = 0; sf->slots[j2] != 0; j2++) {
                void* val = *(void**)((char*)board + sf->slots[j2]);
                log_msg("[REF #%d]   post-JIT slot +0x%X = %p", g_refCounter, sf->slots[j2], val);
            }
        }

        /* Safety check */
        if (!factory_slots_safe(board, sf)) {
            log_msg("[REF #%d] SKIPPING factory '%s' — safety check failed", g_refCounter, sf->name);
            if (injectCount > 0)
                restore_injected_slots(board, injected, injectCount);
            continue;
        }

        /* Clear outputs */
        *outObj = NULL;
        *outCol = NULL;

        log_msg("[REF #%d] CALLING factory '%s' (addr=%p) with refName='%s'...",
                g_refCounter, sf->name, sf->func, refNameSafe);

        sf->func(board, refName, outObj, outCol, refEntry);

        log_msg("[REF #%d] Factory '%s' returned: outObj=%p, outCol=%p",
                g_refCounter, sf->name, *outObj, *outCol);

        /* Restore injected slots */
        if (injectCount > 0)
            restore_injected_slots(board, injected, injectCount);

        if (*outObj != NULL) {
            log_msg("[REF #%d] *** Factory '%s' HANDLED ref '%s' -> obj=%p ***",
                    g_refCounter, sf->name, refNameSafe, *outObj);

            if (is_static_mesh_object(refNameSafe)) {
                log_msg("[REF #%d] CLONE: '%s' is static-mesh, cloning via Level_CloneTree(%p, %p)...",
                        g_refCounter, refNameSafe, *outObj, board);
                *outObj = g_cloneTree(*outObj, (int)board);
                log_msg("[REF #%d] CLONE result: %p", g_refCounter, *outObj);
            }

            if (needDiffBypass) {
                int* app = *(int**)((char*)board + BOARD_APP_PTR);
                if (app) {
                    log_msg("[REF #%d] Restoring difficulty to %d", g_refCounter, savedDiff);
                    *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
                }
            }
            log_msg("[REF #%d] DONE (handled by factory '%s')", g_refCounter, sf->name);
            return;
        }

        log_msg("[REF #%d] Factory '%s' did not handle ref '%s'", g_refCounter, sf->name, refNameSafe);
    }

    /* No factory handled this ref */
    log_msg("[REF #%d] *** NO FACTORY HANDLED ref '%s' — returning NULL ***", g_refCounter, refNameSafe);
    *outObj = NULL;
    *outCol = NULL;

    if (needDiffBypass) {
        int* app = *(int**)((char*)board + BOARD_APP_PTR);
        if (app) {
            log_msg("[REF #%d] Restoring difficulty to %d", g_refCounter, savedDiff);
            *(int*)((char*)app + APP_DIFFICULTY) = savedDiff;
        }
    }
    log_msg("[REF #%d] DONE (unhandled)", g_refCounter);
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

    log_msg("[HOOK] Installing hook at 0x%08X...", HOOK_CALL_SITE);

    memcpy(g_origBytes, callSite, 6);
    log_msg("[HOOK] Saved original bytes: %02X %02X %02X %02X %02X %02X",
            g_origBytes[0], g_origBytes[1], g_origBytes[2],
            g_origBytes[3], g_origBytes[4], g_origBytes[5]);

    rel32 = (DWORD)universal_factory - (DWORD)(callSite + 5);
    log_msg("[HOOK] universal_factory at %p, callSite+5 at %p, rel32 = 0x%08X",
            universal_factory, callSite + 5, rel32);

    patch[0] = 0xE8;
    *(DWORD*)(patch + 1) = rel32;
    patch[5] = 0x90;

    log_msg("[HOOK] Patch bytes: %02X %02X %02X %02X %02X %02X",
            patch[0], patch[1], patch[2], patch[3], patch[4], patch[5]);

    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    log_msg("[HOOK] VirtualProtect OK, old protection = %d", oldProt);

    memcpy(callSite, patch, 6);
    log_msg("[HOOK] Patch written");

    VirtualProtect(callSite, 6, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), callSite, 6);
    g_hooked = TRUE;

    log_msg("[HOOK] Hook installed successfully");
}

static void remove_hook(void)
{
    BYTE* callSite = (BYTE*)HOOK_CALL_SITE;
    DWORD oldProt;

    if (!g_hooked) return;

    log_msg("[HOOK] Removing hook, restoring original bytes...");

    VirtualProtect(callSite, 6, PAGE_EXECUTE_READWRITE, &oldProt);
    memcpy(callSite, g_origBytes, 6);
    VirtualProtect(callSite, 6, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), callSite, 6);

    g_hooked = FALSE;
    log_msg("[HOOK] Hook removed, original bytes restored");
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
    log_msg("[BASS] load_real_bass: tried=%d, handle=%p", g_bassTriedLoad, g_realBass);
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
DEFINE_BASS_FORWARDED(BASS_Start,             int,     (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_Stop,              int,     (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_MusicPlayEx,       int,     (DWORD a, DWORD b, DWORD c, DWORD d), (a,b,c,d), 0)
DEFINE_BASS_FORWARDED(BASS_ErrorGetCode,      int,     (void), (), 0)
DEFINE_BASS_FORWARDED(BASS_ChannelSetAttributes, int,  (DWORD a, DWORD b, float c), (a,b,c), 0)
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
        log_init();
        log_msg("[DLLMAIN] DLL_PROCESS_ATTACH, hinstDLL=%p", hinstDLL);
        log_msg("[DLLMAIN] Installing hook...");
        install_hook();
        log_msg("[DLLMAIN] Hook installed, DLL ready");
        break;

    case DLL_PROCESS_DETACH:
        log_msg("[DLLMAIN] DLL_PROCESS_DETACH");
        remove_hook();
        if (g_realBass) {
            FreeLibrary(g_realBass);
            g_realBass = NULL;
        }
        log_msg("[DLLMAIN] Cleanup done, DLL unloading");
        break;
    }

    return TRUE;
}
