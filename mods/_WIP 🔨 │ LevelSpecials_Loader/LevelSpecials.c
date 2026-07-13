/*
 * LevelSpecials_Loader v3 — Universal Level Handler
 *
 * 1. ALLOCATION PATCH: Patches all 15 level allocation sites in
 *    Tournament_AdvanceRace (0x00427080) to use the union size 0xA2F8,
 *    so every level gets enough board memory for ALL possible objects.
 *
 * 2. COLLISION HOOK: Hooks DispatchCollisionEvents (0x0040C5D0) to
 *    handle N:BUMPER collisions on any level, driven by config.
 *
 * Config file (LevelSpecials.txt) controls which features are active per level:
 *   [BUMPERS]
 *   N:BUMPER1 = 2 5 8
 *   N:BUMPER2 = 1 3
 *   ...
 *
 * If the current level number appears in the list, bumpers are active.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long QWORD;

/* ═══════════════════════════════════════════════════════════════════════════
 * Game addresses (RVAs)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RVA_DispatchCollisionEvents  0x000CC5D0
#define RVA_Sound_Play3D             0x00059860
#define RVA_Scene_CollectByNameFilter 0x000602F0
#define RVA_AthenaString_Format      0x00066C70
#define RVA_AthenaList_Init          0x00053210
#define RVA_operator_new              0x000BA57B
#define RVA_Level_MeshWorldCtor       0x00061510
#define RVA_Level_RenderCtor          0x00065080
#define RVA_Level_InitScene           0x0000B090

/* Bumper physics constants */
#define BUMPER_VEL_SCALE  4.0f
#define BUMPER_MIN_SPEED  5.0f
#define BUMPER_MAX_SPEED  10.0f

/* Struct offsets */
#define BALL_POS_X        0x164
#define BALL_POS_Y        0x168
#define BALL_POS_Z        0x16C
#define BALL_PHYS_PTR     0x1A4
#define PHYS_VEL_X        0xCA4
#define PHYS_VEL_Y        0xCA8
#define PHYS_VEL_Z        0xCAC
#define APP_SOUNDFX_LIST  0x448
#define COLL_MESHBUF      0x4
#define MESHBUF_NAME      0x864
#define BOARD_APP_PTR     0x878
#define BOARD_MESHWORLD   0x8AC  /* board+0x8AC = MeshWorld ptr (set by constructor) */
#define BOARD_RENDEROBJ   0x8B0  /* board+0x8B0 = RenderObj ptr (set by constructor) */

/* Bumper slot layout in board struct (Beginner's pattern, int* arithmetic) */
#define BUMPER_SLOT_BASE   0x436C  /* board + 0x10DB * 4 = first bumper AthenaList */
#define BUMPER_SLOT_STRIDE 0x418   /* 0x106 * 4 = stride between bumper slots */
#define BUMPER_LIT_BASE    0x642C  /* board + 0x190B * 4 = first lit flag DWORD */
#define BUMPER_LIT_STRIDE  4       /* 1 * 4 = stride between lit flags */
#define BUMPER_LIT_COLL     0x6428 /* collision handler writes idx*4 + this */

/* ═══════════════════════════════════════════════════════════════════════════
 * Level vtable addresses (absolute — module base 0x00400000)
 * ═══════════════════════════════════════════════════════════════════════════ */

static const DWORD g_levelVtables[16] = {
    0,              /* index 0 unused */
    0x004D04A8,     /* 1=WarmUp */
    0x004D1098,     /* 2=Beginner */
    0x004D05A0,     /* 3=Intermediate */
    0x004D0890,     /* 4=Dizzy */
    0x004D0A08,     /* 5=Tower */
    0x004D11A0,     /* 6=Up */
    0x004D1DF0,     /* 7=Neon */
    0x004D0B00,     /* 8=Expert */
    0x004D0BC0,     /* 9=Odd */
    0x004D0E78,     /* 10=Toob */
    0x004D0D38,     /* 11=Wobbly */
    0x004D1F90,     /* 12=Glass */
    0x004D0FC8,     /* 13=Sky */
    0x004D12B0,     /* 14=Master */
    0x004D21C0,     /* 15=Impossible */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Config: per-level feature flags
 * ═══════════════════════════════════════════════════════════════════════════ */

/* bumpers_enabled[level] = 1 if bumpers active for that level (1-15) */
static int g_bumpersEnabled[16] = {0};

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD g_moduleBase = 0;
typedef void (__thiscall *Sound_Play3D_t)(void *soundChannel, float x, float y, float z);
static Sound_Play3D_t g_SoundPlay3D = NULL;

/* Must be non-static for asm reference */
typedef void (__thiscall *DispatchCollisionEvents_t)(void *board, void *ball, void *collPair);
DispatchCollisionEvents_t g_OriginalDispatch = NULL;

static unsigned char *g_trampoline = NULL;
static char g_configPath[MAX_PATH] = "";

/* ═══════════════════════════════════════════════════════════════════════════
 * String helpers
 * ═══════════════════════════════════════════════════════════════════════════ */

static int my_strnicmp(const char *a, const char *b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        if (ca == 0) return 0;
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Level identification: read board vtable and match to level number
 * ═══════════════════════════════════════════════════════════════════════════ */

static int GetCurrentLevel(void *board) {
    DWORD vtable = *(DWORD *)board;
    int i;
    for (i = 1; i <= 15; i++) {
        if (vtable == g_levelVtables[i])
            return i;
    }
    return 0; /* unknown */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config file parser
 *
 * Format (LevelSpecials.txt):
 *   [BUMPERS]
 *   BUMPERS = 2 5 8
 *
 * Level numbers: 1=WarmUp 2=Beginner 3=Intermediate 4=Dizzy 5=Tower
 *   6=Up 7=Neon 8=Expert 9=Odd 10=Toob 11=Wobbly 12=Glass 13=Sky
 *   14=Master 15=Impossible
 * ═══════════════════════════════════════════════════════════════════════════ */

static void LoadConfig(void) {
    /* Clear all flags */
    memset(g_bumpersEnabled, 0, sizeof(g_bumpersEnabled));

    HANDLE hFile = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    /* Read entire file */
    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize > 8192) fileSize = 8192;
    char buf[8192];
    DWORD bytesRead = 0;
    ReadFile(hFile, buf, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    buf[bytesRead] = '\0';

    /* Parse line by line */
    char *line = buf;
    int inBumpersSection = 0;

    while (line < buf + bytesRead) {
        /* Find end of line */
        char *eol = line;
        while (*eol && *eol != '\n' && *eol != '\r') eol++;

        /* Null-terminate line */
        char saved = *eol;
        *eol = '\0';

        /* Strip leading whitespace */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        /* Skip empty lines and comments */
        if (*p == '\0' || *p == '#' || *p == ';') goto next_line;

        /* Check for section header */
        if (p[0] == '[') {
            inBumpersSection = (my_strnicmp(p, "[BUMPERS", 8) == 0);
            goto next_line;
        }

        /* If in BUMPERS section, look for BUMPERS = line */
        if (inBumpersSection && my_strnicmp(p, "BUMPERS", 7) == 0) {
            /* Find '=' sign */
            char *eq = p;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                eq++;
                /* Parse level numbers after = */
                while (*eq) {
                    /* Skip whitespace */
                    while (*eq == ' ' || *eq == '\t') eq++;
                    if (!*eq) break;

                    /* Check for empty parens */
                    if (*eq == '(') {
                        eq++;
                        while (*eq && *eq != ')') eq++;
                        if (*eq == ')') eq++;
                        continue;
                    }

                    /* Parse number */
                    if (*eq >= '0' && *eq <= '9') {
                        int levelNum = atoi(eq);
                        if (levelNum >= 1 && levelNum <= 15) {
                            g_bumpersEnabled[levelNum] = 1;
                        }
                        /* Skip the number */
                        while (*eq && *eq >= '0' && *eq <= '9') eq++;
                    } else {
                        eq++;
                    }
                }
            }
        }

    next_line:
        *eol = saved;
        /* Move to next line */
        if (*eol == '\r') eol++;
        if (*eol == '\n') eol++;
        line = eol;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Get config path (next to this DLL)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void GetConfigPath(void) {
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&GetConfigPath, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, g_configPath, MAX_PATH)) {
        char *p = strrchr(g_configPath, '\\');
        if (p) strcpy(p + 1, "LevelSpecials.txt");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Apply bumper bounce physics
 * ═══════════════════════════════════════════════════════════════════════════ */

static void ApplyBumperBounce(void *board, void *ball, void *collPair) {
    DWORD *pair = (DWORD *)collPair;
    DWORD meshBuf = pair[COLL_MESHBUF / 4];
    if (!meshBuf || IsBadReadPtr((void *)meshBuf, 0x870)) return;

    char *name = *(char **)(meshBuf + MESHBUF_NAME);
    if (!name || IsBadReadPtr(name, 9)) return;

    if (my_strnicmp(name, "N:BUMPER", 8) != 0)
        return;

    /* Get ball position */
    if (IsBadReadPtr(ball, 0x1A8)) return;
    float posX = *(float *)((char *)ball + BALL_POS_X);
    float posY = *(float *)((char *)ball + BALL_POS_Y);
    float posZ = *(float *)((char *)ball + BALL_POS_Z);

    /* Play bumper sound */
    DWORD *app = *(DWORD **)((char *)board + BOARD_APP_PTR);
    if (app && !IsBadReadPtr(app, 0x500)) {
        DWORD soundList = app[APP_SOUNDFX_LIST / 4];
        if (soundList && g_SoundPlay3D) {
            g_SoundPlay3D((void *)soundList, posX, posY, posZ);
        }
    }

    /* Get physics struct */
    DWORD phys = *(DWORD *)((char *)ball + BALL_PHYS_PTR);
    if (!phys || IsBadReadPtr((void *)phys, 0xCB0)) return;

    float *velX = (float *)(phys + PHYS_VEL_X);
    float *velY = (float *)(phys + PHYS_VEL_Y);
    float *velZ = (float *)(phys + PHYS_VEL_Z);

    float vx = *velX * BUMPER_VEL_SCALE;
    float vz = *velZ * BUMPER_VEL_SCALE;
    float vy = 0.0f;

    /* Clamp speed */
    float speedSq = vx * vx + vz * vz;
    if (speedSq < BUMPER_MIN_SPEED * BUMPER_MIN_SPEED) {
        if (speedSq > 0.0001f) {
            float speed = sqrtf(speedSq);
            float scale = BUMPER_MIN_SPEED / speed;
            vx *= scale; vz *= scale;
        }
    }
    speedSq = vx * vx + vz * vz;
    if (speedSq > BUMPER_MAX_SPEED * BUMPER_MAX_SPEED) {
        float speed = sqrtf(speedSq);
        if (speed > 0.0001f) {
            float scale = BUMPER_MAX_SPEED / speed;
            vx *= scale; vz *= scale;
        }
    }

    *velX = vx; *velY = vy; *velZ = vz;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal collision logic — called by naked thunk for ALL levels
 * ═══════════════════════════════════════════════════════════════════════════ */

void __cdecl BumperCollisionLogic(void *board, void *ball, void *collPair) {
    /* Identify level by vtable */
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    /* Check if bumpers are enabled for this level */
    if (!g_bumpersEnabled[level]) return;

    /* Check if this collision involves a bumper and apply bounce */
    ApplyBumperBounce(board, ball, collPair);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Naked thunk for DispatchCollisionEvents hook
 *
 * __thiscall: ECX=board, [ESP+4]=ball, [ESP+8]=collPair, RET 0x8
 * Tail-JMPs to trampoline so original RET 0x8 handles cleanup.
 * ═══════════════════════════════════════════════════════════════════════════ */

__attribute__((naked)) static void Hook_DispatchCollisionEvents(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%ecx\n\t"
        "pushl %%edx\n\t"

        /* Call BumperCollisionLogic(board, ball, collPair) via cdecl */
        "pushl 12(%%ebp)\n\t"      /* collPair */
        "pushl 8(%%ebp)\n\t"       /* ball */
        "pushl -4(%%ebp)\n\t"      /* board (saved on stack) */
        "call  _BumperCollisionLogic\n\t"
        "addl  $12, %%esp\n\t"

        "popl  %%edx\n\t"
        "popl  %%ecx\n\t"
        "popl  %%ebp\n\t"

        /* Tail-JMP to trampoline */
        "jmpl  *_g_OriginalDispatch\n\t"
        :: : "eax", "memory"
    );
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game function typedefs
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void *(__cdecl *operator_new_t)(unsigned int size);
typedef void *(__thiscall *Level_MeshWorldCtor_t)(void *mem, void *gfx, const char *meshPath);
typedef void *(__thiscall *Level_RenderCtor_t)(void *mem, void *meshWorld);
typedef void (__thiscall *Level_InitScene_t)(void *board);
typedef void (__thiscall *Scene_CollectByNameFilter_t)(void *meshWorld, char *name, void *destList);
typedef char *(__cdecl *AthenaString_Format_t)(char *dest, const char *fmt, ...);
typedef void *(__thiscall *AthenaList_Init_t)(void *this, int capacity);

static operator_new_t g_operatorNew = NULL;
static Level_MeshWorldCtor_t g_LevelMeshWorldCtor = NULL;
static Level_RenderCtor_t g_LevelRenderCtor = NULL;
static Level_InitScene_t g_LevelInitScene = NULL;
static Scene_CollectByNameFilter_t g_CollectByNameFilter = NULL;
static AthenaString_Format_t g_AthenaStringFormat = NULL;
static AthenaList_Init_t g_AthenaListInit = NULL;

/* Forward declaration — defined below */
static void UniversalPostSetup(void *board);

/* ═══════════════════════════════════════════════════════════════════════════
 * Mesh path table — one per level
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *g_meshPaths[16] = {
    NULL,                          /* 0 unused */
    "levels\\level1",              /* 1=WarmUp */
    "levels\\levelcascade",        /* 2=Beginner */
    "levels\\level2",              /* 3=Intermediate */
    "levels\\level3",              /* 4=Dizzy */
    "levels\\level4",              /* 5=Tower */
    "levels\\levelup",             /* 6=Up */
    "levels\\leveldark",           /* 7=Neon */
    "levels\\level5",              /* 8=Expert */
    "levels\\level6",              /* 9=Odd */
    "levels\\level8",              /* 10=Toob */
    "levels\\level7",              /* 11=Wobbly */
    "levels\\levelglass",          /* 12=Glass */
    "levels\\level9",              /* 13=Sky */
    "levels\\level10",             /* 14=Master */
    "levels\\levelimpossible",     /* 15=Impossible */
};

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Level Constructor — REPLACES all 15 per-level constructors
 *
 * Does the same 4 steps every Scene_LoadLevel* does:
 *   1. operator_new(0x10D0) → Level_MeshWorldCtor(mem, gfx, meshPath)
 *   2. operator_new(0x10D0) → Level_RenderCtor(mem, meshWorld)
 *   3. Level_InitScene(board)
 *   4. board->vtable[0x80]() = Board_Setup
 * Then runs UniversalPostSetup for config-driven features (bumpers, etc.)
 *
 * The per-level Scene_LoadLevel* functions NEVER run.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalConstructor(void *board, int raceIndex) {
    if (!board || raceIndex < 1 || raceIndex > 15) return;
    if (!g_operatorNew || !g_LevelMeshWorldCtor || !g_LevelRenderCtor ||
        !g_LevelInitScene) return;

    const char *meshPath = g_meshPaths[raceIndex];
    if (!meshPath) return;

    /* board+0x878 = App, App+0x174 = gfx */
    DWORD app = *(DWORD *)((char *)board + BOARD_APP_PTR);
    if (!app || IsBadReadPtr((void *)app, 0x200)) return;
    void *gfx = *(void **)((char *)app + 0x174);
    if (!gfx) return;

    /* Step 1: MeshWorld */
    void *meshMem = g_operatorNew(0x10D0);
    if (!meshMem) return;
    void *meshWorld = g_LevelMeshWorldCtor(meshMem, gfx, meshPath);
    *(DWORD *)((char *)board + BOARD_MESHWORLD) = (DWORD)meshWorld;

    /* Step 2: RenderObj */
    void *renderMem = g_operatorNew(0x10D0);
    void *renderObj = NULL;
    if (renderMem) {
        renderObj = g_LevelRenderCtor(renderMem, meshWorld);
    }
    *(DWORD *)((char *)board + BOARD_RENDEROBJ) = (DWORD)renderObj;

    /* Step 3: InitScene */
    g_LevelInitScene(board);

    /* Step 4: Board_Setup via vtable[0x80] */
    DWORD vtable = *(DWORD *)board;
    if (vtable && !IsBadReadPtr((void *)vtable, 0x84)) {
        void (__thiscall *boardSetup)(void *) = *(void (__thiscall **)(void *))((char *)vtable + 0x80);
        if (boardSetup) boardSetup(board);
    }

    /* Step 5: Config-driven features */
    UniversalPostSetup(board);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Universal Post-Setup — config-driven feature initialization
 *
 * Called from UniversalConstructor after the base 4 steps.
 * Currently handles: Bumpers (CollectByNameFilter + lit flags)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void UniversalPostSetup(void *board) {
    int level = GetCurrentLevel(board);
    if (level == 0) return;

    /* Reload config in case it changed */
    LoadConfig();

    /* ── Bumpers ── */
    if (g_bumpersEnabled[level]) {
        DWORD meshWorld = *(DWORD *)((char *)board + BOARD_MESHWORLD);
        if (!meshWorld || IsBadReadPtr((void *)meshWorld, 0x430)) return;

        int i;
        for (i = 0; i < 8; i++) {
            /* Format "N:BUMPER%d" with i+1 */
            char nameBuf[16];
            char *result;

            if (g_AthenaStringFormat) {
                static char globalBuf[256];
                g_AthenaStringFormat(globalBuf, "N:BUMPER%d", i + 1);
                result = globalBuf;
            } else {
                /* Fallback: manual format */
                const char *prefix = "N:BUMPER";
                int p = 0, j;
                for (j = 0; prefix[j]; j++) nameBuf[p++] = prefix[j];
                int num = i + 1;
                if (num >= 10) { nameBuf[p++] = '0' + (num / 10); num %= 10; }
                nameBuf[p++] = '0' + num;
                nameBuf[p] = '\0';
                result = nameBuf;
            }

            /* Destination: board + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE */
            void *dest = (char *)board + BUMPER_SLOT_BASE + i * BUMPER_SLOT_STRIDE;

            /* Initialize the AthenaList at this slot */
            if (g_AthenaListInit) {
                g_AthenaListInit(dest, 0);
            }

            /* Collect mesh objects matching "N:BUMPER%d" into the AthenaList */
            if (g_CollectByNameFilter) {
                g_CollectByNameFilter((void *)meshWorld, result, dest);
            }

            /* Initialize lit flag to 0 */
            *(DWORD *)((char *)board + BUMPER_LIT_BASE + i * BUMPER_LIT_STRIDE) = 0;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Tournament_AdvanceRace hook — replaces vtable[0x48] call
 *
 * Hooks at 0x004273E0 (RVA 0x000273E0).
 * Original bytes: FF 52 48 8B 4E 04 (6 bytes)
 *   FF 52 48     = CALL [EDX+0x48]  (vtable[18] = Scene_LoadLevel*)
 *   8B 4E 04     = MOV ECX, [ESI+4] (next instruction)
 *
 * Patched with: E9 <rel32> 90 (JMP Hook_UniversalConstructor + NOP)
 * Trampoline: only MOV ECX,[ESI+4] + JMP back to 0x004273E6
 * (Skips CALL [EDX+0x48] — original per-level constructor NEVER runs)
 *
 * At hook point:
 *   ECX = board (this, set by MOV ECX,EAX at 0x004273D6)
 *   ESI = tournament struct
 *   [ESI+0x8] = race index (1-based: 1=WarmUp ... 15=Impossible)
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RVA_TournamentVtableCall  0x000273E0

/* Must be non-static for asm reference */
unsigned char *g_ctorTrampoline = NULL;

/* Must be non-static for asm reference */
void __cdecl UniversalConstructorLogic(void *board, int raceIndex) {
    UniversalConstructor(board, raceIndex);
}

__attribute__((naked)) static void Hook_UniversalConstructor(void) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "movl  %%esp, %%ebp\n\t"
        "pushl %%edx\n\t"
        "pushl %%ecx\n\t"

        /* raceIndex = [ESI+0x8] */
        "movl  0x08(%%esi), %%eax\n\t"
        "pushl %%eax\n\t"

        /* board = ECX (still valid — not clobbered yet) */
        "pushl %%ecx\n\t"

        "call  _UniversalConstructorLogic\n\t"
        "addl  $8, %%esp\n\t"

        "popl  %%ecx\n\t"
        "popl  %%edx\n\t"
        "popl  %%ebp\n\t"

        /* JMP to trampoline (executes MOV ECX,[ESI+4] + JMP back to original+6) */
        "jmpl  *_g_ctorTrampoline\n\t"
        :: : "eax", "memory"
    );
}

static void InstallUniversalConstructorHook(void) {
    DWORD targetAddr = g_moduleBase + RVA_TournamentVtableCall;
    unsigned char *orig = (unsigned char *)targetAddr;

    /* Verify: FF 52 48 8B 4E 04 */
    if (orig[0] != 0xFF || orig[1] != 0x52 || orig[2] != 0x48 ||
        orig[3] != 0x8B || orig[4] != 0x4E || orig[5] != 0x04) return;

    /* Trampoline: only MOV ECX,[ESI+4] + JMP back to original+6 */
    g_ctorTrampoline = VirtualAlloc(NULL, 16,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_ctorTrampoline) return;

    /* Trampoline: only MOV ECX,[ESI+4] (skip CALL [EDX+0x48] = original constructor) */
    g_ctorTrampoline[0] = 0x8B;  /* MOV ECX, [ESI+4] */
    g_ctorTrampoline[1] = 0x4E;
    g_ctorTrampoline[2] = 0x04;
    g_ctorTrampoline[3] = 0xE9;  /* JMP back to original+6 */
    *(DWORD *)(g_ctorTrampoline + 4) = (targetAddr + 6) - ((DWORD)g_ctorTrampoline + 8);

    /* Patch original: JMP rel32 + NOP (6 bytes total) */
    DWORD oldProtect;
    VirtualProtect(orig, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(orig + 1) = (DWORD)&Hook_UniversalConstructor - (targetAddr + 5);
    orig[5] = 0x90;   /* NOP */
    VirtualProtect(orig, 6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 6);
}
/* ═══════════════════════════════════════════════════════════════════════════
 * Allocation size patch — make all levels allocate the union size
 *
 * Tournament_AdvanceRace (0x00427080) has a switch with 15 cases.
 * Each case does: PUSH <size>; CALL operator_new
 * The PUSH imm32 instruction is 5 bytes: 68 XX XX XX XX
 * We patch the 4-byte immediate at addr+1 to 0xA2F8 (union of all objects).
 *
 * Level 14 (Master) is already 0x6498, still needs patching to 0xA2F8.
 * ═══════════════════════════════════════════════════════════════════════════ */

#define UNION_SIZE  0xA2F8

/* RVA of each PUSH imm32 instruction (address of the 0x68 opcode) */
static const DWORD g_allocPatchRVAs[15] = {
    0x00027109,  /* 1=WarmUp    (was 0x436C) */
    0x00027136,  /* 2=Beginner  (was 0x644C) */
    0x00027167,  /* 3=Intermed  (was 0x438C) */
    0x00027198,  /* 4=Dizzy     (was 0x4BE0) */
    0x000271C9,  /* 5=Tower     (was 0x5418) */
    0x000271FA,  /* 6=Up        (was 0x4790) */
    0x0002722B,  /* 7=Neon      (was 0x4394) */
    0x0002725C,  /* 8=Expert    (was 0x4FD8) */
    0x0002728D,  /* 9=Odd       (was 0x43B0) */
    0x000272BE,  /* 10=Toob     (was 0x646C) */
    0x000272EF,  /* 11=Wobbly   (was 0x4388) */
    0x00027320,  /* 12=Glass    (was 0x4390) */
    0x00027351,  /* 13=Sky      (was 0x47F8) */
    0x0002737B,  /* 14=Master   (was 0x6498) */
    0x000273A5,  /* 15=Imposs   (was 0x4380) */
};

static void PatchAllocSizes(void) {
    int i;
    for (i = 0; i < 15; i++) {
        unsigned char *site = (unsigned char *)(g_moduleBase + g_allocPatchRVAs[i]);
        if (IsBadReadPtr(site, 5)) continue;
        if (site[0] != 0x68) continue;  /* verify PUSH opcode */

        DWORD oldProtect;
        VirtualProtect(site, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
        *(DWORD *)(site + 1) = UNION_SIZE;
        VirtualProtect(site, 5, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), site, 5);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Install collision hook
 * ═══════════════════════════════════════════════════════════════════════════ */

static void InstallHook(void) {
    DWORD targetAddr = g_moduleBase + RVA_DispatchCollisionEvents;
    unsigned char *orig = (unsigned char *)targetAddr;

    /* Verify prologue: 6A FF 64 A1 00 00 00 00 */
    if (orig[0] != 0x6A || orig[1] != 0xFF ||
        orig[2] != 0x64 || orig[3] != 0xA1) return;

    g_trampoline = VirtualAlloc(NULL, 16,
                    MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) return;

    memcpy(g_trampoline, orig, 8);
    g_trampoline[8] = 0xE9;
    *(DWORD *)(g_trampoline + 9) = (targetAddr + 8) - ((DWORD)g_trampoline + 13);

    g_OriginalDispatch = (DispatchCollisionEvents_t)g_trampoline;

    DWORD oldProtect;
    VirtualProtect(orig, 16, PAGE_EXECUTE_READWRITE, &oldProtect);
    orig[0] = 0xE9;
    *(DWORD *)(orig + 1) = (DWORD)&Hook_DispatchCollisionEvents - (targetAddr + 5);
    orig[5] = 0x90;
    orig[6] = 0x90;
    orig[7] = 0x90;
    VirtualProtect(orig, 16, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), orig, 16);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS.dll proxy exports — explicit typedefs (MinGW-compatible)
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

typedef BOOL  (__stdcall *BASS_Init_t)(int,DWORD,DWORD,HWND,void*);
typedef void  (__stdcall *BASS_Free_t)(void);
typedef BOOL  (__stdcall *BASS_Start_t)(void);
typedef void  (__stdcall *BASS_Stop_t)(void);
typedef BOOL  (__stdcall *BASS_Pause_t)(void);
typedef DWORD (__stdcall *BASS_GetVersion_t)(void);
typedef int   (__stdcall *BASS_ErrorGetCode_t)(void);
typedef BOOL  (__stdcall *BASS_SetVolume_t)(float);
typedef float (__stdcall *BASS_GetVolume_t)(void);
typedef BOOL  (__stdcall *BASS_SetConfig_t)(DWORD,DWORD);
typedef DWORD (__stdcall *BASS_GetConfig_t)(DWORD);
typedef DWORD (__stdcall *BASS_GetDevice_t)(void);
typedef BOOL  (__stdcall *BASS_SetDevice_t)(DWORD);
typedef BOOL  (__stdcall *BASS_GetInfo_t)(void*);
typedef int   (__stdcall *BASS_GetDeviceInfo_t)(DWORD,void*);
typedef BOOL  (__stdcall *BASS_Update_t)(DWORD);
typedef DWORD (__stdcall *BASS_SampleLoad_t)(BOOL,const char*,QWORD,DWORD,DWORD,DWORD);
typedef DWORD (__stdcall *BASS_SampleCreate_t)(DWORD,DWORD,DWORD,DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_SampleFree_t)(DWORD);
typedef DWORD (__stdcall *BASS_SampleGetChannel_t)(DWORD,BOOL);
typedef BOOL  (__stdcall *BASS_SampleGetInfo_t)(DWORD,void*);
typedef BOOL  (__stdcall *BASS_SampleSetInfo_t)(DWORD,void*);
typedef DWORD (__stdcall *BASS_StreamCreate_t)(DWORD,DWORD,DWORD,void*,void*);
typedef DWORD (__stdcall *BASS_StreamCreateFile_t)(BOOL,const void*,QWORD,QWORD,DWORD);
typedef DWORD (__stdcall *BASS_StreamCreateURL_t)(const char*,DWORD,DWORD,void*,void*);
typedef BOOL  (__stdcall *BASS_StreamFree_t)(DWORD);
typedef QWORD (__stdcall *BASS_StreamGetFilePosition_t)(DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_ChannelPlay_t)(DWORD,BOOL);
typedef BOOL  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef BOOL  (__stdcall *BASS_ChannelPause_t)(DWORD);
typedef BOOL  (__stdcall *BASS_ChannelSetPosition_t)(DWORD,QWORD,DWORD);
typedef QWORD (__stdcall *BASS_ChannelGetPosition_t)(DWORD,DWORD);
typedef QWORD (__stdcall *BASS_ChannelGetLength_t)(DWORD,DWORD);
typedef DWORD (__stdcall *BASS_ChannelSetSync_t)(DWORD,DWORD,QWORD,void*,void*);
typedef BOOL  (__stdcall *BASS_ChannelRemoveSync_t)(DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_ChannelSetAttribute_t)(DWORD,DWORD,float);
typedef BOOL  (__stdcall *BASS_ChannelGetAttribute_t)(DWORD,DWORD,float*);
typedef BOOL  (__stdcall *BASS_ChannelSlideAttribute_t)(DWORD,DWORD,float,DWORD);
typedef BOOL  (__stdcall *BASS_ChannelIsSliding_t)(DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_ChannelGetInfo_t)(DWORD,void*);
typedef BOOL  (__stdcall *BASS_ChannelUpdate_t)(DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_ChannelLock_t)(DWORD,BOOL);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(BOOL,const void*,QWORD,DWORD,DWORD,DWORD);
typedef BOOL  (__stdcall *BASS_MusicFree_t)(DWORD);
typedef BOOL  (__stdcall *BASS_RecordInit_t)(int);
typedef BOOL  (__stdcall *BASS_RecordFree_t)(void);
typedef DWORD (__stdcall *BASS_RecordGetDevice_t)(void);
typedef BOOL  (__stdcall *BASS_RecordSetDevice_t)(DWORD);
typedef BOOL  (__stdcall *BASS_RecordGetInfo_t)(void*);
typedef DWORD (__stdcall *BASS_RecordStart_t)(DWORD,DWORD,DWORD,void*,void*);
typedef DWORD (__stdcall *BASS_ChannelGetLevel_t)(DWORD);
typedef BOOL  (__stdcall *BASS_PluginLoad_t)(const char*,DWORD);
typedef BOOL  (__stdcall *BASS_PluginFree_t)(DWORD);

#define LOAD_FN(name) static name##_t real_##name
LOAD_FN(BASS_Init); LOAD_FN(BASS_Free); LOAD_FN(BASS_Start);
LOAD_FN(BASS_Stop); LOAD_FN(BASS_Pause); LOAD_FN(BASS_GetVersion);
LOAD_FN(BASS_ErrorGetCode); LOAD_FN(BASS_SetVolume); LOAD_FN(BASS_GetVolume);
LOAD_FN(BASS_SetConfig); LOAD_FN(BASS_GetConfig); LOAD_FN(BASS_GetDevice);
LOAD_FN(BASS_SetDevice); LOAD_FN(BASS_GetInfo); LOAD_FN(BASS_GetDeviceInfo);
LOAD_FN(BASS_Update); LOAD_FN(BASS_SampleLoad); LOAD_FN(BASS_SampleCreate);
LOAD_FN(BASS_SampleFree); LOAD_FN(BASS_SampleGetChannel); LOAD_FN(BASS_SampleGetInfo);
LOAD_FN(BASS_SampleSetInfo); LOAD_FN(BASS_StreamCreate); LOAD_FN(BASS_StreamCreateFile);
LOAD_FN(BASS_StreamCreateURL); LOAD_FN(BASS_StreamFree); LOAD_FN(BASS_StreamGetFilePosition);
LOAD_FN(BASS_ChannelPlay); LOAD_FN(BASS_ChannelStop); LOAD_FN(BASS_ChannelPause);
LOAD_FN(BASS_ChannelSetPosition); LOAD_FN(BASS_ChannelGetPosition); LOAD_FN(BASS_ChannelGetLength);
LOAD_FN(BASS_ChannelSetSync); LOAD_FN(BASS_ChannelRemoveSync); LOAD_FN(BASS_ChannelSetAttribute);
LOAD_FN(BASS_ChannelGetAttribute); LOAD_FN(BASS_ChannelSlideAttribute); LOAD_FN(BASS_ChannelIsSliding);
LOAD_FN(BASS_ChannelGetInfo); LOAD_FN(BASS_ChannelUpdate); LOAD_FN(BASS_ChannelLock);
LOAD_FN(BASS_MusicLoad); LOAD_FN(BASS_MusicFree); LOAD_FN(BASS_RecordInit);
LOAD_FN(BASS_RecordFree); LOAD_FN(BASS_RecordGetDevice); LOAD_FN(BASS_RecordSetDevice);
LOAD_FN(BASS_RecordGetInfo); LOAD_FN(BASS_RecordStart); LOAD_FN(BASS_ChannelGetLevel);
LOAD_FN(BASS_PluginLoad); LOAD_FN(BASS_PluginFree);

#undef LOAD_FN
#define LOAD_FN(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)

static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH]; HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p+1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (!g_hRealBass) return;
    LOAD_FN(BASS_Init); LOAD_FN(BASS_Free); LOAD_FN(BASS_Start);
    LOAD_FN(BASS_Stop); LOAD_FN(BASS_Pause); LOAD_FN(BASS_GetVersion);
    LOAD_FN(BASS_ErrorGetCode); LOAD_FN(BASS_SetVolume); LOAD_FN(BASS_GetVolume);
    LOAD_FN(BASS_SetConfig); LOAD_FN(BASS_GetConfig); LOAD_FN(BASS_GetDevice);
    LOAD_FN(BASS_SetDevice); LOAD_FN(BASS_GetInfo); LOAD_FN(BASS_GetDeviceInfo);
    LOAD_FN(BASS_Update); LOAD_FN(BASS_SampleLoad); LOAD_FN(BASS_SampleCreate);
    LOAD_FN(BASS_SampleFree); LOAD_FN(BASS_SampleGetChannel); LOAD_FN(BASS_SampleGetInfo);
    LOAD_FN(BASS_SampleSetInfo); LOAD_FN(BASS_StreamCreate); LOAD_FN(BASS_StreamCreateFile);
    LOAD_FN(BASS_StreamCreateURL); LOAD_FN(BASS_StreamFree); LOAD_FN(BASS_StreamGetFilePosition);
    LOAD_FN(BASS_ChannelPlay); LOAD_FN(BASS_ChannelStop); LOAD_FN(BASS_ChannelPause);
    LOAD_FN(BASS_ChannelSetPosition); LOAD_FN(BASS_ChannelGetPosition); LOAD_FN(BASS_ChannelGetLength);
    LOAD_FN(BASS_ChannelSetSync); LOAD_FN(BASS_ChannelRemoveSync); LOAD_FN(BASS_ChannelSetAttribute);
    LOAD_FN(BASS_ChannelGetAttribute); LOAD_FN(BASS_ChannelSlideAttribute); LOAD_FN(BASS_ChannelIsSliding);
    LOAD_FN(BASS_ChannelGetInfo); LOAD_FN(BASS_ChannelUpdate); LOAD_FN(BASS_ChannelLock);
    LOAD_FN(BASS_MusicLoad); LOAD_FN(BASS_MusicFree); LOAD_FN(BASS_RecordInit);
    LOAD_FN(BASS_RecordFree); LOAD_FN(BASS_RecordGetDevice); LOAD_FN(BASS_RecordSetDevice);
    LOAD_FN(BASS_RecordGetInfo); LOAD_FN(BASS_RecordStart); LOAD_FN(BASS_ChannelGetLevel);
    LOAD_FN(BASS_PluginLoad); LOAD_FN(BASS_PluginFree);
}

#undef LOAD_FN

__declspec(dllexport) BOOL  __stdcall BASS_Init(int a,DWORD b,DWORD c,HWND d,void*e){return real_BASS_Init?real_BASS_Init(a,b,c,d,e):FALSE;}
__declspec(dllexport) void  __stdcall BASS_Free(void){if(real_BASS_Free)real_BASS_Free();}
__declspec(dllexport) BOOL  __stdcall BASS_Start(void){return real_BASS_Start?real_BASS_Start():FALSE;}
__declspec(dllexport) void  __stdcall BASS_Stop(void){if(real_BASS_Stop)real_BASS_Stop();}
__declspec(dllexport) BOOL  __stdcall BASS_Pause(void){return real_BASS_Pause?real_BASS_Pause():FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_GetVersion(void){return real_BASS_GetVersion?real_BASS_GetVersion():0;}
__declspec(dllexport) int   __stdcall BASS_ErrorGetCode(void){return real_BASS_ErrorGetCode?real_BASS_ErrorGetCode():0;}
__declspec(dllexport) BOOL  __stdcall BASS_SetVolume(float a){return real_BASS_SetVolume?real_BASS_SetVolume(a):FALSE;}
__declspec(dllexport) float __stdcall BASS_GetVolume(void){return real_BASS_GetVolume?real_BASS_GetVolume():0.0f;}
__declspec(dllexport) BOOL  __stdcall BASS_SetConfig(DWORD a,DWORD b){return real_BASS_SetConfig?real_BASS_SetConfig(a,b):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_GetConfig(DWORD a){return real_BASS_GetConfig?real_BASS_GetConfig(a):0;}
__declspec(dllexport) DWORD __stdcall BASS_GetDevice(void){return real_BASS_GetDevice?real_BASS_GetDevice():0;}
__declspec(dllexport) BOOL  __stdcall BASS_SetDevice(DWORD a){return real_BASS_SetDevice?real_BASS_SetDevice(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_GetInfo(void*a){return real_BASS_GetInfo?real_BASS_GetInfo(a):FALSE;}
__declspec(dllexport) int   __stdcall BASS_GetDeviceInfo(DWORD a,void*b){return real_BASS_GetDeviceInfo?real_BASS_GetDeviceInfo(a,b):0;}
__declspec(dllexport) BOOL  __stdcall BASS_Update(DWORD a){return real_BASS_Update?real_BASS_Update(a):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(BOOL a,const char*b,QWORD c,DWORD d,DWORD e,DWORD f){return real_BASS_SampleLoad?real_BASS_SampleLoad(a,b,c,d,e,f):0;}
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a,DWORD b,DWORD c,DWORD d,DWORD e){return real_BASS_SampleCreate?real_BASS_SampleCreate(a,b,c,d,e):0;}
__declspec(dllexport) BOOL  __stdcall BASS_SampleFree(DWORD a){return real_BASS_SampleFree?real_BASS_SampleFree(a):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a,BOOL b){return real_BASS_SampleGetChannel?real_BASS_SampleGetChannel(a,b):0;}
__declspec(dllexport) BOOL  __stdcall BASS_SampleGetInfo(DWORD a,void*b){return real_BASS_SampleGetInfo?real_BASS_SampleGetInfo(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_SampleSetInfo(DWORD a,void*b){return real_BASS_SampleSetInfo?real_BASS_SampleSetInfo(a,b):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_StreamCreate(DWORD a,DWORD b,DWORD c,void*d,void*e){return real_BASS_StreamCreate?real_BASS_StreamCreate(a,b,c,d,e):0;}
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(BOOL a,const void*b,QWORD c,QWORD d,DWORD e){return real_BASS_StreamCreateFile?real_BASS_StreamCreateFile(a,b,c,d,e):0;}
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateURL(const char*a,DWORD b,DWORD c,void*d,void*e){return real_BASS_StreamCreateURL?real_BASS_StreamCreateURL(a,b,c,d,e):0;}
__declspec(dllexport) BOOL  __stdcall BASS_StreamFree(DWORD a){return real_BASS_StreamFree?real_BASS_StreamFree(a):FALSE;}
__declspec(dllexport) QWORD __stdcall BASS_StreamGetFilePosition(DWORD a,DWORD b){return real_BASS_StreamGetFilePosition?real_BASS_StreamGetFilePosition(a,b):0;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelPlay(DWORD a,BOOL b){return real_BASS_ChannelPlay?real_BASS_ChannelPlay(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelStop(DWORD a){return real_BASS_ChannelStop?real_BASS_ChannelStop(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelPause(DWORD a){return real_BASS_ChannelPause?real_BASS_ChannelPause(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSetPosition(DWORD a,QWORD b,DWORD c){return real_BASS_ChannelSetPosition?real_BASS_ChannelSetPosition(a,b,c):FALSE;}
__declspec(dllexport) QWORD __stdcall BASS_ChannelGetPosition(DWORD a,DWORD b){return real_BASS_ChannelGetPosition?real_BASS_ChannelGetPosition(a,b):0;}
__declspec(dllexport) QWORD __stdcall BASS_ChannelGetLength(DWORD a,DWORD b){return real_BASS_ChannelGetLength?real_BASS_ChannelGetLength(a,b):0;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a,DWORD b,QWORD c,void*d,void*e){return real_BASS_ChannelSetSync?real_BASS_ChannelSetSync(a,b,c,d,e):0;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelRemoveSync(DWORD a,DWORD b){return real_BASS_ChannelRemoveSync?real_BASS_ChannelRemoveSync(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSetAttribute(DWORD a,DWORD b,float c){return real_BASS_ChannelSetAttribute?real_BASS_ChannelSetAttribute(a,b,c):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelGetAttribute(DWORD a,DWORD b,float*c){return real_BASS_ChannelGetAttribute?real_BASS_ChannelGetAttribute(a,b,c):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelSlideAttribute(DWORD a,DWORD b,float c,DWORD d){return real_BASS_ChannelSlideAttribute?real_BASS_ChannelSlideAttribute(a,b,c,d):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelIsSliding(DWORD a,DWORD b){return real_BASS_ChannelIsSliding?real_BASS_ChannelIsSliding(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelGetInfo(DWORD a,void*b){return real_BASS_ChannelGetInfo?real_BASS_ChannelGetInfo(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelUpdate(DWORD a,DWORD b){return real_BASS_ChannelUpdate?real_BASS_ChannelUpdate(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_ChannelLock(DWORD a,BOOL b){return real_BASS_ChannelLock?real_BASS_ChannelLock(a,b):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(BOOL a,const void*b,QWORD c,DWORD d,DWORD e,DWORD f){return real_BASS_MusicLoad?real_BASS_MusicLoad(a,b,c,d,e,f):0;}
__declspec(dllexport) BOOL  __stdcall BASS_MusicFree(DWORD a){return real_BASS_MusicFree?real_BASS_MusicFree(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_RecordInit(int a){return real_BASS_RecordInit?real_BASS_RecordInit(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_RecordFree(void){return real_BASS_RecordFree?real_BASS_RecordFree():FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_RecordGetDevice(void){return real_BASS_RecordGetDevice?real_BASS_RecordGetDevice():0;}
__declspec(dllexport) BOOL  __stdcall BASS_RecordSetDevice(DWORD a){return real_BASS_RecordSetDevice?real_BASS_RecordSetDevice(a):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_RecordGetInfo(void*a){return real_BASS_RecordGetInfo?real_BASS_RecordGetInfo(a):FALSE;}
__declspec(dllexport) DWORD __stdcall BASS_RecordStart(DWORD a,DWORD b,DWORD c,void*d,void*e){return real_BASS_RecordStart?real_BASS_RecordStart(a,b,c,d,e):0;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a){return real_BASS_ChannelGetLevel?real_BASS_ChannelGetLevel(a):0;}
__declspec(dllexport) BOOL  __stdcall BASS_PluginLoad(const char*a,DWORD b){return real_BASS_PluginLoad?real_BASS_PluginLoad(a,b):FALSE;}
__declspec(dllexport) BOOL  __stdcall BASS_PluginFree(DWORD a){return real_BASS_PluginFree?real_BASS_PluginFree(a):FALSE;}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch thread + DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI PatchThread(LPVOID param) {
    Sleep(2000);
    g_moduleBase = (DWORD)GetModuleHandleA("Hamsterball.exe");
    if (!g_moduleBase) g_moduleBase = 0x00400000;
    g_operatorNew = (operator_new_t)(g_moduleBase + RVA_operator_new);
    g_LevelMeshWorldCtor = (Level_MeshWorldCtor_t)(g_moduleBase + RVA_Level_MeshWorldCtor);
    g_LevelRenderCtor = (Level_RenderCtor_t)(g_moduleBase + RVA_Level_RenderCtor);
    g_LevelInitScene = (Level_InitScene_t)(g_moduleBase + RVA_Level_InitScene);
    g_SoundPlay3D = (Sound_Play3D_t)(g_moduleBase + RVA_Sound_Play3D);
    g_CollectByNameFilter = (Scene_CollectByNameFilter_t)(g_moduleBase + RVA_Scene_CollectByNameFilter);
    g_AthenaStringFormat = (AthenaString_Format_t)(g_moduleBase + RVA_AthenaString_Format);
    g_AthenaListInit = (AthenaList_Init_t)(g_moduleBase + RVA_AthenaList_Init);

    GetConfigPath();
    LoadConfig();
    PatchAllocSizes();
    InstallUniversalConstructorHook();
    InstallHook();
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        CreateThread(NULL, 0, PatchThread, NULL, 0, NULL);
    }
    return TRUE;
}
