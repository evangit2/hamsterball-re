/*
 * custom_events — Custom E: collision events for Hamsterball
 *
 * v1: E:SOUND<file>name</file> — plays a custom .ogg/.wav from Sounds/ folder
 *     Behaves exactly like E:POPOUT (cooldown + dizzy immunity) but with a
 *     user-specified sound file instead of the hardcoded popout sound.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll custom_events.c -I../shared \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"
#include <math.h>

/* ============================================================
 * BASS stream playback (for custom .ogg/.wav files)
 * The game already called BASS_Init through our proxy, so the
 * BASS device is ready. We load BASS_StreamCreateFile and
 * BASS_ChannelPlay from bass_real.dll to play custom sounds.
 *
 * BASS 2.0: BASS_StreamCreateFile(BOOL, char*, DWORD, DWORD, DWORD) — 20 bytes
 * BASS 2.4: BASS_StreamCreateFile(BOOL, void*, QWORD, QWORD, DWORD) — 28 bytes
 * We detect version by checking for BASS_ChannelSetAttribute (2.4-only).
 * ============================================================ */

typedef DWORD (__stdcall *StreamCreateFile20_t)(BOOL, const char*, DWORD, DWORD, DWORD);
typedef DWORD (__stdcall *StreamCreateFile24_t)(BOOL, const char*, unsigned long long, unsigned long long, DWORD);
typedef int  (__stdcall *ChannelPlay_t)(DWORD, BOOL);
typedef int  (__stdcall *StreamPlay20_t)(DWORD, BOOL);  /* BASS 2.0: BASS_StreamPlay */

static StreamCreateFile20_t real_StreamCreate20 = NULL;
static StreamCreateFile24_t real_StreamCreate24 = NULL;
static ChannelPlay_t        real_ChannelPlay    = NULL;  /* BASS 2.4 */
static StreamPlay20_t       real_StreamPlay     = NULL;  /* BASS 2.0 */
static int g_bassIs24 = 0;

/* ============================================================
 * Game addresses
 * ============================================================ */

#define DISPATCH_COLLISION_EVENTS 0x0040C5D0
#define BALL_DIZZY_IMMUNITY       0x00402400
#define BALL_POPOUT_COOLDOWN       0x7CC    /* ball+0x7CC, same field E:POPOUT uses */
#define COLL_OBJ_NAME_OFFSET       0x864    /* collision entry: event name char* */

/* ============================================================
 * Sound cache — avoid reloading the same .ogg every collision
 * ============================================================ */

#define MAX_CUSTOM_SOUNDS 32
typedef struct {
    char  name[64];
    DWORD stream;   /* BASS stream handle */
} CachedSound;

static CachedSound g_soundCache[MAX_CUSTOM_SOUNDS];
static int g_soundCount = 0;

/* ============================================================
 * Hook globals (same 8-byte detour pattern as teleport mod)
 * ============================================================ */

static unsigned char g_trampoline[16];
static unsigned char *g_detourBuf = NULL;

static volatile void *g_savedBoard   = NULL;
static volatile int  *g_savedBall    = NULL;
static volatile int  *g_savedCollObj = NULL;

static volatile int g_hookFireCount   = 0;
static volatile int g_soundMatchCount = 0;

/* ============================================================
 * Logging
 * ============================================================ */

static char g_logPath[MAX_PATH] = "";

static void diag_log(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}

static void diag_logf(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    diag_log(buf);
}

/* ============================================================
 * Parse <file>...</file> tag from event name string
 *
 * Event format: E:SOUND<file>mysfx</file>
 * Returns: 1 on success, 0 on failure
 * ============================================================ */

static int parseFileTag(const char *eventName, char *outFile, int outSize) {
    const char *start, *end;
    int len;

    start = strstr(eventName, "<file>");
    if (!start) return 0;
    start += 6;  /* skip "<file>" */

    end = strstr(start, "</file>");
    if (!end) return 0;

    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return 0;

    strncpy(outFile, start, len);
    outFile[len] = 0;
    return 1;
}

/* ============================================================
 * Load or retrieve a cached BASS stream for a sound file
 *
 * Tries Sounds\<name>.ogg first, then Sounds\<name>.wav
 * ============================================================ */

static DWORD getOrLoadSound(const char *name) {
    int i;
    char path[MAX_PATH];

    /* Check cache */
    for (i = 0; i < g_soundCount; i++) {
        if (_stricmp(g_soundCache[i].name, name) == 0)
            return g_soundCache[i].stream;
    }

    if (g_soundCount >= MAX_CUSTOM_SOUNDS) {
        diag_log("[SOUND] Cache full, skipping load");
        return 0;
    }

    /* Try .ogg first */
    snprintf(path, sizeof(path), "Sounds\\%s.ogg", name);
    DWORD stream = 0;

    if (g_bassIs24 && real_StreamCreate24) {
        stream = real_StreamCreate24(FALSE, path, 0, 0, 0);
    } else if (real_StreamCreate20) {
        stream = real_StreamCreate20(FALSE, path, 0, 0, 0);
    } else {
        diag_log("[SOUND] BASS stream functions not available");
        return 0;
    }

    /* Try .wav if .ogg failed */
    if (!stream) {
        snprintf(path, sizeof(path), "Sounds\\%s.wav", name);
        if (g_bassIs24 && real_StreamCreate24)
            stream = real_StreamCreate24(FALSE, path, 0, 0, 0);
        else if (real_StreamCreate20)
            stream = real_StreamCreate20(FALSE, path, 0, 0, 0);
    }

    if (!stream) {
        diag_logf("[SOUND] Failed to load: %s (ogg and wav)", name);
        return 0;
    }

    /* Cache it */
    strncpy(g_soundCache[g_soundCount].name, name, 63);
    g_soundCache[g_soundCount].name[63] = 0;
    g_soundCache[g_soundCount].stream = stream;
    g_soundCount++;

    diag_logf("[SOUND] Loaded: %s -> stream 0x%08X", name, stream);
    return stream;
}

/* ============================================================
 * Call Ball_DizzyImmunity(ball, time)
 *
 * __thiscall: ECX=ball, stack: [time], RET 0x4
 * Address: 0x00402400
 * ============================================================ */

static void callBallDizzyImmunity(int *ball, int time) {
    void *func = (void *)BALL_DIZZY_IMMUNITY;
    int  ballVal = (int)ball;
    int  timeVal = time;

    __asm__ volatile(
        "push %[time]\n\t"
        "movl %[ballVal], %%ecx\n\t"
        "call *%[func]\n\t"
        :
        : [func]   "r"(func),
          [ballVal] "r"(ballVal),
          [time]   "r"(timeVal)
        : "eax", "ecx", "edx", "memory",
          "st", "st(1)", "st(2)", "st(3)",
          "st(4)", "st(5)", "st(6)", "st(7)"
    );
}

/* ============================================================
 * Collision handler — called from asm detour
 *
 * Checks for E:SOUND<file>name</file> event name.
 * If matched, plays the custom sound and applies POPOUT-like
 * cooldown + dizzy immunity.
 * ============================================================ */

static void SoundCollisionHandler(void) {
    void *board = (void *)g_savedBoard;
    int  *ball  = (int *)g_savedBall;
    int  *collObj = (int *)g_savedCollObj;

    g_hookFireCount++;

    if (!collObj || IsBadReadPtr(collObj, 8)) return;
    if (!collObj[1] || IsBadReadPtr((void*)collObj[1], 0x868)) return;

    char *eventName = *(char **)((char *)collObj[1] + COLL_OBJ_NAME_OFFSET);
    if (!eventName || IsBadReadPtr(eventName, 8)) return;

    /* Log first few events for debugging */
    if (g_hookFireCount <= 10) {
        diag_logf("[hook #%d] event=\"%s\"", g_hookFireCount, eventName);
    }

    /* Check for E:SOUND prefix */
    if (_strnicmp(eventName, "E:SOUND", 7) != 0) return;

    g_soundMatchCount++;

    /* Check cooldown (same as E:POPOUT: ball+0x7CC < 1) */
    if (*(int *)((char *)ball + BALL_POPOUT_COOLDOWN) >= 1) return;

    /* Parse <file>...</file> tag */
    {
        char fileName[64];
        if (!parseFileTag(eventName, fileName, sizeof(fileName))) {
            diag_logf("[SOUND] No <file> tag in: %s", eventName);
            return;
        }

        diag_logf("[SOUND MATCH #%d] file=\"%s\"", g_soundMatchCount, fileName);

        /* Load or get cached stream */
        DWORD stream = getOrLoadSound(fileName);
        if (stream) {
            /* Play with restart=TRUE (restart from beginning) */
            int playResult = 0;
            if (g_bassIs24 && real_ChannelPlay) {
                playResult = real_ChannelPlay(stream, TRUE);
            } else if (real_StreamPlay) {
                /* BASS 2.0: BASS_StreamPlay(handle, restart) */
                playResult = real_StreamPlay(stream, TRUE);
            }
            if (playResult) {
                diag_logf("[SOUND] Playing: %s", fileName);
            } else {
                diag_logf("[SOUND] Play failed for: %s (BASS error)", fileName);
            }
        } else {
            diag_logf("[SOUND] Could not load: %s", fileName);
        }

        /* Set cooldown to 50 frames (same as POPOUT) */
        *(int *)((char *)ball + BALL_POPOUT_COOLDOWN) = 0x32;

        /* Grant 100 frames of dizzy immunity (same as POPOUT) */
        callBallDizzyImmunity(ball, 100);
    }
}

/* ============================================================
 * Install 8-byte detour on DispatchCollisionEvents
 *
 * Original prologue (8 bytes):
 *   6A FF                 PUSH -1
 *   64 A1 00 00 00 00     MOV EAX, FS:[0]
 *
 * We patch with: JMP detour + 3 NOPs
 * Detour saves registers, calls C handler, restores,
 * executes original 8 bytes, JMPs back to original+8.
 * ============================================================ */

static void InstallHooks(void) {
    unsigned char *dispatchAddr = (unsigned char *)DISPATCH_COLLISION_EVENTS;
    DWORD oldProtect;

    /* Verify signature */
    if (dispatchAddr[0] != 0x6A || dispatchAddr[1] != 0xFF ||
        dispatchAddr[2] != 0x64 || dispatchAddr[3] != 0xA1) {
        diag_log("[FATAL] Signature mismatch at 0x40C5D0! Hook NOT installed.");
        return;
    }
    diag_log("[InstallHooks] DispatchCollisionEvents signature OK at 0x40C5D0");

    /* Save original 8 bytes into trampoline */
    memcpy(g_trampoline, dispatchAddr, 8);

    /* Build trampoline: original 8 bytes + JMP to dispatchAddr+8 */
    g_trampoline[8] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(g_trampoline + 9) = (DWORD)(dispatchAddr + 8) - (DWORD)(g_trampoline + 13);

    /* Make trampoline executable */
    VirtualProtect(g_trampoline, sizeof(g_trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);

    /* Allocate executable buffer for detour */
    g_detourBuf = (unsigned char *)VirtualAlloc(NULL, 256, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_detourBuf) {
        diag_log("[FATAL] VirtualAlloc failed for detour buffer");
        return;
    }

    /* Build detour code:
     *
     * Stack at entry: [return_addr] [ball] [collObj]
     * ECX = board
     *
     * 1. Save ECX, EDX
     * 2. Store board, ball, collObj to globals
     * 3. Call SoundCollisionHandler
     * 4. Restore registers
     * 5. Execute original 8 bytes (PUSH -1; MOV EAX,FS:[0])
     * 6. JMP to dispatchAddr+8
     */
    {
        unsigned char *p = g_detourBuf;

        /* Save registers */
        *p++ = 0x51;  /* PUSH ECX (board) */
        *p++ = 0x52;  /* PUSH EDX */

        /* MOV [g_savedBoard], ECX */
        *p++ = 0x89; *p++ = 0x0D;
        *(DWORD *)p = (DWORD)&g_savedBoard; p += 4;

        /* MOV EAX, [ESP+12] — ball (after 2 pushes, [ESP+12] = original [ESP+4]) */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x0C;
        /* MOV [g_savedBall], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedBall; p += 4;

        /* MOV EAX, [ESP+16] — collObj (after 2 pushes, [ESP+16] = original [ESP+8]) */
        *p++ = 0x8B; *p++ = 0x44; *p++ = 0x24; *p++ = 0x10;
        /* MOV [g_savedCollObj], EAX */
        *p++ = 0xA3;
        *(DWORD *)p = (DWORD)&g_savedCollObj; p += 4;

        /* Call SoundCollisionHandler */
        *p++ = 0xB8;  /* MOV EAX, SoundCollisionHandler */
        *(DWORD *)p = (DWORD)&SoundCollisionHandler; p += 4;
        *p++ = 0xFF; *p++ = 0xD0;  /* CALL EAX */

        /* Restore registers */
        *p++ = 0x5A;  /* POP EDX */
        *p++ = 0x59;  /* POP ECX */

        /* Execute original 8 bytes */
        *p++ = 0x6A; *p++ = 0xFF;  /* PUSH -1 */
        *p++ = 0x64; *p++ = 0xA1; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;  /* MOV EAX, FS:[0] */

        /* JMP to dispatchAddr+8 */
        *p++ = 0xE9;
        *(DWORD *)p = (DWORD)(dispatchAddr + 8) - (DWORD)(p + 4);
        p += 4;
    }

    /* Patch original function: JMP to detour + 3 NOPs */
    VirtualProtect(dispatchAddr, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    dispatchAddr[0] = 0xE9;  /* JMP rel32 */
    *(DWORD *)(dispatchAddr + 1) = (DWORD)g_detourBuf - (DWORD)(dispatchAddr + 5);
    dispatchAddr[5] = 0x90;
    dispatchAddr[6] = 0x90;
    dispatchAddr[7] = 0x90;
    VirtualProtect(dispatchAddr, 8, oldProtect, &oldProtect);

    diag_log("[InstallHooks] DispatchCollisionEvents hook installed OK.");
}

/* ============================================================
 * DllMain
 * ============================================================ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        /* Set up log path (next to bass.dll) */
        char modPath[MAX_PATH];
        if (GetModuleFileNameA(hModule, modPath, MAX_PATH)) {
            char *p = strrchr(modPath, '\\');
            if (p) {
                strcpy(p + 1, "custom_events_log.txt");
                strncpy(g_logPath, modPath, MAX_PATH - 1);
            }
        }

        diag_log("=== custom_events v1 starting ===");

        /* Load real BASS library */
        load_real_bass();

        /* Load BASS stream functions from bass_real.dll */
        if (g_hRealBass) {
            /* Detect BASS version: BASS_ChannelSetAttribute (singular) is 2.4-only */
            g_bassIs24 = (GetProcAddress(g_hRealBass, "BASS_ChannelSetAttribute") != NULL) ? 1 : 0;
            diag_logf("[init] BASS version: %s", g_bassIs24 ? "2.4" : "2.0");

            if (g_bassIs24) {
                real_StreamCreate24 = (StreamCreateFile24_t)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
                real_ChannelPlay = (ChannelPlay_t)GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
            } else {
                real_StreamCreate20 = (StreamCreateFile20_t)GetProcAddress(g_hRealBass, "BASS_StreamCreateFile");
                real_StreamPlay = (StreamPlay20_t)GetProcAddress(g_hRealBass, "BASS_StreamPlay");
            }

            if (real_StreamCreate20 || real_StreamCreate24) {
                diag_log("[init] BASS stream functions loaded OK");
            } else {
                diag_log("[init] WARNING: BASS_StreamCreateFile not found in bass_real.dll");
            }

            if (real_ChannelPlay || real_StreamPlay) {
                diag_log("[init] BASS playback function loaded OK");
            } else {
                diag_log("[init] WARNING: No BASS playback function found");
            }
        } else {
            diag_log("[init] WARNING: bass_real.dll not found — E:SOUND will be silent");
        }

        /* Install collision hook */
        InstallHooks();

        diag_log("=== custom_events v1 ready ===");
    }
    return TRUE;
}
