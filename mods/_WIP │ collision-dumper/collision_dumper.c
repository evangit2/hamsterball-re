/*
 * collision_dumper.c — BASS.dll proxy that dumps all collision entry fields.
 *
 * HOOK: 5-byte JMP detour on Ball_Update (0x00405E00)
 *   After the original Ball_Update runs, we walk the PhysicsObject's
 *   collision entry list and dump every field of every entry to a file.
 *
 * WHAT IT DUMPS:
 *   - PhysicsObject header (count, array pointer, adjacent fields)
 *   - For each collision entry: 0x80 bytes (32 DWORDs) as both hex and
 *     interpreted floats, with known-field annotations
 *
 * OUTPUT:
 *   collision_dump.txt in the game directory (next to bass.dll)
 *   One dump per frame where collision_count > 0, throttled to ~1/sec
 *   MessageBoxA popup on first collision confirming the dump is working
 *
 * BUILD:
 *   i686-w64-mingw32-gcc -shared -o bass.dll collision_dumper.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * INSTALL:
 *   1. Rename original bass.dll → bass_real.dll
 *   2. Copy this bass.dll to the Hamsterball directory
 *   3. Launch Hamsterball.exe
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward to bass_real.dll
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

/* Forward a few key BASS exports. The game only imports a handful. */
typedef int  (__stdcall *BASS_MusicLoadEx_t)(const char*, DWORD, DWORD, DWORD);
static BASS_MusicLoadEx_t real_BASS_MusicLoadEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicLoadEx(const char* a, DWORD b, DWORD c, DWORD d) {
    if (real_BASS_MusicLoadEx) return real_BASS_MusicLoadEx(a, b, c, d);
    return 0;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 0;
}

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 0;
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 0;
}

typedef void (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) void __stdcall BASS_Free(void) {
    if (real_BASS_Free) real_BASS_Free();
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}

typedef int  (__stdcall *BASS_MusicFree_t)(DWORD);
static BASS_MusicFree_t real_BASS_MusicFree = NULL;
__declspec(dllexport) int __stdcall BASS_MusicFree(DWORD a) {
    if (real_BASS_MusicFree) return real_BASS_MusicFree(a);
    return 0;
}

/* Generic forwarder for any other BASS export we missed.
 * The game's import table only has ~7 BASS functions, so the above
 * explicit forwarders should cover everything. */

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define ADDR_Ball_Update       0x00405E00

/* Ball struct */
#define BALL_PLAYER_INDEX      0x18    /* int32: -1 = NPC, 0-3 = Player 1-4 */
#define BALL_PHYSICS_BODY      0x1A4   /* void* -> PhysicsObject */

/* PhysicsObject struct (from user's struct definition) */
#define PHYS_COLLISION_COUNT   0x1C    /* int: number of collision entries */
#define PHYS_COLLISION_ARR     0x424   /* void**: array of CollisionEntry* */
#define PHYS_FIELD_0x18        0x18    /* unknown — dump for comparison */
#define PHYS_FIELD_0x20        0x20    /* unknown — dump for comparison */

/* Entry dump size: 0x80 = 128 bytes = 32 DWORDs.
 * Known fields go up to +0x64 (collision_id), so 0x80 gives headroom. */
#define ENTRY_DUMP_SIZE       0x80
#define ENTRY_DUMP_DWORDS     (ENTRY_DUMP_SIZE / 4)

/* Known collision entry field offsets (from decompilation analysis) */
#define ENTRY_TYPE            0x00    /* int32: 1=ball-ball, 2=wall, 5=floor */
#define ENTRY_OTHER_BALL      0x0C    /* void*: other ball pointer (type==1) */
#define ENTRY_NORMAL_X        0x20    /* float: surface normal X */
#define ENTRY_NORMAL_Y        0x24
#define ENTRY_NORMAL_Z        0x28
#define ENTRY_COLLISION_PT    0x2C    /* float: collision point or distance */
#define ENTRY_NORMAL2_X       0x30    /* float: secondary normal */
#define ENTRY_NORMAL2_Y       0x34
#define ENTRY_NORMAL2_Z       0x38
#define ENTRY_COLLISION_ID    0x64    /* int32: dedup token */

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour Infrastructure
 * ═══════════════════════════════════════════════════════════════════════════ */

static unsigned char g_trampoline[16];
static int g_hook_installed = 0;

/* Original Ball_Update function pointer (via trampoline) */
typedef void (__fastcall *BallUpdate_t)(void* ball, void* edx_dummy);
static BallUpdate_t g_orig_Ball_Update = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * State
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_dump_path[MAX_PATH] = {0};
static volatile LONG g_frame_counter = 0;
static volatile LONG g_last_dump_frame = -300;  /* ensure first collision dumps */
static int g_first_dump = 1;
static CRITICAL_SECTION g_dump_cs;

/* ═══════════════════════════════════════════════════════════════════════════
 * Safe Memory Read
 * ═══════════════════════════════════════════════════════════════════════════ */

static int safe_read(void* addr, void* dest, size_t len) {
    if (!addr || !dest || len == 0) return 0;
    if (IsBadReadPtr(addr, len)) return 0;
    memcpy(dest, addr, len);
    return 1;
}

static DWORD safe_read_dword(void* addr) {
    DWORD val = 0;
    safe_read(addr, &val, 4);
    return val;
}

static float safe_read_float(void* addr) {
    float val = 0.0f;
    safe_read(addr, &val, 4);
    return val;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Collision Entry Dumper
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* type_name(int type) {
    switch (type) {
        case 1:  return "BALL-BALL";
        case 2:  return "WALL";
        case 5:  return "FLOOR";
        default: return "UNKNOWN";
    }
}

static void dump_entry(FILE* f, int entry_idx, void* entry_ptr) {
    BYTE raw[ENTRY_DUMP_SIZE];
    memset(raw, 0, sizeof(raw));

    if (!safe_read(entry_ptr, raw, ENTRY_DUMP_SIZE)) {
        fprintf(f, "  [Entry %d] FAILED to read %d bytes at %p\n", entry_idx, ENTRY_DUMP_SIZE, entry_ptr);
        return;
    }

    DWORD* dw = (DWORD*)raw;

    fprintf(f, "\n--- Entry %d ---\n", entry_idx);
    fprintf(f, "  Entry pointer: %p\n", entry_ptr);

    /* Print all DWORDs as hex */
    for (int i = 0; i < ENTRY_DUMP_DWORDS; i++) {
        DWORD offset = i * 4;
        DWORD val = dw[i];
        float fval = *(float*)&val;

        fprintf(f, "  +0x%02X: 0x%08X", offset, val);

        /* Annotate known fields */
        if (offset == ENTRY_TYPE) {
            fprintf(f, "  | type = %d (%s)", (int)val, type_name((int)val));
        } else if (offset == ENTRY_OTHER_BALL) {
            fprintf(f, "  | other_ball = %p", (void*)val);
        } else if (offset == ENTRY_NORMAL_X) {
            fprintf(f, "  | normal_x = %.6f", fval);
        } else if (offset == ENTRY_NORMAL_Y) {
            fprintf(f, "  | normal_y = %.6f", fval);
        } else if (offset == ENTRY_NORMAL_Z) {
            fprintf(f, "  | normal_z = %.6f", fval);
        } else if (offset == ENTRY_COLLISION_PT) {
            fprintf(f, "  | collision_pt = %.6f", fval);
        } else if (offset == ENTRY_NORMAL2_X) {
            fprintf(f, "  | normal2_x = %.6f", fval);
        } else if (offset == ENTRY_NORMAL2_Y) {
            fprintf(f, "  | normal2_y = %.6f", fval);
        } else if (offset == ENTRY_NORMAL2_Z) {
            fprintf(f, "  | normal2_z = %.6f", fval);
        } else if (offset == ENTRY_COLLISION_ID) {
            fprintf(f, "  | collision_id = %d (0x%X)", (int)val, (int)val);
        }

        /* For non-zero floats, also show the float interpretation */
        if (val != 0 && val != 0xFFFFFFFF && offset != ENTRY_TYPE &&
            offset != ENTRY_OTHER_BALL && offset != ENTRY_COLLISION_ID) {
            /* Check if it looks like a valid float (not NaN/Inf) */
            if (fval == fval && !((val & 0x7F800000) == 0x7F800000 && (val & 0x007FFFFF) != 0)) {
                fprintf(f, "  (as float: %.6f)", fval);
            }
        }

        fprintf(f, "\n");
    }

    /* Raw hex block for easy copy-paste */
    fprintf(f, "  Raw hex:\n  ");
    for (int i = 0; i < ENTRY_DUMP_DWORDS; i++) {
        fprintf(f, "%08X ", dw[i]);
        if ((i + 1) % 8 == 0) fprintf(f, "\n  ");
    }
    fprintf(f, "\n");
}

static void dump_collisions(void* ball) {
    if (!ball) return;

    /* Get physics body from Ball+0x1A4 */
    void* physics = NULL;
    if (!safe_read((char*)ball + BALL_PHYSICS_BODY, &physics, sizeof(physics)) || !physics)
        return;

    /* Read collision_count from PhysicsObject+0x1C */
    int count = 0;
    if (!safe_read((char*)physics + PHYS_COLLISION_COUNT, &count, sizeof(count)))
        return;
    if (count <= 0 || count > 1000) return;  /* sanity check */

    /* Read collision_arr from PhysicsObject+0x424 */
    void** entries = NULL;
    if (!safe_read((char*)physics + PHYS_COLLISION_ARR, &entries, sizeof(entries)) || !entries)
        return;

    /* Throttle: dump at most once per ~60 frames (~2 seconds at 30fps) */
    LONG frame = InterlockedIncrement(&g_frame_counter);
    if (frame - g_last_dump_frame < 60) return;
    g_last_dump_frame = frame;

    EnterCriticalSection(&g_dump_cs);

    FILE* f = fopen(g_dump_path, "a");
    if (!f) {
        LeaveCriticalSection(&g_dump_cs);
        return;
    }

    /* Header */
    int player_idx = 0;
    safe_read((char*)ball + BALL_PLAYER_INDEX, &player_idx, sizeof(player_idx));
    fprintf(f, "\n");
    fprintf(f, "=== Collision Dump — Frame %ld ===\n", frame);
    fprintf(f, "Ball: %p (player_index=%d, %s)\n", ball, player_idx,
            player_idx == -1 ? "NPC/8-ball" : "Player");
    fprintf(f, "PhysicsObject: %p\n", physics);

    /* Dump PhysicsObject header fields around the collision list */
    DWORD val18 = safe_read_dword((char*)physics + PHYS_FIELD_0x18);
    DWORD val1c = safe_read_dword((char*)physics + PHYS_COLLISION_COUNT);
    DWORD val20 = safe_read_dword((char*)physics + PHYS_FIELD_0x20);
    DWORD val424 = safe_read_dword((char*)physics + PHYS_COLLISION_ARR);

    fprintf(f, "\nPhysicsObject collision-list fields:\n");
    fprintf(f, "  +0x18: 0x%08X (%d)  -- possibly count or capacity\n", val18, (int)val18);
    fprintf(f, "  +0x1C: 0x%08X (%d)  -- collision_count\n", val1c, count);
    fprintf(f, "  +0x20: 0x%08X (%p)  -- possibly data pointer or capacity\n", val20, (void*)val20);
    fprintf(f, "  +0x424: 0x%08X (%p)  -- collision_arr\n", val424, entries);

    /* Count entry types */
    int type_counts[6] = {0};
    for (int i = 0; i < count && i < 1000; i++) {
        void* entry_ptr = NULL;
        if (!safe_read(&entries[i], &entry_ptr, sizeof(entry_ptr)) || !entry_ptr)
            continue;
        int type = 0;
        safe_read(entry_ptr, &type, sizeof(type));
        if (type >= 0 && type < 6) type_counts[type]++;
    }

    fprintf(f, "\nEntry type summary (count=%d):\n", count);
    for (int t = 0; t < 6; t++) {
        if (type_counts[t] > 0)
            fprintf(f, "  type %d (%s): %d entries\n", t, type_name(t), type_counts[t]);
    }

    /* Dump each entry */
    for (int i = 0; i < count && i < 1000; i++) {
        void* entry_ptr = NULL;
        if (!safe_read(&entries[i], &entry_ptr, sizeof(entry_ptr)) || !entry_ptr) {
            fprintf(f, "\n--- Entry %d --- NULL\n", i);
            continue;
        }
        dump_entry(f, i, entry_ptr);
    }

    fprintf(f, "\n");

    fclose(f);
    LeaveCriticalSection(&g_dump_cs);

    /* First-dump popup */
    if (g_first_dump) {
        g_first_dump = 0;
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "Collision Dumper active!\n\n"
                 "Found %d collision entries.\n"
                 "Dump written to:\n%s\n\n"
                 "Subsequent dumps throttled to ~1/sec.",
                 count, g_dump_path);
        MessageBoxA(NULL, msg, "Collision Dumper", MB_OK | MB_ICONINFORMATION);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball_Update Hook
 * ═══════════════════════════════════════════════════════════════════════════ */

static void __fastcall hook_Ball_Update(void* ball, void* edx_dummy) {
    /* Call original Ball_Update via trampoline */
    if (g_orig_Ball_Update) {
        g_orig_Ball_Update(ball, edx_dummy);
    }

    /* After Ball_Update completes, the collision entries for this frame
     * are still in the PhysicsObject's list (they get cleared next frame). */
    dump_collisions(ball);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Detour Installation (5-byte JMP)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int install_detour(void* target, void* hook, unsigned char* trampoline) {
    DWORD oldProtect;
    unsigned char* t = (unsigned char*)target;

    if (!VirtualProtect(t, 16, PAGE_EXECUTE_READWRITE, &oldProtect))
        return 0;

    /* Copy original first 5 bytes to trampoline */
    memcpy(trampoline, t, 5);

    /* Append JMP back to target+5 */
    trampoline[5] = 0xE9;  /* JMP rel32 */
    *(unsigned long*)(trampoline + 6) =
        (unsigned long)((char*)target + 5 - (char*)(trampoline + 5) - 5);

    /* Make trampoline executable */
    DWORD tp;
    VirtualProtect(trampoline, 16, PAGE_EXECUTE_READWRITE, &tp);

    /* Overwrite target: JMP rel32 to hook */
    unsigned long rel = (unsigned long)((char*)hook - (char*)target - 5);
    t[0] = 0xE9;
    *(unsigned long*)(t + 1) = rel;

    FlushInstructionCache(GetCurrentProcess(), target, 5);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Initialization
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_bass_proxy(void) {
    /* Load real bass.dll from same directory as our proxy */
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);  /* get exe path */
    char* slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "bass_real.dll");
    } else {
        strcpy(path, "bass_real.dll");
    }

    g_hRealBass = LoadLibraryA(path);
    if (g_hRealBass) {
        real_BASS_MusicLoadEx       = (BASS_MusicLoadEx_t)GetProcAddress(g_hRealBass, "BASS_MusicLoadEx");
        real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_MusicFree         = (BASS_MusicFree_t)GetProcAddress(g_hRealBass, "BASS_MusicFree");
    }
}

static void init_dump_path(void) {
    /* Write collision_dump.txt in the game directory */
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) {
        strcpy(slash + 1, "collision_dump.txt");
    } else {
        strcpy(path, "collision_dump.txt");
    }
    strncpy(g_dump_path, path, MAX_PATH - 1);

    /* Clear previous dump file */
    FILE* f = fopen(g_dump_path, "w");
    if (f) {
        fprintf(f, "Hamsterball Collision Entry Dumper\n");
        fprintf(f, "==================================\n");
        fprintf(f, "\n");
        fprintf(f, "This file is appended each time a ball has collision entries\n");
        fprintf(f, "after Ball_Update completes. Dumps are throttled to ~1/sec.\n");
        fprintf(f, "\n");
        fprintf(f, "Entry struct fields (from decompilation):\n");
        fprintf(f, "  +0x00  int32   type (1=ball-ball, 2=wall, 5=floor)\n");
        fprintf(f, "  +0x0C  void*   other_ball (type==1 only)\n");
        fprintf(f, "  +0x20  float   normal_x\n");
        fprintf(f, "  +0x24  float   normal_y\n");
        fprintf(f, "  +0x28  float   normal_z\n");
        fprintf(f, "  +0x2C  float   collision_pt\n");
        fprintf(f, "  +0x30  float   normal2_x\n");
        fprintf(f, "  +0x34  float   normal2_y\n");
        fprintf(f, "  +0x38  float   normal2_z\n");
        fprintf(f, "  +0x64  int32   collision_id\n");
        fprintf(f, "  (all other offsets are UNKNOWN — that's what we're testing)\n");
        fprintf(f, "\n");
        fprintf(f, "==================================\n\n");
        fclose(f);
    }
}

static DWORD WINAPI patch_thread(LPVOID param) {
    (void)param;
    Sleep(1000);  /* wait for game to fully load */

    InitializeCriticalSection(&g_dump_cs);
    init_bass_proxy();
    init_dump_path();

    /* Install Ball_Update detour */
    void* target = (void*)ADDR_Ball_Update;
    g_orig_Ball_Update = (BallUpdate_t)g_trampoline;

    if (install_detour(target, (void*)hook_Ball_Update, g_trampoline)) {
        g_hook_installed = 1;
        MessageBoxA(NULL,
            "Collision Dumper installed!\n\n"
            "Hooked Ball_Update (0x405E00).\n"
            "Play the game — collision entries will be\n"
            "dumped to collision_dump.txt on each collision.",
            "Collision Dumper", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxA(NULL, "Failed to install Ball_Update hook!", "Collision Dumper Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }

    return TRUE;
}
