/*
 * global_objects_mkn.c — Global SWIRL Spawner for Hamsterball
 *
 * Spawns the Dizzy Race SWIRL object on ANY race/level.
 * Loads "Levels\Level3-Swirl" mesh, registers in render+collision lists,
 * and hooks collision dispatch to handle "N:SWIRL" events.
 *
 * Build: i686-w64-mingw32-gcc -shared -o bass.dll global_objects_mkn.c -lwinmm \
 *        -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * Install: Rename original bass.dll to bass_real.dll, place this bass.dll in game folder.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========== Game function addresses (VA, image base 0x400000) ========== */
#define ADDR_OPERATOR_NEW       0x004BA57B  /* __cdecl: size → void* */
#define ADDR_MESHWORLD_CTOR     0x00461510  /* __thiscall: ECX=mem, [graphics, filename], RET 8 */
#define ADDR_COLLISIONLEVEL_CTOR 0x00465080  /* __thiscall: ECX=mem, [meshworld], RET 4 */
#define ADDR_ATHENALIST_APPEND  0x00453810  /* __thiscall: ECX=list, [item], RET 4 */
#define ADDR_TIMER_INIT         0x00457AD0  /* __fastcall: ECX=&timer, plain RET */
#define ADDR_TIMER_CLEANUP      0x00457A40  /* __fastcall: ECX=&timer, plain RET */
#define ADDR_DISPATCH_COLLISION 0x0040C5D0  /* __thiscall: ECX=board, [ball, entry], RET 8 */
#define ADDR_MASTER_ORCHESTRATOR 0x0041C5B0 /* __thiscall: ECX=board, [app], RET 4 */

/* ========== Struct offsets ========== */
#define APP_GFX_DEVICE          0x174       /* App+0x174 = Graphics* (D3D device) */
#define APP_BASE_ADDR           0x005341E0  /* Global App pointer address */

/* Board offsets */
#define BOARD_APP_PTR           0x878       /* board+0x878 = App* */
#define BOARD_SWIRL_MESH        0x4BC4     /* board+0x4BC4 = SWIRL MeshWorld* (visual) */
#define BOARD_SWIRL_COLLISION   0x4BC8     /* board+0x4BC8 = SWIRL Level* (collision) */
#define BOARD_SWIRL_POS_X       0x4BCC     /* board+0x4BCC = SWIRL position X (float) */
#define BOARD_SWIRL_POS_Y       0x4BD0     /* board+0x4BD0 = SWIRL position Y (float) */
#define BOARD_SWIRL_POS_Z       0x4BD4     /* board+0x4BD4 = SWIRL position Z (float) */
#define BOARD_RENDER_LIST       0x0CD4     /* board+0xCD4 = scene objects render list (AthenaList) */
#define BOARD_COLLISION_LIST    0x10EC     /* board+0x10EC = collision levels master list */
#define BOARD_LEVEL_COLLISION   0x08B0     /* board+0x8B0 = level CollisionLevel */
#define LEVEL_MESH_PTR          0x08AC     /* board+0x8AC = level MeshWorld* */
#define LEVEL_SCENE_OBJ         0x0480     /* MeshWorld+0x480 = SceneObject */

/* Ball offsets */
#define BALL_SWIRLED_FLAG       0x779       /* ball+0x779 = swirled flag (byte, set by N:SWIRL) */

/* Collision entry offsets */
#define COLLISION_ENTRY_NAME    0x864       /* collision_data+0x864 = entity name string ptr */

/* SWIRL mesh path */
static const char *SWIRL_MESH_PATH = "Levels\\Level3-Swirl";
static const char *N_SWIRL_STRING = "N:SWIRL";

/* vtable indices */
#define VTABLE_CALLUPDATE       0x58        /* vtable[0x58] = SceneObject_CallUpdate */
#define VTABLE_CALLRENDER        0x54        /* vtable[0x54] = SceneObject_CallRender */

/* ========== Function pointer types ========== */
typedef void* (__cdecl *operator_new_t)(unsigned int size);
typedef void* (__thiscall *meshworld_ctor_t)(void *mem, void *graphics, const char *filename);
typedef void* (__thiscall *collisionlevel_ctor_t)(void *mem, void *meshworld);
typedef void  (__thiscall *athenalist_append_t)(void *list, int item);
typedef void  (__fastcall *timer_init_t)(void *timer);
typedef void  (__fastcall *timer_cleanup_t)(void *timer);
typedef void  (__thiscall *dispatch_collision_t)(void *board, void *ball, void *entry);
typedef void  (__thiscall *master_orchestrator_t)(void *board, void *app);

/* ========== Function pointers ========== */
static operator_new_t        pfn_operator_new;
static meshworld_ctor_t      pfn_meshworld_ctor;
static collisionlevel_ctor_t pfn_collisionlevel_ctor;
static athenalist_append_t   pfn_athenalist_append;
static timer_init_t          pfn_timer_init;
static timer_cleanup_t       pfn_timer_cleanup;
static dispatch_collision_t  pfn_dispatch_collision;
static master_orchestrator_t pfn_master_orchestrator;

/* ========== Hook state ========== */
static unsigned char *g_orchestrator_tramp = NULL;
static unsigned char *g_collision_tramp = NULL;
static void *g_swirl_mesh = NULL;       /* cached SWIRL MeshWorld* */
static void *g_swirl_collision = NULL;  /* cached SWIRL CollisionLevel* */
static int  g_swirl_spawned = 0;        /* flag: SWIRL already spawned this level */
static int  g_hooked = 0;

/* Default SWIRL position (configurable via global_objects_mkn.txt) */
static float g_swirl_x = 0.0f;
static float g_swirl_y = 0.0f;
static float g_swirl_z = 0.0f;

/* ========== Config file ========== */
static void get_config_path(char *out, DWORD len)
{
    HMODULE hSelf = NULL;
    char dll_path[MAX_PATH];
    BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           (LPCSTR)&DllMain, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, dll_path, MAX_PATH);
        char *slash = strrchr(dll_path, '\\');
        if (slash) { slash[1] = '\0'; _snprintf(out, len, "%sglobal_objects_mkn.txt", dll_path); return; }
    }
    _snprintf(out, len, "global_objects_mkn.txt");
}

static void read_config(void)
{
    char path[MAX_PATH];
    get_config_path(path, MAX_PATH);

    FILE *f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) {
        /* Generate default config */
        FILE *wf = NULL;
        if (fopen_s(&wf, path, "w") != 0 || !wf) return;
        fprintf(wf, "# SWIRL Spawner Configuration\n");
        fprintf(wf, "# Position coordinates for the SWIRL object\n");
        fprintf(wf, "X = 0.0\n");
        fprintf(wf, "Y = 0.0\n");
        fprintf(wf, "Z = 0.0\n");
        fclose(wf);
        return; /* defaults already 0.0 */
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\n' || *p == '\r' || *p == '\0') continue;

        if (_strnicmp(p, "X", 1) == 0) {
            char *eq = strchr(p, '=');
            if (eq) g_swirl_x = (float)strtod(eq + 1, NULL);
        } else if (_strnicmp(p, "Y", 1) == 0) {
            char *eq = strchr(p, '=');
            if (eq) g_swirl_y = (float)strtod(eq + 1, NULL);
        } else if (_strnicmp(p, "Z", 1) == 0) {
            char *eq = strchr(p, '=');
            if (eq) g_swirl_z = (float)strtod(eq + 1, NULL);
        }
    }
    fclose(f);
}

/* ========== Utility: check if address is safe to read ========== */
static int safe_read(void *addr, unsigned int size)
{
    return !IsBadReadPtr(addr, size);
}

/* ========== Spawn SWIRL on the current board ========== */
static void spawn_swirl(void *board)
{
    void *app;
    void *graphics;

    debug_log("spawn_swirl: ENTRY");

    if (!board || !safe_read(board, 0x5000)) {
        debug_log("spawn_swirl: bad board ptr");
        return;
    }
    if (g_swirl_spawned) {
        debug_log("spawn_swirl: already spawned");
        return;
    }

    app = *(void **)((unsigned char *)board + BOARD_APP_PTR);
    if (!app || !safe_read(app, 0x300)) {
        debug_log("spawn_swirl: bad app ptr");
        return;
    }

    graphics = *(void **)((unsigned char *)app + APP_GFX_DEVICE);
    if (!graphics) {
        debug_log("spawn_swirl: bad graphics ptr");
        return;
    }

    debug_log("spawn_swirl: loading mesh");

    /* Load SWIRL mesh (cached — only load once per game session) */
    if (!g_swirl_mesh) {
        void *mem = pfn_operator_new(0x10D0);
        if (!mem) return;
        g_swirl_mesh = pfn_meshworld_ctor(mem, graphics, SWIRL_MESH_PATH);
        if (!g_swirl_mesh) {
            debug_log("spawn_swirl: mesh ctor failed");
            return;
        }

        debug_log("spawn_swirl: mesh loaded, creating collision");

        /* Create collision data from mesh */
        mem = pfn_operator_new(0x10D0);
        if (!mem) return;
        g_swirl_collision = pfn_collisionlevel_ctor(mem, g_swirl_mesh);
        if (!g_swirl_collision) {
            debug_log("spawn_swirl: collision ctor failed");
            return;
        }
    }

    debug_log("spawn_swirl: storing in board slots");

    /* Store mesh and collision in board slots (same offsets as BoardLevel3_ctor) */
    *(void **)((unsigned char *)board + BOARD_SWIRL_MESH) = g_swirl_mesh;
    *(void **)((unsigned char *)board + BOARD_SWIRL_COLLISION) = g_swirl_collision;

    /* Set SWIRL position to origin (same as BoardLevel3_ctor: 0,0,0) */
    *(float *)((unsigned char *)board + BOARD_SWIRL_POS_X) = g_swirl_x;
    *(float *)((unsigned char *)board + BOARD_SWIRL_POS_Y) = g_swirl_y;
    *(float *)((unsigned char *)board + BOARD_SWIRL_POS_Z) = g_swirl_z;

    /* === Register in board render list (board+0xCD4) ===
     * CreateMouseTrap does: AthenaList_Append(board+0xCD4, object)
     * For SWIRL, the MeshWorld itself is the renderable object */
    pfn_athenalist_append((unsigned char *)board + BOARD_RENDER_LIST, (int)g_swirl_mesh);

    /* === Register in scene render tree (board+0x8AC → MeshWorld → +0x480 → SceneObject+0x1C) ===
     * CreateMouseTrap does: AthenaList_Append(board+0x8AC→MeshWorld→+0x480→SceneObject+0x1C, object) */
    {
        void *level_mesh = *(void **)((unsigned char *)board + LEVEL_MESH_PTR);
        if (level_mesh && safe_read(level_mesh, 0x500)) {
            void *scene_obj = *(void **)((unsigned char *)level_mesh + LEVEL_SCENE_OBJ);
            if (scene_obj && safe_read(scene_obj, 0x30)) {
                pfn_athenalist_append((unsigned char *)scene_obj + 0x1C, (int)g_swirl_mesh);
            }
        }
    }

    /* === Register collision data ===
     * BoardLevel3_ctor stores the CollisionLevel at board+0x4BC8.
     * The board's own update loop handles rotating the SWIRL via board+0x4BC4 mesh.
     * We also need to register the collision level in the board's collision lists. */
    if (g_swirl_collision) {
        /* Add to board collision master list (board+0x10EC) */
        pfn_athenalist_append((unsigned char *)board + BOARD_COLLISION_LIST, (int)g_swirl_collision);

        /* Add to level CollisionLevel's child list (board+0x8B0+0x18) */
        {
            void *level_col = *(void **)((unsigned char *)board + BOARD_LEVEL_COLLISION);
            if (level_col && safe_read(level_col, 0x30)) {
                pfn_athenalist_append((unsigned char *)level_col + 0x18, (int)g_swirl_collision);
            }
        }
    }

    g_swirl_spawned = 1;
    debug_log("spawn_swirl: SUCCESS");
}

/* ========== Detour: Master Orchestrator Hook ========== */
/* Original prologue (7 bytes): 6A FF 68 2B A1 4C 00 */
/* After original runs, spawn SWIRL on the board */

/* Ghidra confirms: __fastcall, 1 param (ECX=board), RET 0.
 * Using __fastcall with 2 reg params so callee does RET 0 (EDX unused). */
static void __fastcall orchestrator_hook(void *board, void *unused)
{
    debug_log("orchestrator_hook: ENTRY");

    /* Call original function via trampoline */
    typedef void (__fastcall *orig_t)(void *, void *);
    ((orig_t)g_orchestrator_tramp)(board, unused);

    debug_log("orchestrator_hook: original returned");

    /* After original finishes, spawn SWIRL */
    if (board && safe_read(board, 0x20)) {
        /* Verify board vtable pointer is valid */
        void **vt = *(void ***)board;
        if (vt && safe_read(vt, 4)) {
            debug_log("orchestrator_hook: calling spawn_swirl");
            spawn_swirl(board);
            debug_log("orchestrator_hook: spawn_swirl returned");
        }
    }
}

/* ========== Detour: DispatchCollisionEvents Hook ========== */
/* Original prologue (8 bytes): 6A FF 64 A1 00 00 00 00 */
/* Intercept "N:SWIRL" collision events on non-Dizzy boards */

/* DispatchCollisionEvents is __thiscall: ECX=board, [ESP+4]=ball, [ESP+8]=entry, RET 8.
 * Using __fastcall with 4 params (2 reg + 2 stack) so callee does RET 8. */
static void __fastcall collision_hook(void *board, void *unused, void *ball, void *entry)
{
    /* Check for "N:SWIRL" collision event BEFORE calling original */
    if (entry && safe_read(entry, 8)) {
        int *pair = (int *)entry;
        int collision_data_ptr = pair[1];
        if (collision_data_ptr && safe_read((void *)collision_data_ptr, COLLISION_ENTRY_NAME + 4)) {
            char *event_name = *(char **)((unsigned char *)collision_data_ptr + COLLISION_ENTRY_NAME);
            if (event_name && safe_read(event_name, 8)) {
                if (_stricmp(event_name, N_SWIRL_STRING) == 0) {
                    /* Set swirled flag on ball */
                    if (ball && safe_read(ball, BALL_SWIRLED_FLAG + 1)) {
                        *(unsigned char *)((unsigned char *)ball + BALL_SWIRLED_FLAG) = 1;
                    }
                }
            }
        }
    }

    /* Call original DispatchCollisionEvents */
    typedef void (__fastcall *orig_t)(void *, void *, void *, void *);
    ((orig_t)g_collision_tramp)(board, unused, ball, entry);
}

/* ========== Detour installation ========== */
static void install_detour(DWORD target_addr, void *hook_fn,
                           const unsigned char *orig_bytes, int orig_len,
                           unsigned char **out_tramp)
{
    DWORD old_prot;
    unsigned char *tramp;
    unsigned char jmp_patch[5];
    int patch_len;

    /* Allocate executable trampoline: orig_len bytes + 5 for JMP back */
    tramp = (unsigned char *)VirtualAlloc(NULL, orig_len + 5,
                MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return;

    /* Copy original prologue to trampoline */
    memcpy(tramp, orig_bytes, orig_len);

    /* Add JMP back to target+orig_len */
    tramp[orig_len] = 0xE9; /* JMP rel32 */
    *(DWORD *)(tramp + orig_len + 1) =
        (target_addr + orig_len) - ((DWORD)tramp + orig_len + 5);

    /* Patch original: JMP to hook */
    VirtualProtect((void *)target_addr, orig_len, PAGE_EXECUTE_READWRITE, &old_prot);

    jmp_patch[0] = 0xE9; /* JMP rel32 */
    *(DWORD *)(jmp_patch + 1) = (DWORD)hook_fn - (target_addr + 5);
    memcpy((void *)target_addr, jmp_patch, 5);

    /* Fill remaining bytes with NOPs */
    patch_len = orig_len - 5;
    if (patch_len > 0) {
        memset((void *)(target_addr + 5), 0x90, patch_len);
    }

    VirtualProtect((void *)target_addr, orig_len, old_prot, &old_prot);
    FlushInstructionCache(GetCurrentProcess(), (void *)target_addr, orig_len);

    *out_tramp = tramp;
}

static void install_hooks(void)
{
    /* Verify original bytes at master orchestrator (0x41C5B0) */
    static const unsigned char ORCH_PROLOGUE[7] = {
        0x6A, 0xFF, 0x68, 0x2B, 0xA1, 0x4C, 0x00
    };
    /* Verify original bytes at DispatchCollisionEvents (0x40C5D0) */
    static const unsigned char COLL_PROLOGUE[8] = {
        0x6A, 0xFF, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00
    };

    unsigned char *p;

    /* Verify orchestrator prologue */
    p = (unsigned char *)ADDR_MASTER_ORCHESTRATOR;
    if (!safe_read(p, 7)) return;
    if (memcmp(p, ORCH_PROLOGUE, 7) != 0) return;

    /* Verify collision dispatch prologue */
    p = (unsigned char *)ADDR_DISPATCH_COLLISION;
    if (!safe_read(p, 8)) return;
    if (memcmp(p, COLL_PROLOGUE, 8) != 0) return;

    /* Install orchestrator detour (7-byte prologue) */
    install_detour(ADDR_MASTER_ORCHESTRATOR, orchestrator_hook,
                   ORCH_PROLOGUE, 7, &g_orchestrator_tramp);

    /* Install collision dispatch detour (8-byte prologue) */
    install_detour(ADDR_DISPATCH_COLLISION, collision_hook,
                   COLL_PROLOGUE, 8, &g_collision_tramp);

    g_hooked = 1;
}

/* ========== Init function pointers ========== */
static void init_function_pointers(void)
{
    pfn_operator_new       = (operator_new_t)ADDR_OPERATOR_NEW;
    pfn_meshworld_ctor     = (meshworld_ctor_t)ADDR_MESHWORLD_CTOR;
    pfn_collisionlevel_ctor = (collisionlevel_ctor_t)ADDR_COLLISIONLEVEL_CTOR;
    pfn_athenalist_append  = (athenalist_append_t)ADDR_ATHENALIST_APPEND;
    pfn_timer_init         = (timer_init_t)ADDR_TIMER_INIT;
    pfn_timer_cleanup      = (timer_cleanup_t)ADDR_TIMER_CLEANUP;
    pfn_dispatch_collision = (dispatch_collision_t)ADDR_DISPATCH_COLLISION;
    pfn_master_orchestrator = (master_orchestrator_t)ADDR_MASTER_ORCHESTRATOR;
}

/* ========== BASS Proxy Stubs ========== */
/* Minimal BASS proxy — lazy-loads bass_real.dll if present */

static HMODULE g_real_bass = NULL;

static void lazy_load_bass(void)
{
    if (g_real_bass) return;
    g_real_bass = LoadLibraryA("bass_real.dll");
}

/* Game imports exactly these 10 BASS functions */
__declspec(dllexport) int __stdcall BASS_Init(int a, int b, int c, int d, void *e)
{
    (void)a; (void)b; (void)c; (void)d; (void)e;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(int, int, int, int, void *);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Init");
        if (fn) return fn(a, b, c, d, e);
    }
    return 0; /* FALSE — no crash on missing bass_real */
}

__declspec(dllexport) void __stdcall BASS_Free(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef void (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Free");
        if (fn) fn();
    }
}

__declspec(dllexport) int __stdcall BASS_Stop(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Stop");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_Start(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_Start");
        if (fn) return fn();
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_SetConfig(int a, int b)
{
    (void)a; (void)b;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_SetConfig");
        if (fn) return fn(a, b);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ErrorGetCode");
        if (fn) return fn();
    }
    return 0;
}

__declspec(dllexport) void * __stdcall BASS_MusicLoad(int a, void *b, int c, int d, int e, int f)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
    lazy_load_bass();
    if (g_real_bass) {
        typedef void * (__stdcall *fn_t)(int, void *, int, int, int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_MusicLoad");
        if (fn) return fn(a, b, c, d, e, f);
    }
    return NULL;
}

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(void *a, int b, int c)
{
    (void)a; (void)b; (void)c;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *, int, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_MusicPlayEx");
        if (fn) return fn(a, b, c);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelStop(void *a)
{
    (void)a;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ChannelStop");
        if (fn) return fn(a);
    }
    return 1;
}

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(void *a, int b, float c, int d)
{
    (void)a; (void)b; (void)c; (void)d;
    lazy_load_bass();
    if (g_real_bass) {
        typedef int (__stdcall *fn_t)(void *, int, float, int);
        fn_t fn = (fn_t)GetProcAddress(g_real_bass, "BASS_ChannelSetAttributes");
        if (fn) return fn(a, b, c, d);
    }
    return 1;
}

/* ========== DllMain ========== */
static void debug_log(const char *msg)
{
    FILE *f = NULL;
    if (fopen_s(&f, "swirl_debug.log", "a") == 0 && f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    (void)hinstDLL; (void)lpvReserved;

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        debug_log("DllMain: DLL_PROCESS_ATTACH start");
        init_function_pointers();
        debug_log("DllMain: init_function_pointers done");
        read_config();
        debug_log("DllMain: read_config done");
        install_hooks();
        debug_log("DllMain: install_hooks done");
        break;

    case DLL_PROCESS_DETACH:
        /* Hooks are automatically removed when process exits */
        break;
    }

    return TRUE;
}
