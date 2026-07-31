/*
 * WARP (Level Warp) Mod v8
 *
 * CHANGES FROM v7:
 *   - REPLACED the NOP-based timer freeze (3 patches across 2 functions +
 *     per-frame board+0x3624 write-back) with a clean code-cave approach.
 *     Two small caves check a custom g_freezeTimer flag alongside the
 *     original "finished" flag. When g_freezeTimer=1, the DEC/INC of the
 *     per-player timer (App+PID*0xA0+0x5E8, obj+0x1C) is skipped — same
 *     as if the player had reached the goal, but WITHOUT any N:GOAL side
 *     effects (no music, no camera change, no BTT save, no popup).
 *   - REMOVED: freeze_timer_decrement(), restore_timer_decrement(),
 *     g_timerFrozen, g_savedTimerValue, g_timerOrigBytes, g_timerInc1/2OrigBytes,
 *     TIMER_DEC_ADDR, TIMER_INC1_ADDR, TIMER_INC2_ADDR, and all their
 *     save/restore logic (~200 lines deleted).
 *   - REMOVED: board+0x3624 write-back logic in updateWarpStateMachine()
 *     (the g_whiteAlpha vs g_savedTimerValue dance). The fade alpha now
 *     works naturally — we don't touch it at all.
 *   - The fade alpha (board+0x3624) is no longer frozen. It decrements
 *     and increments normally. The warp effect uses the game's native
 *     fade at scene+0x3624 for the white flash by writing g_whiteAlpha
 *     to it during fade phases only.
 *
 * v7 changes (carried forward):
 *   - Node-point proximity trigger (WARP(Name) in MESHWORLD Section 1)
 *   - No collision hook
 *   - 2-second cooldown after warp
 *   - Pause-aware game clock
 *
 * Timer freeze mechanism:
 *   Board_UpdateRaceState (0x41B130) decrements obj+0x1C (App+PID*0xA0+0x5E8)
 *   each frame. The DEC is gated by obj+0x0A (finished flag) via JNZ at 0x41B3E5.
 *   We patch the JNZ+DEC (9 bytes) with a JMP to a cave that checks BOTH the
 *   original finished flag AND g_freezeTimer. If either is set, skip the DEC.
 *
 *   The practice/TT/party path at 0x41B50C (JNZ+INC, 5 bytes) gets the same
 *   treatment — cave checks both flags, skips INC if either is set.
 *
 *   This is the same passive mechanism the game uses for N:GOAL — we just add
 *   a second condition to the same check.
 *
 * Phase timeline (real-time, not frame-based):
 *   RUMBLE: 2.0 sec — Ball frozen + CPUID jitter + sound + music fade start
 *   FLASH:  0.25 sec — Ball invisible + quick white flash
 *   HOLD:   1.0 sec — Pause, screen clear, ball stays invisible
 *   FADE:   2.0 sec — Screen fades to solid white
 *   LOAD:   instant — Load target level while screen stays white
 *   REVEAL: 1.0 sec — Fade from white to reveal new level
 *   Total:  ~6.25 sec
 *
 * Music fade: 3.0 sec starting at RUMBLE start
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll warp_mod_v8.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ============================================================
 * Diagnostic logging — uses vsnprintf (supports %f, unlike wvsprintfA)
 * ============================================================ */

static char g_logPath[MAX_PATH] = "";

static void diag_log(const char *msg) {
    if (g_logPath[0] == '\0') return;
    {
        HANDLE hFile = CreateFileA(g_logPath,
            FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
            WriteFile(hFile, "\r\n", 2, &written, NULL);
            CloseHandle(hFile);
        }
    }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list args;
    if (g_logPath[0] == '\0') return;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}

/* ============================================================
 * BASS Proxy Exports
 * ============================================================ */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, float b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}

typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, BOOL c) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c);
    return 1;
}

typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}

typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, DWORD, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, DWORD d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}

typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}

typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

/* Extra BASS stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) { (void)a; }
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { (void)a; return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) { (void)a; }
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { (void)a; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { (void)a;(void)b; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { (void)a;(void)b;(void)c; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { (void)a;(void)b; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { (void)a; return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { (void)a;(void)b; return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { (void)a;(void)b;(void)c;(void)d;(void)e; return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { (void)a;(void)b; return 0; }

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char *p = strrchr(path, '\\');
            if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
        }
    }
    if (g_hRealBass) {
        real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
        real_BASS_MusicPlayEx          = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
        real_BASS_SetConfig             = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
        real_BASS_Init                  = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
        real_BASS_Free                  = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
        real_BASS_Start                 = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
        real_BASS_Stop                  = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
        real_BASS_ErrorGetCode          = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
        real_BASS_MusicLoad             = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        real_BASS_ChannelStop            = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    }
}

/* ============================================================
 * Game constants (verified via Ghidra decompilation)
 * ============================================================ */

#define EXE_BASE        0x00400000
#define APP_PTR         0x005341E0

/* Function addresses */
#define APP_START_PRACTICE_RACE     0x00428C50

/* App offsets */
#define APP_PROFILE_PTR          0x220
#define APP_MUSIC_DEVICE_PTR     0x17C
#define APP_BTT_RECORDING        0x90C
#define APP_BTT_PLAYBACK         0x910
#define APP_TIMER_FINISHED       0x5D6  /* obj+0x0A: finished flag (set on timeout or goal) */

/* Board/Scene offsets */
#define BOARD_SCENE_PTR_OFFSET   0x878

/* MeshWorld access (from decompiled Scene_SetupLevelUp @ 0x411540) */
#define BOARD_MESHWORLD_PTR      0x8AC
#define MW_SECTION3_PTR          0x480
#define MW_NODELIST_OFFSET       0x894
#define MW_NODELIST_COUNT        0x898
#define MW_NODELIST_DATA         0xCA0

/* Node entry layout (16 bytes each) */
#define NODE_NAME_OFFSET         0x00
#define NODE_X_OFFSET            0x04
#define NODE_Y_OFFSET            0x08
#define NODE_Z_OFFSET            0x0C

/* Ball list access (from decompiled CollisionFace_Update @ 0x43D160) */
#define BALL_LIST_OFFSET         0x29D4
#define BALL_LIST_DATA_OFFSET    0x2DE0

/* PlayerProfile offsets */
#define PROFILE_RACE_INDEX       0x08
#define PROFILE_BOARD_PTR        0x0C
#define PROFILE_IS_PRACTICE      0x11
#define PROFILE_SCORE_ARRAY      0x50
#define PROFILE_TIME_ARRAY       0x14

/* Ball offsets */
#define BALL_DEATH_PENDING       0x2E9
#define BALL_POS_X               0x164
#define BALL_POS_Y               0x168
#define BALL_POS_Z               0x16C
#define BALL_IMPACT_FREEZE       0x808
#define BALL_IN_TAR              0x2CC
#define BALL_ALPHA               0x2FC
#define BALL_RENDER_JITTER       0x2D4

/* Ball runtime color multiplier — read by Ball_Render every frame and passed
 * to Graphics_SetColorMultiplier(0x00401160). These are the SAME fields the
 * magnifying glass heat system uses (ball+0xC50 subtracts from G and B).
 * Writing here immediately affects the rendered ball color. */
#define BALL_COLOR_R            0x2AC
#define BALL_COLOR_G            0x2B0
#define BALL_COLOR_B            0x2B4

/* Board goal-reached flag */
#define BOARD_GOAL_REACHED       0xCD0

/* Pause-block patch addresses (RVA from exe base) */
#define PAUSE_PATCH_PATH1        0x19D5B
#define PAUSE_PATCH_PATH2        0x130B5
#define PAUSE_PATCH_PATH3        0x0B405
static BYTE g_pauseOrigBytes[3] = {0, 0, 0};
static int g_pauseBlocked = 0;

/* MusicChannel offsets */
#define MUSIC_CHAN_BASS_CHANNEL  0x08
#define MUSIC_CHAN_VOLUME       0x528
#define MUSIC_CHAN_FADE_RATE    0x52C
#define MUSIC_CHAN_FADE_OUT     0x530
#define MUSIC_CHAN_FADE_IN      0x531

/* MusicDevice offsets */
#define MUSIC_DEV_CHANNEL_LIST  0x418

/* Proximity thresholds (matching HB+ plus_level_warp) */
#define WARP_TRIGGER_DIST_SQ    625.0f  /* 25.0 squared */

/* Cooldown after warp completes */
#define WARP_COOLDOWN_MS        2000

/* ============================================================
 * Timer freeze — code cave approach
 *
 * Instead of NOP'ing the FSUB/FADD instructions on board+0x3624
 * (which is the fade alpha, NOT the race timer), we patch the
 * actual timer DEC/INC instructions with JMPs to code caves.
 *
 * The caves check both the original "finished" flag (obj+0x0A)
 * and our custom g_freezeTimer flag. If either is set, the
 * DEC/INC is skipped — same passive mechanism the game uses
 * for N:GOAL, just with an extra condition.
 *
 * Patch 1: 0x41B3E5 (JNZ near 6B + DEC 3B = 9 bytes)
 *   Original: JNZ 0x41B49D; DEC [EDI+0x1C]
 *   Patched:  JMP cave1; NOP; NOP; NOP; NOP
 *   Cave1:    JNZ skip_dec (flags from TEST AL,AL at 0x41B3E3)
 *             CMP [g_freezeTimer], 0
 *             JNE skip_dec
 *             DEC [EDI+0x1C]
 *             JMP 0x41B3EE (JNS — next original instruction)
 *   skip_dec: JMP 0x41B49D (original skip target)
 *
 * Patch 2: 0x41B50C (JNZ short 2B + INC 3B = 5 bytes)
 *   Original: JNZ 0x41B511; INC [EDX+0x1C]
 *   Patched:  JMP cave2
 *   Cave2:    JNZ skip_inc (flags from TEST CL,CL at 0x41B50A)
 *             CMP [g_freezeTimer], 0
 *             JNE skip_inc
 *             INC [EDX+0x1C]
 *   skip_inc: JMP 0x41B511 (both paths converge here)
 * ============================================================ */

/* RVAs for patch points */
#define TIMER_DEC_PATCH_RVA     0x1B3E5   /* 9 bytes: JNZ near + DEC */
#define TIMER_DEC_PATCH_SIZE    9
#define TIMER_DEC_RETURN_RVA    0x1B3EE   /* JNS instruction (after DEC) */
#define TIMER_DEC_SKIP_RVA      0x1B49D   /* original JNZ target */

#define TIMER_INC_PATCH_RVA     0x1B50C   /* 5 bytes: JNZ short + INC */
#define TIMER_INC_PATCH_SIZE    5
#define TIMER_INC_RETURN_RVA    0x1B511   /* both paths converge here */

/* The custom freeze flag. 0=normal, 1=timer frozen. */
static volatile BYTE g_freezeTimer = 0;

/* Saved original bytes for restore on unload */
static unsigned char g_decOrigBytes[TIMER_DEC_PATCH_SIZE];
static unsigned char g_incOrigBytes[TIMER_INC_PATCH_SIZE];
static int g_decPatched = 0;
static int g_incPatched = 0;

/* Code cave memory */
static unsigned char *g_decCave = NULL;
static unsigned char *g_incCave = NULL;

/* Build a JMP rel32 instruction (5 bytes) */
static void write_jmp(unsigned char *at, DWORD target) {
    at[0] = 0xE9;
    *(DWORD*)(at + 1) = target - (DWORD)at - 5;
}

/* Install the timer freeze code caves.
 *
 * Cave 1 (DEC path — tournament countdown):
 *   Flags from TEST AL,AL (0x41B3E3) are preserved across JMP.
 *   75 05           JNZ skip_dec        (if finished → skip)
 *   80 3D xx xx xx xx 00  CMP byte [g_freezeTimer], 0
 *   75 02           JNE skip_dec        (if freeze → skip)
 *   FF 4F 1C       DEC [EDI+0x1C]      (original)
 * skip_dec:
 *   E9 xx xx xx xx  JMP 0x41B3EE        (back to JNS)
 *
 *   Wait — JNZ skip_dec needs to jump to the JMP back to skip target.
 *   Let's restructure:
 *
 *   0F 85 xx xx xx xx  JNZ skip_target   (6 bytes, near JNZ — reuse original)
 *   80 3D aa bb cc dd 00  CMP byte [g_freezeTimer], 0  (7 bytes)
 *   0F 85 xx xx xx xx  JNE skip_target   (6 bytes, near JNE)
 *   FF 4F 1C           DEC [EDI+0x1C]   (3 bytes, original)
 *   E9 xx xx xx xx     JMP return       (5 bytes, back to 0x41B3EE)
 * skip_target:
 *   E9 xx xx xx xx     JMP 0x41B49D     (5 bytes, original skip target)
 *
 *   Total cave size: 6+7+6+3+5+5 = 32 bytes
 *
 * Cave 2 (INC path — practice/TT/party):
 *   Flags from TEST CL,CL (0x41B50A) are preserved across JMP.
 *   75 05           JNZ skip_inc        (if finished → skip, short JNZ)
 *   80 3D xx xx xx xx 00  CMP byte [g_freezeTimer], 0  (7 bytes)
 *   75 02           JNE skip_inc        (if freeze → skip, short JNE)
 *   FF 42 1C       INC [EDX+0x1C]      (original)
 * skip_inc:
 *   E9 xx xx xx xx  JMP 0x41B511        (back, both paths converge)
 *
 *   Total cave size: 2+7+2+3+5 = 19 bytes
 */
static void install_timer_caves(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;

    /* --- Cave 1: DEC path (tournament) --- */
    if (!g_decPatched) {
        DWORD patchAddr = base + TIMER_DEC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_DEC_RETURN_RVA;
        DWORD skipAddr = base + TIMER_DEC_SKIP_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        /* Allocate cave (64 bytes, more than enough) */
        g_decCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!g_decCave) {
            diag_log("[warp] FATAL: VirtualAlloc failed for DEC cave");
            return;
        }

        /* Build cave 1 */
        unsigned char *p = g_decCave;

        /* JNZ skip_target (near, 6 bytes) — flags from TEST AL,AL */
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 7 + 6 + 3 + 5) - (DWORD)(p + 2 + 4); /* skip_target offset */
        p += 6;

        /* CMP byte [g_freezeTimer], 0 (7 bytes) */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        /* JNE skip_target (near, 6 bytes) */
        p[0] = 0x0F; p[1] = 0x85;
        *(DWORD*)(p + 2) = (DWORD)(p + 6 + 3 + 5) - (DWORD)(p + 2 + 4); /* skip_target offset */
        p += 6;

        /* DEC [EDI+0x1C] (3 bytes, original) */
        p[0] = 0xFF; p[1] = 0x4F; p[2] = 0x1C;
        p += 3;

        /* JMP return (0x41B3EE) */
        write_jmp(p, returnAddr);
        p += 5;

        /* skip_target: JMP 0x41B49D */
        write_jmp(p, skipAddr);
        p += 5;

        /* Save original bytes and patch */
        if (VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {
            memcpy(g_decOrigBytes, (void*)patchAddr, TIMER_DEC_PATCH_SIZE);
            /* JMP cave (5 bytes) + NOP (4 bytes) */
            write_jmp((unsigned char*)patchAddr, (DWORD)g_decCave);
            memset((unsigned char*)patchAddr + 5, 0x90, 4);
            VirtualProtect((void*)patchAddr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
            g_decPatched = 1;
            diag_log("[warp] DEC timer cave installed at 0x41B3E5 (9 bytes patched)");
        } else {
            diag_log("[warp] WARNING: VirtualProtect failed for DEC patch");
        }
    }

    /* --- Cave 2: INC path (practice/TT/party) --- */
    if (!g_incPatched) {
        DWORD patchAddr = base + TIMER_INC_PATCH_RVA;
        DWORD returnAddr = base + TIMER_INC_RETURN_RVA;
        DWORD freezeAddr = (DWORD)&g_freezeTimer;

        /* Allocate cave (64 bytes) */
        g_incCave = (unsigned char *)VirtualAlloc(NULL, 64, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!g_incCave) {
            diag_log("[warp] FATAL: VirtualAlloc failed for INC cave");
            return;
        }

        /* Build cave 2 */
        {
            unsigned char *p = g_incCave;

        /* JNZ skip_inc (short, 2 bytes) — flags from TEST CL,CL */
        p[0] = 0x75;
        p[1] = 0x0C;  /* skip 12 bytes (7+2+3) to reach skip_inc JMP */
        p += 2;

        /* CMP byte [g_freezeTimer], 0 (7 bytes) */
        p[0] = 0x80; p[1] = 0x3D;
        *(DWORD*)(p + 2) = freezeAddr;
        p[6] = 0x00;
        p += 7;

        /* JNE skip_inc (short, 2 bytes) */
        p[0] = 0x75;
        p[1] = 0x03;  /* skip 3 bytes (INC) to reach skip_inc JMP */
        p += 2;

        /* INC [EDX+0x1C] (3 bytes, original) */
        p[0] = 0xFF; p[1] = 0x42; p[2] = 0x1C;
        p += 3;

        /* skip_inc: JMP 0x41B511 (return, both paths converge) */
        write_jmp(p, returnAddr);
        p += 5;
        }

        /* Save original bytes and patch */
        if (VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {
            memcpy(g_incOrigBytes, (void*)patchAddr, TIMER_INC_PATCH_SIZE);
            /* JMP cave (5 bytes) — exactly fits */
            write_jmp((unsigned char*)patchAddr, (DWORD)g_incCave);
            VirtualProtect((void*)patchAddr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
            g_incPatched = 1;
            diag_log("[warp] INC timer cave installed at 0x41B50C (5 bytes patched)");
        } else {
            diag_log("[warp] WARNING: VirtualProtect failed for INC patch");
        }
    }
}

/* Restore original timer instructions */
static void restore_timer_caves(void) {
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;

    if (g_decPatched) {
        DWORD addr = base + TIMER_DEC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_decOrigBytes, TIMER_DEC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_DEC_PATCH_SIZE, oldProt, &oldProt);
        }
        g_decPatched = 0;
        diag_log("[warp] DEC timer cave restored");
    }

    if (g_incPatched) {
        DWORD addr = base + TIMER_INC_PATCH_RVA;
        if (VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, PAGE_READWRITE, &oldProt)) {
            memcpy((void*)addr, g_incOrigBytes, TIMER_INC_PATCH_SIZE);
            VirtualProtect((void*)addr, TIMER_INC_PATCH_SIZE, oldProt, &oldProt);
        }
        g_incPatched = 0;
        diag_log("[warp] INC timer cave restored");
    }
}

/* ============================================================
 * Level name -> race index mapping
 * ============================================================ */

typedef struct {
    const char* meshName;
    int raceIndex;
} LevelMapping;

static const LevelMapping levelMap[] = {
    {"level1", 1}, {"warmup", 1}, {"warm-up", 1},
    {"level2", 2}, {"beginner", 2}, {"cascade", 2},
    {"level3", 3}, {"intermediate", 3},
    {"level4", 4}, {"dizzy", 4},
    {"level5", 5}, {"tower", 5},
    {"level6", 6}, {"up", 6},
    {"level7", 7}, {"neon", 7},
    {"level8", 8}, {"expert", 8},
    {"level9", 9}, {"odd", 9},
    {"level10", 10}, {"toob", 10},
    {"level11", 11}, {"wobbly", 11},
    {"level12", 12}, {"glass", 12},
    {"level13", 13}, {"sky", 13},
    {"level14", 14}, {"master", 14},
    {"level15", 15}, {"impossible", 15},
    {NULL, 0}
};

static int findRaceIndex(const char *levelName) {
    if (levelName[0] >= '1' && levelName[0] <= '9') {
        int num = atoi(levelName);
        if (num >= 1 && num <= 15) return num;
    }
    {
        int j;
        for (j = 0; levelMap[j].meshName; j++) {
            if (_stricmp(levelName, levelMap[j].meshName) == 0)
                return levelMap[j].raceIndex;
        }
    }
    return -1;
}

/* ============================================================
 * Warp state machine — time-based (game clock), not frame-based
 * ============================================================ */

typedef enum {
    PHASE_IDLE = 0,
    PHASE_RUMBLE,
    PHASE_FLASH,
    PHASE_HOLD,
    PHASE_FADE,
    PHASE_LOAD,
    PHASE_REVEAL
} WarpPhase;

static volatile WarpPhase g_phase = PHASE_IDLE;
static volatile DWORD g_phaseStartTime = 0;
static volatile DWORD g_warpStartTime = 0;
static volatile int g_warpLevelIndex = -1;
static volatile int g_musicFadeStarted = 0;
static volatile DWORD g_cooldownUntil = 0;

/* Pause-aware game clock */
static volatile DWORD g_gameClock = 0;
static volatile DWORD g_lastRealTime = 0;

#define MAX_MUSIC_CHANNELS 8
static float g_musicOrigVolumes[MAX_MUSIC_CHANNELS];
static int g_musicChannelCount = 0;

#define RUMBLE_DURATION_MS    2000
#define FLASH_DURATION_MS      250
#define HOLD_DURATION_MS      1000
#define FADE_DURATION_MS      2000
#define REVEAL_DURATION_MS    1000
#define MUSIC_FADE_MS         3000

#define FLASH_PEAK_MS         125

static volatile float g_whiteAlpha = 0.0f;

static volatile int g_warpBall = 0;
static volatile int g_rumbleInit = 0;

/* Saved original ball color multiplier (ball+0x2AC/0x2B0/0x2B4) for restore after warp.
 * These are the runtime color fields read by Ball_Render every frame —
 * the same fields the magnifying glass heat system subtracts from. */
static float g_origBallR = 1.0f, g_origBallG = 1.0f, g_origBallB = 1.0f;
static int g_colorSaved = 0;

/* Target electric blue (#03fff2) for the RUMBLE phase color transition.
 * Hex #03fff2 → R=0x03/255=0.0118, G=0xFF/255=1.0, B=0xF2/255=0.9490 */
#define TARGET_BLUE_R  (3.0f / 255.0f)    /* 0.0118 */
#define TARGET_BLUE_G  (255.0f / 255.0f)  /* 1.0    */
#define TARGET_BLUE_B  (242.0f / 255.0f)  /* 0.9490 */

/* Board offset: scene+0x874 = paused flag */
#define BOARD_PAUSED_FLAG      0x874

/* Scene offset for the game's native fade alpha */
#define SCENE_FADE_ALPHA        0x3624

/* ============================================================
 * Memory helpers
 * ============================================================ */

static void block_pause(void) {
    if (g_pauseBlocked) return;
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    int i;
    for (i = 0; i < 3; i++) {
        DWORD addr = base + addrs[i];
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            g_pauseOrigBytes[i] = *((BYTE*)addr);
            *((BYTE*)addr) = 0xEB;
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    g_pauseBlocked = 1;
    diag_log("[warp] Pause blocked (3 paths patched)");
}

static void unblock_pause(void) {
    if (!g_pauseBlocked) return;
    HMODULE hExe = GetModuleHandleA("Hamsterball.exe");
    if (!hExe) hExe = GetModuleHandleA(NULL);
    DWORD base = (DWORD)hExe;
    DWORD oldProt;
    DWORD addrs[3] = {PAUSE_PATCH_PATH1, PAUSE_PATCH_PATH2, PAUSE_PATCH_PATH3};
    int i;
    for (i = 0; i < 3; i++) {
        DWORD addr = base + addrs[i];
        if (VirtualProtect((void*)addr, 1, PAGE_READWRITE, &oldProt)) {
            *((BYTE*)addr) = g_pauseOrigBytes[i];
            VirtualProtect((void*)addr, 1, oldProt, &oldProt);
        }
    }
    g_pauseBlocked = 0;
    diag_log("[warp] Pause unblocked (originals restored)");
}

static int GetApp(void) {
    return *(int *)APP_PTR;
}

static int is_valid_ball(int ball) {
    if (!ball) return 0;
    if (ball < 0x10000) return 0;
    return 1;
}

static int is_valid_ptr(int ptr) {
    if (!ptr) return 0;
    if (ptr < 0x10000) return 0;
    return 1;
}

/* ============================================================
 * Pause-aware game clock
 * ============================================================ */

static int is_game_paused(int board) {
    if (!board) return 0;
    if (IsBadReadPtr((void *)(board + BOARD_PAUSED_FLAG), 1)) return 0;
    return *((unsigned char *)((char *)board + BOARD_PAUSED_FLAG)) != 0;
}

static DWORD getGameTime(void) {
    return g_gameClock;
}

static void updateGameClock(int board) {
    DWORD now = GetTickCount();
    if (g_lastRealTime == 0) {
        g_lastRealTime = now;
        return;
    }
    if (!is_game_paused(board)) {
        g_gameClock += (now - g_lastRealTime);
    }
    g_lastRealTime = now;
}

/* ============================================================
 * Parse level name from WARP(LevelName)
 * ============================================================ */

static int isWarpNode(const char *nodeName) {
    if (!nodeName) return 0;
    return _strnicmp(nodeName, "WARP(", 5) == 0;
}

static int parseWarpLevel(const char *nodeName, char *outLevelName, int outSize) {
    const char *start = strchr(nodeName, '(');
    const char *end;
    int len;
    if (!start) return 0;
    start++;
    end = strchr(start, ')');
    if (!end) return 0;
    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;
    strncpy(outLevelName, start, len);
    outLevelName[len] = 0;
    return 1;
}

/* ============================================================
 * Music fade — fade out music over MUSIC_FADE_MS (3 seconds)
 * ============================================================ */

static void startMusicFade(void) {
    int app = GetApp();
    int musicDev;
    int chanListData;
    int chanCount;
    int i;

    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        g_musicChannelCount = count;
        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                float vol = *(float *)(chan + MUSIC_CHAN_VOLUME);
                g_musicOrigVolumes[i] = vol;
            } else {
                g_musicOrigVolumes[i] = 0.0f;
            }
        }

        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                *(char *)((char *)chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(char *)((char *)chan + MUSIC_CHAN_FADE_IN) = 0;
                *(float *)((char *)chan + MUSIC_CHAN_FADE_RATE) = 0.0f;
            }
        }
    }

    g_musicFadeStarted = 1;
}

static void updateMusicFade(void) {
    int app, musicDev;
    int chanListData, chanCount, i;
    DWORD elapsed;
    float t;

    if (!g_musicFadeStarted) return;

    elapsed = getGameTime() - g_warpStartTime;
    t = (float)elapsed / (float)MUSIC_FADE_MS;
    if (t > 1.0f) t = 1.0f;

    app = GetApp();
    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                float origVol = g_musicOrigVolumes[i];
                float chanVol = origVol * (1.0f - t);
                *(float *)(chan + MUSIC_CHAN_VOLUME) = chanVol;
                if (real_BASS_ChannelSetAttributes) {
                    int bassChan = *(int *)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) {
                        real_BASS_ChannelSetAttributes(bassChan, -1.0f, (int)(chanVol * 100.0f), -1);
                    }
                }
            }
        }
    }
}

/* Restore music channel volumes and re-enable the game's native fade-in
 * system after a warp. startMusicFade() zeroed the fade flags and fade_rate
 * on each channel and manually drove volume to 0. Without this restore,
 * channels are left at volume 0 with fades disabled — music stays silent
 * on the destination level.
 *
 * We restore the saved original volume and set fade_in=1 so the game's
 * own MusicChannel_FadeUpdate brings the volume back naturally. */
static void restoreMusicFade(void) {
    int app, musicDev;
    int chanListData, chanCount, i;

    if (!g_musicFadeStarted) return;

    app = GetApp();
    if (!app) return;
    musicDev = *(int *)((char *)app + APP_MUSIC_DEVICE_PTR);
    if (!musicDev) return;

    chanListData = *(int *)(musicDev + MUSIC_DEV_CHANNEL_LIST);
    chanCount = *(int *)(musicDev + 0x10);

    if (chanCount > 0 && chanListData) {
        int count = (chanCount < MAX_MUSIC_CHANNELS) ? chanCount : MAX_MUSIC_CHANNELS;
        for (i = 0; i < count; i++) {
            int chan = *(int *)(chanListData + i * 4);
            if (chan) {
                /* Restore original volume */
                *(float *)(chan + MUSIC_CHAN_VOLUME) = g_musicOrigVolumes[i];
                /* Re-enable fade-in so the game's native system can manage it */
                *(char *)((char *)chan + MUSIC_CHAN_FADE_OUT) = 0;
                *(char *)((char *)chan + MUSIC_CHAN_FADE_IN) = 1;
                /* Leave fade_rate at 0 — the game sets it when it starts
                 * a new song via Audio_PlayMusic. fade_in=1 with rate=0
                 * means FadeUpdate will add 0 each frame (no-op), but
                 * the flag being set prevents us from clobbering it again. */
                if (real_BASS_ChannelSetAttributes) {
                    int bassChan = *(int *)(chan + MUSIC_CHAN_BASS_CHANNEL);
                    if (bassChan) {
                        real_BASS_ChannelSetAttributes(bassChan, -1.0f,
                            (int)(g_musicOrigVolumes[i] * 100.0f), -1);
                    }
                }
            }
        }
    }

    g_musicFadeStarted = 0;
    diag_log("[warp] Music channels restored (volumes + fade_in re-enabled)");
}

/* ============================================================
 * Node proximity scanner
 * ============================================================ */

static void scanWarpNodes(void) {
    int app, profile, board, meshWorld, section3;
    int nodeCount, nodeDataArray;
    int ballDataArray, ball;
    float ballX, ballY, ballZ;
    int i;

    if (g_phase != PHASE_IDLE) return;
    if (getGameTime() < g_cooldownUntil) return;

    app = GetApp();
    if (!app) return;

    profile = *(int *)((char *)app + APP_PROFILE_PTR);
    if (!profile) return;

    board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
    if (!board) return;

    if (*((char *)board + BOARD_GOAL_REACHED) != 0) return;

    meshWorld = *(int *)((char *)board + BOARD_MESHWORLD_PTR);
    if (!is_valid_ptr(meshWorld)) return;

    section3 = *(int *)((char *)meshWorld + MW_SECTION3_PTR);
    if (!is_valid_ptr(section3)) return;

    nodeCount = *(int *)((char *)section3 + MW_NODELIST_COUNT);
    nodeDataArray = *(int *)((char *)section3 + MW_NODELIST_DATA);
    if (nodeCount <= 0 || !is_valid_ptr(nodeDataArray)) return;
    if (nodeCount > 256) nodeCount = 256;

    ballDataArray = *(int *)((char *)board + BALL_LIST_DATA_OFFSET);
    if (!is_valid_ptr(ballDataArray)) return;
    ball = *(int *)((char *)ballDataArray);
    if (!is_valid_ball(ball)) return;

    if (*((char *)ball + BALL_DEATH_PENDING) != 0) return;

    ballX = *(float *)((char *)ball + BALL_POS_X);
    ballY = *(float *)((char *)ball + BALL_POS_Y);
    ballZ = *(float *)((char *)ball + BALL_POS_Z);

    for (i = 0; i < nodeCount; i++) {
        int entry = *(int *)((char *)nodeDataArray + i * 4);
        const char *name;

        if (!is_valid_ptr(entry)) continue;

        name = *(const char **)((char *)entry);
        if (!name) continue;

        if (isWarpNode(name)) {
            float nodeX = *(float *)((char *)entry + NODE_X_OFFSET);
            float nodeY = *(float *)((char *)entry + NODE_Y_OFFSET);
            float nodeZ = *(float *)((char *)entry + NODE_Z_OFFSET);

            float dx = ballX - nodeX;
            float dy = ballY - nodeY;
            float dz = ballZ - nodeZ;
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq < WARP_TRIGGER_DIST_SQ) {
                char levelName[128];
                if (parseWarpLevel(name, levelName, sizeof(levelName))) {
                    int raceIndex = findRaceIndex(levelName);
                    diag_logf("[WARP] Node \"%s\" at (%.1f, %.1f, %.1f) — ball at (%.1f, %.1f, %.1f), distSq=%.1f",
                              name, nodeX, nodeY, nodeZ, ballX, ballY, ballZ, distSq);
                    if (raceIndex > 0) {
                        g_warpLevelIndex = raceIndex - 1;
                        g_phase = PHASE_RUMBLE;
                        g_rumbleInit = 0;
                        g_colorSaved = 0;
                        g_pauseBlocked = 0;
                        g_warpBall = ball;
                        {
                            DWORD now = getGameTime();
                            g_phaseStartTime = now;
                            g_warpStartTime = now;
                        }
                        g_whiteAlpha = 0.0f;
                        g_musicFadeStarted = 0;
                        diag_logf("[WARP] Triggered! Starting effect sequence, level=%d", raceIndex);
                    } else {
                        diag_logf("[WARP] Unknown level name \"%s\" in node \"%s\"", levelName, name);
                    }
                }
                return;
            }
        }
    }
}

/* ============================================================
 * App_FrameUpdate epilogue hook
 * ============================================================ */

#define APP_FRAME_UPDATE_EPILOGUE 0x0046C1F1
static unsigned char *g_frameUpdateDetour = NULL;

/* ============================================================
 * Warp state machine — called every frame from FrameUpdateHandler
 * ============================================================ */

static void updateWarpStateMachine(void) {
    int app;
    int board = 0;
    int ball;
    DWORD now;
    DWORD elapsed;

    if (g_phase == PHASE_IDLE) return;

    now = getGameTime();

    app = GetApp();
    if (!app) {
        diag_log("[warp] App null during warp, aborting");
        g_freezeTimer = 0;
        unblock_pause();
        g_phase = PHASE_IDLE;
        return;
    }

    ball = g_warpBall;
    if (!is_valid_ball(ball)) {
        if (ball) {
            diag_logf("[warp] g_warpBall 0x%08X invalid — continuing without ball", ball);
        }
        ball = 0;
    }

    {
        int profile = *(int *)((char *)app + APP_PROFILE_PTR);
        if (profile) {
            board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
        }
    }
    if (!board) board = 0;

    /* Write white flash alpha to board+0x3624 during fade phases.
     * The fade alpha is NOT the race timer — the race timer is frozen
     * separately via the code caves. We only write to the fade alpha
     * for the visual white flash effect. */
    if (board) {
        if (g_phase == PHASE_FLASH || g_phase == PHASE_FADE || g_phase == PHASE_REVEAL) {
            *(float *)((char *)board + SCENE_FADE_ALPHA) = g_whiteAlpha;
        }
    }

    /* Abort warp if tournament timer runs out during RUMBLE or early FLASH.
     * Board_UpdateRaceState sets App+0x5D6 (obj+0x0A "finished" flag) when
     * the timer goes below 0 — creating a RaceResultPopup and playing
     * "Game Over". When this happens, we must cancel the warp and let the
     * game's natural timeout sequence play out.
     * We check this before the switch so it applies to all phases before
     * the timer is frozen (which happens at FLASH peak via g_freezeTimer). */
    if (g_phase == PHASE_RUMBLE || (g_phase == PHASE_FLASH && g_whiteAlpha < 0.99f)) {
        if (app && !IsBadReadPtr((void*)(app + APP_TIMER_FINISHED), 1)) {
            char finished = *((char *)app + APP_TIMER_FINISHED);
            if (finished) {
                diag_logf("[warp] ABORT: Tournament timer expired (App+0x5D6=1) during %s — letting game handle timeout",
                          g_phase == PHASE_RUMBLE ? "RUMBLE" : "FLASH");

                /* Restore ball physics state but KEEP the blue color and alpha
                 * fade — they persist as a visual effect during the game's
                 * natural timeout sequence (RaceResultPopup + Game Over). */
                if (ball) {
                    *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 0;
                    *(char *)((char *)ball + BALL_RENDER_JITTER) = 0;
                    *(char *)((char *)ball + BALL_IN_TAR) = 0;
                    /* Ball alpha and color multiplier are intentionally NOT
                     * restored — the partially-faded, electric-blue ball
                     * remains visible during the timeout popup. */
                }

                /* Restore music, unfreeze timer, unblock pause */
                restoreMusicFade();
                g_freezeTimer = 0;
                unblock_pause();

                /* Clear fade alpha so it doesn't interfere with the game's popup */
                if (board) {
                    *(float *)((char *)board + SCENE_FADE_ALPHA) = 0.0f;
                }

                g_whiteAlpha = 0.0f;
                g_phase = PHASE_IDLE;
                g_cooldownUntil = getGameTime() + WARP_COOLDOWN_MS;
                g_warpBall = 0;
                diag_log("[warp] Warp aborted — returning to IDLE with game timeout intact");
                return;
            }
        }
    }

    switch (g_phase) {
    case PHASE_RUMBLE: {
        elapsed = now - g_phaseStartTime;

        if (!g_rumbleInit && ball) {
            g_rumbleInit = 1;
            block_pause();
            diag_log("[warp] RUMBLE start: pause blocked");

            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 1000;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 1;
            startMusicFade();

            /* Save the ball's runtime color multiplier (ball+0x2AC/0x2B0/0x2B4).
             * These are the SAME fields Ball_Render reads every frame and passes
             * to Graphics_SetColorMultiplier — the magnifying glass heat system
             * subtracts ball+0xC50 from G and B here. Writing to these fields
             * immediately changes the rendered ball color.
             * (OLD CODE used ball+0x20C/0x210/0x214 which are material diffuse
             *  values — NOT read by the color multiplier path, so color changes
             *  were invisible.) */
            if (!IsBadReadPtr((void*)(ball + BALL_COLOR_R), 12)) {
                g_origBallR = *(float*)(ball + BALL_COLOR_R);
                g_origBallG = *(float*)(ball + BALL_COLOR_G);
                g_origBallB = *(float*)(ball + BALL_COLOR_B);
                g_colorSaved = 1;
            }
            diag_logf("[warp] PHASE_RUMBLE start: steering disabled (ball+0x808=1000), jitter on");
        }

        /* Fade ball color multiplier from original to electric blue (#03fff2)
         * over RUMBLE phase. Writes to ball+0x2AC/0x2B0/0x2B4 — the exact
         * fields Ball_Render reads and passes to Graphics_SetColorMultiplier
         * every frame. This is the same mechanism the magnifying glass heat
         * system uses (it subtracts ball+0xC50 from G and B at these offsets). */
        if (g_colorSaved && ball && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
            float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
            if (t > 1.0f) t = 1.0f;
            float r = g_origBallR + (TARGET_BLUE_R - g_origBallR) * t;
            float g = g_origBallG + (TARGET_BLUE_G - g_origBallG) * t;
            float b = g_origBallB + (TARGET_BLUE_B - g_origBallB) * t;
            *(float*)(ball + BALL_COLOR_R) = r;
            *(float*)(ball + BALL_COLOR_G) = g;
            *(float*)(ball + BALL_COLOR_B) = b;
        }

        /* Fade ball alpha from 1.0 to 0.5 over RUMBLE */
        if (ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                float t = (float)elapsed / (float)RUMBLE_DURATION_MS;
                if (t > 1.0f) t = 1.0f;
                *(float *)((char *)ball + BALL_ALPHA) = 1.0f - (0.5f * t);
            }
        }

        updateMusicFade();

        if (elapsed >= RUMBLE_DURATION_MS) {
            g_phase = PHASE_FLASH;
            g_phaseStartTime = now;
            diag_logf("[warp] -> PHASE_FLASH (rumble ran %lums)", (unsigned long)elapsed);
        }
        break;
    }

    case PHASE_FLASH: {
        elapsed = now - g_phaseStartTime;

        if (elapsed < FLASH_PEAK_MS) {
            g_whiteAlpha = (float)elapsed / (float)FLASH_PEAK_MS;
        } else {
            DWORD remaining = FLASH_DURATION_MS - elapsed;
            g_whiteAlpha = (float)remaining / (float)(FLASH_DURATION_MS - FLASH_PEAK_MS);
            if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;
        }

        if (g_whiteAlpha >= 0.99f && ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
                /* Freeze ball physics by setting in_tar flag (ball+0x2CC=1).
                 * This stops the ball's physics simulation — same effect as
                 * when the ball enters a tar pit in-game. */
                *((char *)ball + BALL_IN_TAR) = 1;
                /* Freeze the race timer at the moment the ball vanishes.
                 * This stops the per-player timer (App+PID*0xA0+0x5E8)
                 * from decrementing — same passive mechanism as N:GOAL,
                 * but without any N:GOAL side effects. */
                if (!g_freezeTimer) {
                    g_freezeTimer = 1;
                    diag_log("[warp] Ball vanished — timer frozen + in_tar set (physics stopped)");
                }
            }
        }

        updateMusicFade();

        if (elapsed >= FLASH_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_HOLD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_HOLD");
        }
        break;
    }

    case PHASE_HOLD: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 0.0f;

        if (ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
        }

        updateMusicFade();

        if (elapsed >= HOLD_DURATION_MS) {
            g_phase = PHASE_FADE;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_FADE");
        }
        break;
    }

    case PHASE_FADE: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = (float)elapsed / (float)FADE_DURATION_MS;
        if (g_whiteAlpha > 1.0f) g_whiteAlpha = 1.0f;

        if (ball) {
            int respawning = *((unsigned char *)((char *)ball + 0x2F9));
            if (!respawning) {
                *(float *)((char *)ball + BALL_ALPHA) = 0.0f;
            }
        }

        updateMusicFade();

        if (elapsed >= FADE_DURATION_MS) {
            g_whiteAlpha = 1.0f;
            g_phase = PHASE_LOAD;
            g_phaseStartTime = now;
            diag_log("[warp] -> PHASE_LOAD: loading level");
        }
        break;
    }

    case PHASE_LOAD: {
        int levelIdx = g_warpLevelIndex;
        g_warpLevelIndex = -1;

        /* Clear ball freeze + jitter flags before loading */
        if (ball) {
            *(int *)((char *)ball + BALL_IMPACT_FREEZE) = 0;
            *(char *)((char *)ball + BALL_RENDER_JITTER) = 0;
            *(float *)((char *)ball + BALL_ALPHA) = 1.0f;
            /* Clear in_tar to unfreeze ball physics on the new level */
            *((char *)ball + BALL_IN_TAR) = 0;

            /* Restore original ball color multiplier (ball+0x2AC/0x2B0/0x2B4)
             * so the destination level shows the normal ball color, not blue. */
            if (g_colorSaved && !IsBadWritePtr((void*)(ball + BALL_COLOR_R), 12)) {
                *(float*)(ball + BALL_COLOR_R) = g_origBallR;
                *(float*)(ball + BALL_COLOR_G) = g_origBallG;
                *(float*)(ball + BALL_COLOR_B) = g_origBallB;
                g_colorSaved = 0;
            }
        }

        /* Unfreeze timer before loading new level — the new level
         * gets a fresh timer from Tournament_AdvanceRace. */
        g_freezeTimer = 0;
        diag_log("[warp] Timer unfrozen before level load (g_freezeTimer=0)");

        if (levelIdx >= 0 && levelIdx <= 14) {
            void *func = (void *)APP_START_PRACTICE_RACE;
            int appVal = app;
            int idx = levelIdx;
            char savedDifficulty = *((char *)app + 0x23C);
            int oldProfile = *(int *)((char *)app + APP_PROFILE_PTR);

            char wasInTournament = 0;
            int savedScores[16];
            int savedTimes[16];
            int hasTournamentData = 0;
            int savedTimeRemaining = 0;
            float savedPlayerScore = 0.0f;
            int isSameLevel = 0;

            if (oldProfile) {
                char isPractice = *((char *)oldProfile + PROFILE_IS_PRACTICE);
                wasInTournament = (isPractice == 0) ? 1 : 0;

                /* Detect same-level warp: compare target with current race index.
                 * g_warpLevelIndex is 0-based (0-14), but profile+0x08 is 1-based (1-15).
                 * The game stores raceIdx as 1-based and passes raceIdx-1 to
                 * App_StartPracticeRace (verified from caller at 0x4315BD). */
                {
                    int currentRaceIdx = *(int *)((char *)oldProfile + 0x08);
                    if (levelIdx == currentRaceIdx - 1) {
                        isSameLevel = 1;
                        diag_logf("[warp] Same-level warp detected (levelIdx=%d, profile raceIdx=%d)",
                                  levelIdx, currentRaceIdx);
                    }
                }

                if (wasInTournament) {
                    int raceIdx = *(int *)((char *)oldProfile + 0x08);
                    hasTournamentData = 1;
                    memcpy(savedScores, (void *)((char *)oldProfile + PROFILE_SCORE_ARRAY), sizeof(savedScores));
                    memcpy(savedTimes, (void *)((char *)oldProfile + PROFILE_TIME_ARRAY), sizeof(savedTimes));
                    if (raceIdx >= 0 && raceIdx < 16) {
                        /* App+0x5E4 = SCORE (float), App+0x5E8 = TIME REMAINING (int) */
                        savedScores[raceIdx] = (int)*(float *)((char *)app + 0x5E4);
                        diag_logf("[warp] Saved race score %d (from float %f) at index %d", savedScores[raceIdx], *(float *)((char *)app + 0x5E4), raceIdx);
                    }
                    if (isSameLevel) {
                        /* Same-level tournament: use the carry-over time from the previous
                         * race (stored when this race started). This gives a "fresh restart"
                         * of the level — same starting time as when you first entered it. */
                        if (raceIdx >= 0 && raceIdx < 16) {
                            savedTimeRemaining = *(int *)((char *)oldProfile + PROFILE_TIME_ARRAY + raceIdx * 4);
                            diag_logf("[warp] Same-level: saved carry-over timer %d from profile+TIME_ARRAY+raceIdx*4",
                                      savedTimeRemaining);
                        }
                    } else {
                        /* Different-level tournament: carry over current remaining timer. */
                        savedTimeRemaining = *(int *)((char *)app + 0x5E8);
                        diag_logf("[warp] Saved time remaining (carry-over): %d", savedTimeRemaining);
                    }
                    savedPlayerScore = *(float *)((char *)app + 0x5E4);
                    diag_logf("[warp] Saved player score: %f", savedPlayerScore);
                } else if (isSameLevel) {
                    /* Same-level time trial: save the current counted-up timer
                     * so we can resume counting from where we left off.
                     * In TT, App+0x5E8 counts UP (INC path), starting from 0. */
                    savedTimeRemaining = *(int *)((char *)app + 0x5E8);
                    diag_logf("[warp] Same-level TT: saved current timer %d for resume", savedTimeRemaining);
                }
            }

            diag_logf("[warp] App_StartPracticeRace(app=0x%08X, level=%d, difficulty=%d, tourney=%d)",
                       appVal, idx, (int)savedDifficulty, (int)wasInTournament);

            __asm__ volatile (
                "push %[idx]\n\t"
                "movl %[appVal], %%ecx\n\t"
                "call *%[func]\n\t"
                : /* no outputs */
                : [func] "r" (func),
                  [appVal] "r" (appVal),
                  [idx] "r" (idx)
                : "eax", "edx", "ecx",
                  "st", "st(1)", "st(2)", "st(3)",
                  "st(4)", "st(5)", "st(6)", "st(7)", "memory"
            );

            *((char *)app + 0x23C) = savedDifficulty;

            if (wasInTournament) {
                {
                    int bttRec = *(int *)((char *)app + APP_BTT_RECORDING);
                    int bttPlay = *(int *)((char *)app + APP_BTT_PLAYBACK);
                    if (bttRec) {
                        *(int *)((char *)app + APP_BTT_RECORDING) = 0;
                        diag_log("[warp] Cleared BTT recording pointer (tournament mode)");
                    }
                    if (bttPlay) {
                        *(int *)((char *)app + APP_BTT_PLAYBACK) = 0;
                        diag_log("[warp] Cleared BTT playback pointer (tournament mode)");
                    }
                }

                {
                    int newProfile = *(int *)((char *)app + APP_PROFILE_PTR);
                    if (newProfile) {
                        *((char *)newProfile + PROFILE_IS_PRACTICE) = 0;
                        if (hasTournamentData) {
                            memcpy((void *)((char *)newProfile + PROFILE_SCORE_ARRAY), savedScores, sizeof(savedScores));
                            memcpy((void *)((char *)newProfile + PROFILE_TIME_ARRAY), savedTimes, sizeof(savedTimes));
                        }
                        diag_log("[warp] Tournament mode restored: practice=0, scores copied to new profile");

                        {
                            /* The game's Tournament_AdvanceRace sets App+0x5E8 = board+0x2998
                             * + difficulty_bonus (1000 normal, 500 easy) for tournament mode.
                             * But we forced practice mode (profile+0x11=1) so it set timer=0.
                             * We need to compute: carryOver + newLevelBaseTime + difficultyBonus.
                             * board+0x2998 is the <TIME> value from racedata.xml (in seconds).
                             * The game multiplies by 100 (ticks per second) — but actually
                             * racedata.xml TIME is already in ticks (e.g. 60 = 60 seconds).
                             * The +1000/+500 is a per-level time bonus added on top. */
                            int newProfile2 = *(int *)((char *)app + APP_PROFILE_PTR);
                            int newBoard2 = 0;
                            int levelBaseTime = 0;
                            int difficultyBonus = 0;
                            int finalTimer = savedTimeRemaining;

                            if (newProfile2) {
                                newBoard2 = *(int *)((char *)newProfile2 + PROFILE_BOARD_PTR);
                            }
                            if (newBoard2 && !IsBadReadPtr((void *)(newBoard2 + 0x2998), 4)) {
                                levelBaseTime = *(int *)((char *)newBoard2 + 0x2998);
                            }

                            /* Difficulty bonus: 0=normal(+1000), 1=easy(+500).
                             * savedDifficulty was saved before App_StartPracticeRace
                             * which sets App+0x23C=1. We restored savedDifficulty
                             * already, so read the current value. */
                            {
                                int diff = (int)*((unsigned char *)((char *)app + 0x23C));
                                if (diff == 0) difficultyBonus = 1000;
                                else if (diff == 1) difficultyBonus = 500;
                            }

                            finalTimer = savedTimeRemaining + levelBaseTime + difficultyBonus;
                            *(int *)((char *)app + 0x5E8) = finalTimer;
                            diag_logf("[warp] Timer set: carryOver=%d + levelBase=%d + diffBonus=%d = %d ticks",
                                      savedTimeRemaining, levelBaseTime, difficultyBonus, finalTimer);
                        }
                        *(float *)((char *)app + 0x5E4) = savedPlayerScore;
                        diag_logf("[warp] Player score restored: %f", savedPlayerScore);
                    }
                }
            } else if (isSameLevel && savedTimeRemaining > 0) {
                /* Same-level time trial: resume timer from where we left off.
                 * App_StartPracticeRace starts TT timer at 0; we overwrite with
                 * the saved counted-up value so the timer continues from the
                 * point of the warp. */
                *(int *)((char *)app + 0x5E8) = savedTimeRemaining;
                diag_logf("[warp] Same-level TT: timer resumed at %d", savedTimeRemaining);
            }

            diag_log("[warp] Level loaded OK");
        } else {
            diag_logf("[warp] Invalid level index %d, aborting", levelIdx);
        }

        /* Write white alpha to the NEW board to prevent 1-frame visual gap */
        {
            int newProfile = *(int *)((char *)app + APP_PROFILE_PTR);
            if (newProfile) {
                int newBoard = *(int *)((char *)newProfile + PROFILE_BOARD_PTR);
                if (newBoard) {
                    *(float *)((char *)newBoard + SCENE_FADE_ALPHA) = 1.0f;
                }
            }
        }

        /* Restore music channel volumes before entering REVEAL.
         * This undoes the fade-out from startMusicFade/updateMusicFade:
         * restores saved volumes, re-enables fade_in, and pushes the
         * volume to BASS so music is audible on the new level. */
        restoreMusicFade();

        g_warpBall = 0;
        g_whiteAlpha = 1.0f;
        g_phase = PHASE_REVEAL;
        g_phaseStartTime = now;
        diag_log("[warp] -> PHASE_REVEAL: fading from white");
        break;
    }

    case PHASE_REVEAL: {
        elapsed = now - g_phaseStartTime;
        g_whiteAlpha = 1.0f - (float)elapsed / (float)REVEAL_DURATION_MS;
        if (g_whiteAlpha < 0.0f) g_whiteAlpha = 0.0f;

        if (elapsed >= REVEAL_DURATION_MS) {
            g_whiteAlpha = 0.0f;
            g_phase = PHASE_IDLE;
            g_cooldownUntil = getGameTime() + WARP_COOLDOWN_MS;
            unblock_pause();
            diag_log("[warp] -> PHASE_IDLE: warp complete (2s cooldown)");
        }
        break;
    }

    default:
        g_freezeTimer = 0;
        unblock_pause();
        g_phase = PHASE_IDLE;
        break;
    }
}

/* ============================================================
 * FrameUpdateHandler — called from App_FrameUpdate epilogue detour
 * ============================================================ */

static void FrameUpdateHandler(void) {
    {
        int app = GetApp();
        if (app) {
            int profile = *(int *)((char *)app + APP_PROFILE_PTR);
            if (profile) {
                int board = *(int *)((char *)profile + PROFILE_BOARD_PTR);
                updateGameClock(board);
            } else {
                updateGameClock(0);
            }
        } else {
            updateGameClock(0);
        }
    }

    if (g_phase == PHASE_IDLE) {
        scanWarpNodes();
    }
    updateWarpStateMachine();
}

/* ============================================================
 * Install hooks
 * ============================================================ */

static void InstallFrameUpdateHook(void) {
    unsigned char *epiAddr = (unsigned char *)APP_FRAME_UPDATE_EPILOGUE;
    DWORD oldProtect;

    if (epiAddr[0] != 0x5E || epiAddr[1] != 0x83 || epiAddr[2] != 0xC4 ||
        epiAddr[3] != 0x08 || epiAddr[4] != 0xC3) {
        diag_log("[FATAL] FrameUpdate epilogue signature mismatch!");
        return;
    }
    diag_log("[InstallHooks] FrameUpdate epilogue signature OK");

    g_frameUpdateDetour = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_frameUpdateDetour) return;

    {
        unsigned char *p = g_frameUpdateDetour;
        *p++ = 0x56;  /* PUSH ESI */
        *p++ = 0xB8; *(DWORD *)p = (DWORD)&FrameUpdateHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */
        *p++ = 0x5E;  /* POP ESI */
        *p++ = 0x5E;  /* original: POP ESI */
        *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;  /* ADD ESP, 0x8 */
        *p++ = 0xC3;  /* RET */
    }

    VirtualProtect(epiAddr, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    epiAddr[0] = 0xE9;
    *(DWORD *)(epiAddr + 1) = (DWORD)g_frameUpdateDetour - (DWORD)(epiAddr + 5);
    VirtualProtect(epiAddr, 5, oldProtect, &oldProtect);

    diag_log("[InstallHooks] FrameUpdate epilogue hook installed");
}

/* ============================================================
 * Init thread
 * ============================================================ */

static DWORD WINAPI InitThread(LPVOID param) {
    (void)param;
    Sleep(2000);

    InstallFrameUpdateHook();
    install_timer_caves();

    diag_log("[warp mod v8.5] Hooks + timer caves installed. Scanning MeshWorld nodes for WARP(Name) entries.");
    return 0;
}

/* ============================================================
 * DllMain
 * ============================================================ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved) {
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        {
            char mod_path[MAX_PATH];
            if (GetModuleFileNameA(hInst, mod_path, MAX_PATH)) {
                char *p = strrchr(mod_path, '\\');
                if (p) {
                    strcpy(p + 1, "warp_log.txt");
                    strncpy(g_logPath, mod_path, MAX_PATH - 1);
                }
            }
        }

        diag_log("=== LEVEL WARP MOD v8.5 LOADED ===");
        diag_log("v8.5: Abort warp if tournament timer expires during RUMBLE/early FLASH. Checks App+0x5D6 (finished flag) — if set, restores ball state and lets game's timeout (RaceResultPopup + Game Over music) play out naturally.");

        load_real_bass();
        diag_logf("bass_real.dll handle: 0x%08X", (unsigned)g_hRealBass);

        CreateThread(NULL, 0, InitThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        /* Restore original code on unload */
        g_freezeTimer = 0;
        restore_timer_caves();
        unblock_pause();
        break;
    }
    return TRUE;
}
