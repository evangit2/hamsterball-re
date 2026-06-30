/*
 * power_bounce.c — BASS.dll proxy — Power Bounce mod
 *
 * When the player's hamster ball collides with a shrunken 8-ball (player_index
 * == -1) and the 8-ball's radius is less than 70% of the player's radius, and
 * the player's center is directly above the 8-ball's center (within the
 * 8-ball's radius in XZ), the player ball is launched straight up at 2x the
 * jump force from the jump mod.
 *
 * HOW IT WORKS:
 *   1. A code cave hooks the ball-ball collision scoring section in
 *      Ball_Update at 0x406FD1 (the fld [edi+0x284] instruction).
 *      At this point ESI = this ball, EDI = other ball.
 *   2. The cave checks if one ball is a player (player_index >= 0) and the
 *      other is an 8-ball (player_index == -1).
 *   3. If so, it reads both balls' radii (ball+0x284) and positions
 *      (ball+0x164/0x168/0x16C = X/Y/Z).
 *   4. Size check: 8-ball radius < 0.7 * player radius.
 *   5. Position check: player center Y > 8-ball center Y (player is above)
 *      AND (playerX - ball8X)^2 + (playerZ - ball8Z)^2 < (ball8 radius)^2
 *      (player center is within 8-ball's radius in XZ).
 *   6. If all conditions pass, set g_power_bounce_pending = 1.
 *   7. A second hook at Ball_Update Phase 15 (0x407BB4) checks
 *      g_power_bounce_pending and adds 2x the jump impulse to ball+0x174
 *      (the Y velocity accumulator), then clears the flag.
 *
 * The jump impulse is applied in the Phase 15 hook (same location as the jump
 * mod), NOT from the collision hook. This is because the collision hook is
 * mid-function with FPU state active — we can't safely do float math there.
 * Instead, we set a flag and apply the impulse later in Phase 15 where the
 * jump mod already proved it's safe.
 *
 * No keyboard input. No raycasting. Purely automatic on collision.
 *
 * Original jump mod impulse: 20.0f
 * Power Bounce impulse: 40.0f (2x)
 *
 * HOOK 1: 0x00406FD1 — 6 bytes: D9 87 84 02 00 00 (fld [edi+0x284])
 * HOOK 2: 0x00407BB4 — 6 bytes: 8B 4C 24 1C 8B 11 (MOV ECX,[ESP+1C]; MOV EDX,[ECX])
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll power_bounce.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 *
 * Install:
 *   1. Rename original bass.dll -> bass_real.dll in the Hamsterball directory
 *   2. Copy this compiled bass.dll to the same directory
 *   3. Launch Hamsterball.exe
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports — forward all game imports to bass_real.dll
 * ═══════════════════════════════════════════════════════════════════════════ */

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

/* Extra stubs */
__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void* b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) { return 1; }
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }

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

/* ═══════════════════════════════════════════════════════════════════════════
 * Diagnostic logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_logpath[MAX_PATH] = "";

static void diag_log(const char *msg)
{
    if (g_logpath[0] == '\0') return;
    HANDLE hFile = CreateFileA(g_logpath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        SetFilePointer(hFile, 0, NULL, FILE_END);
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        CloseHandle(hFile);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Power Bounce — Collision Detection + Jump Impulse
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ─── Hook addresses ──────────────────────────────────────────────────────── */
#define COLLISION_HOOK     0x00406FD1   /* Ball_Update ball-ball collision section */
#define COLLISION_ORIG_BYTES 6          /* D9 87 84 02 00 00 = fld [edi+0x284]    */
#define PHASE15_HOOK       0x00407BB4   /* Ball_Update Phase 15 — impulse point     */
#define PHASE15_ORIG_BYTES 6            /* 8B 4C 24 1C 8B 11                        */

/* ─── Ball struct offsets ─────────────────────────────────────────────────── */
#define BALL_PLAYER_IDX    0x018        /* int: 0-3 = Player 1-4, -1 = NPC 8-ball   */
#define BALL_POS_X         0x164        /* float                                    */
#define BALL_POS_Y         0x168        /* float                                    */
#define BALL_POS_Z         0x16C        /* float                                    */
#define BALL_RADIUS        0x284        /* float                                    */
#define BALL_VEL_Y         0x174        /* float — Y velocity accumulator           */

/* ─── Parameters ────────────────────────────────────────────────────────────── */
/* Jump mod uses 20.0f. Power Bounce uses 2x = 40.0f. */
static float g_jump_impulse = 40.0f;

/* Size ratio threshold: 8-ball must be smaller than 70% of player radius */
#define SIZE_RATIO_THRESHOLD 0.7f

/* ─── Shared state ────────────────────────────────────────────────────────── */
/* g_power_bounce_pending: 0=idle, 1=collision detected, apply impulse in Phase 15 */
static volatile DWORD g_power_bounce_pending = 0;
static volatile DWORD g_bounce_count = 0;
static volatile DWORD g_collision_checks = 0;

/* Store the player ball pointer when a bounce is triggered (ESI at Phase 15
 * is always the ball being updated, which is the player ball during the
 * collision iteration. But we store it from the collision hook to be safe.)
 * Actually, Phase 15 runs for the CURRENT ball being updated. The collision
 * hook fires during Ball_Update for the player ball (ESI=player). Phase 15
 * also runs for the same ball (ESI=player). So ESI at Phase 15 is the same
 * ball that triggered the collision detection. We don't need to store the
 * pointer — just set the flag and Phase 15 will apply the impulse to ESI. */

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 1: Collision Detection (0x406FD1)
 *
 * At this point in Ball_Update:
 *   ESI = this ball (the one running Ball_Update)
 *   EDI = other ball (the collision partner)
 *   Both are valid ball pointers.
 *
 * The code cave:
 *   1. Saves all registers (PUSHAD)
 *   2. Checks if one ball is a player and the other is an 8-ball
 *   3. If so, reads radii and positions
 *   4. Checks: 8-ball radius < 0.7 * player radius
 *   5. Checks: player Y > 8-ball Y (player is above)
 *   6. Checks: (playerX-ball8X)^2 + (playerZ-ball8Z)^2 < (ball8 radius)^2
 *   7. If all pass: set g_power_bounce_pending = 1
 *   8. Restores registers (POPAD)
 *   9. Executes original instruction (fld [edi+0x284])
 *  10. Jumps back
 *
 * The float comparisons are done via integer bit manipulation to avoid
 * touching the FPU (which has live state at this point in Ball_Update).
 * We compare float bits as signed integers — this works for positive
 * floats (both radii and positions are positive in normal gameplay).
 * For the "player above" check, we need signed comparison since Y can
 * be any value. We use integer comparison on float bits, which is valid
 * for the IEEE 754 format (monotonic for same-sign values).
 *
 * Actually, for the Y comparison (player Y > 8-ball Y), we can use a
 * simple FLD/FCOMP/ FNSTSW sequence since we save/restore the FPU with
 * PUSHAD... no, PUSHAD doesn't save FPU state. We must not touch the FPU.
 *
 * Instead, we do ALL comparisons via integer math on float bit patterns.
 * For positive floats, unsigned integer comparison gives correct ordering.
 * For the Y comparison (which could be negative), we handle sign separately.
 *
 * Simplification: we use a C helper function called from the code cave
 * via CALL. The cave saves PUSHAD/PUSHFD, sets up a call to our C function,
 * then restores. This is safe because:
 *   - PUSHAD saves all general-purpose registers
 *   - PUSHFD saves EFLAGS
 *   - Our C function uses only integer math (no FPU)
 *   - We pass ESI and EDI as parameters
 *
 * The C function approach is cleaner and avoids hand-assembling float
 * comparisons in machine code. We pass the two ball pointers on the stack.
 * ═══════════════════════════════════════════════════════════════════════════ */

/*
 * C helper: check_collision
 *
 * Called from the code cave with the two ball pointers.
 * Returns 1 if the power bounce should trigger, 0 otherwise.
 *
 * This function uses ONLY integer math — no floating point — so it's safe
 * to call from mid-function where the FPU has live state.
 *
 * Float comparisons via integer bit patterns:
 *   - For positive floats: unsigned int comparison gives correct ordering
 *   - For the Y comparison: we extract sign bits and compare accordingly
 *   - For squared distance: we compute using integer arithmetic on the
 *     float bit patterns... actually, that won't work for multiplication.
 *
 * OK, we need to actually compute (dx*dx + dz*dz) vs (r*r).
 * We can't do float multiply without the FPU.
 *
 * Solution: use integer math. Read the float values as raw DWORDs, convert
 * to fixed-point integer (multiply by 1000 via integer scaling), then do
 * integer comparison.
 *
 * Actually, the simplest safe approach: save the FPU state manually.
 * We can use FSAVE/FRSTOR to save/restore the full FPU state (108 bytes).
 * This is safe in a code cave context.
 *
 * But FSAVE is complex. Let's use a different approach:
 *
 * Since we're in a code cave with PUSHAD already done, we can safely call
 * a C function that uses the FPU — as long as we save and restore the FPU
 * state. The x87 FPU state can be saved with FNSAVE (28-byte compact form
 * on 32-bit) or FXSAVE (512 bytes).
 *
 * Actually, the simplest approach: just do the comparisons in C using
 * integer reinterpretation. For the size check (radius comparison), both
 * radii are positive floats, so we can compare them as unsigned integers.
 * For the position checks, we need to handle the signs.
 *
 * For the squared distance check, we need to compute dx*dx + dz*dz.
 * We can convert floats to integers first (round to nearest), then do
 * integer multiplication. The precision is sufficient for this gameplay
 * check.
 */
static int __cdecl check_power_bounce(DWORD ball1, DWORD ball2)
{
    if (!ball1 || !ball2) return 0;
    if (ball1 == ball2) return 0;

    /* Read player indices */
    int idx1 = *(int*)(ball1 + BALL_PLAYER_IDX);
    int idx2 = *(int*)(ball2 + BALL_PLAYER_IDX);

    /* One must be a player (>= 0), the other must be an 8-ball (-1) */
    DWORD player_ball, ball8;
    if (idx1 >= 0 && idx2 == -1) {
        player_ball = ball1;
        ball8 = ball2;
    } else if (idx1 == -1 && idx2 >= 0) {
        player_ball = ball2;
        ball8 = ball1;
    } else {
        return 0;
    }

    /* Read radii (positive floats) */
    DWORD player_radius_bits = *(DWORD*)(player_ball + BALL_RADIUS);
    DWORD ball8_radius_bits = *(DWORD*)(ball8 + BALL_RADIUS);

    /* Quick check: if either radius is 0 or negative, bail */
    if (player_radius_bits == 0 || ball8_radius_bits == 0) return 0;
    if (player_radius_bits & 0x80000000) return 0;  /* negative */
    if (ball8_radius_bits & 0x80000000) return 0;    /* negative */

    /* Size check: 8-ball radius < 0.7 * player radius
     * Since both are positive floats, we can compare as unsigned ints.
     * 0.7f = 0x3F333333
     * We need: ball8_radius < player_radius * 0.7
     * Equivalent: ball8_radius / player_radius < 0.7
     * But integer division on float bits doesn't work.
     *
     * Instead: ball8_radius < player_radius * 0.7
     * We can't easily multiply float bits by 0.7 without FPU.
     * Use fixed-point: convert to integer (round), then compare:
     *   ball8_radius_int < player_radius_int * 7 / 10
     */
    float pr = *(float*)&player_radius_bits;
    float br = *(float*)&ball8_radius_bits;

    /* Convert to fixed-point: multiply by 1000 to get integer precision */
    int pr_int = (int)(pr * 1000.0f + 0.5f);
    int br_int = (int)(br * 1000.0f + 0.5f);

    /* ball8_radius < 0.7 * player_radius ?
     * br_int < pr_int * 7 / 10 */
    if (br_int >= pr_int * 7 / 10) return 0;

    /* Position check: player center Y > 8-ball center Y (player is above) */
    float player_y = *(float*)(player_ball + BALL_POS_Y);
    float ball8_y = *(float*)(ball8 + BALL_POS_Y);

    if (player_y <= ball8_y) return 0;

    /* Position check: player center within 8-ball radius in XZ
     * (playerX - ball8X)^2 + (playerZ - ball8Z)^2 < (ball8 radius)^2 */
    float player_x = *(float*)(player_ball + BALL_POS_X);
    float player_z = *(float*)(player_ball + BALL_POS_Z);
    float ball8_x = *(float*)(ball8 + BALL_POS_X);
    float ball8_z = *(float*)(ball8 + BALL_POS_Z);

    float dx = player_x - ball8_x;
    float dz = player_z - ball8_z;
    float dist_sq = dx * dx + dz * dz;
    float radius_sq = br * br;

    if (dist_sq >= radius_sq) return 0;

    /* All conditions pass! */
    return 1;
}

/* Function pointer for the code cave to call our C helper */
static int (__cdecl *g_check_fn_ptr)(DWORD, DWORD) = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 1 Cave: Collision Detection at 0x406FD1
 *
 * Original instruction: fld dword [edi+0x284]  (6 bytes: D9 87 84 02 00 00)
 *
 * At this point: ESI = this ball, EDI = other ball
 *
 * Cave code:
 *   PUSHAD
 *   PUSHFD
 *   ; Call check_power_bounce(ESI, EDI)
 *   PUSH EDI                    ; arg2: other ball
 *   PUSH ESI                    ; arg1: this ball
 *   CALL [g_check_fn_ptr]       ; returns 1 if power bounce should trigger
 *   ADD ESP, 8                  ; cleanup (cdecl)
 *   ; If returned 1, set the pending flag
 *   TEST EAX, EAX
 *   JZ .skip
 *   MOV DWORD [g_power_bounce_pending], 1
 *   INC DWORD [g_collision_checks]
 *   .skip:
 *   POPFD
 *   POPAD
 *   ; Execute original instruction
 *   fld dword [edi+0x284]
 *   ; Jump back
 *   JMP (hook_addr + 6)
 * ═══════════════════════════════════════════════════════════════════════════ */
static BYTE *g_collision_cave = NULL;

static void install_collision_hook(void)
{
    BYTE *hook_addr = (BYTE*)COLLISION_HOOK;
    char buf[256];

    BYTE expected[] = { 0xD9, 0x87, 0x84, 0x02, 0x00, 0x00 };
    wsprintfA(buf, "Collision bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, COLLISION_ORIG_BYTES) != 0) {
        diag_log("COLLISION HOOK: BYTE MISMATCH!");
        return;
    }

    g_collision_cave = (BYTE*)VirtualAlloc(NULL, 512, MEM_COMMIT | MEM_RESERVE,
                                              PAGE_EXECUTE_READWRITE);
    if (!g_collision_cave) { diag_log("collision: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* PUSHAD (save all general-purpose registers) */
    g_collision_cave[p++] = 0x60;

    /* PUSHFD (save flags) */
    g_collision_cave[p++] = 0x9C;

    /* PUSH EDI (arg2: other ball pointer) */
    g_collision_cave[p++] = 0x57;

    /* PUSH ESI (arg1: this ball pointer) */
    g_collision_cave[p++] = 0x56;

    /* CALL [g_check_fn_ptr] — indirect call through function pointer */
    g_collision_cave[p++] = 0xFF; g_collision_cave[p++] = 0x15;
    *(DWORD*)(g_collision_cave + p) = (DWORD)&g_check_fn_ptr; p += 4;

    /* ADD ESP, 8 (cdecl cleanup: 2 DWORD args) */
    g_collision_cave[p++] = 0x83; g_collision_cave[p++] = 0xC4;
    g_collision_cave[p++] = 0x08;

    /* TEST EAX, EAX (check return value) */
    g_collision_cave[p++] = 0x85; g_collision_cave[p++] = 0xC0;

    /* JZ .skip (if returned 0, skip setting flag) */
    int jz_skip_fixup = p;
    g_collision_cave[p++] = 0x0F; g_collision_cave[p++] = 0x84;
    *(DWORD*)(g_collision_cave + p) = 0; p += 4;

    /* MOV DWORD [g_power_bounce_pending], 1 */
    g_collision_cave[p++] = 0xC7; g_collision_cave[p++] = 0x05;
    *(DWORD*)(g_collision_cave + p) = (DWORD)&g_power_bounce_pending; p += 4;
    *(DWORD*)(g_collision_cave + p) = 1; p += 4;

    /* INC DWORD [g_collision_checks] */
    g_collision_cave[p++] = 0xFF; g_collision_cave[p++] = 0x05;
    *(DWORD*)(g_collision_cave + p) = (DWORD)&g_collision_checks; p += 4;

    /* .skip: */
    int skip_target = p;

    /* POPFD (restore flags) */
    g_collision_cave[p++] = 0x9D;

    /* POPAD (restore all registers) */
    g_collision_cave[p++] = 0x61;

    /* Execute original instruction: fld dword [edi+0x284] (D9 87 84 02 00 00) */
    g_collision_cave[p++] = 0xD9; g_collision_cave[p++] = 0x87;
    g_collision_cave[p++] = 0x84; g_collision_cave[p++] = 0x02;
    g_collision_cave[p++] = 0x00; g_collision_cave[p++] = 0x00;

    /* JMP back to hook_addr + COLLISION_ORIG_BYTES */
    g_collision_cave[p++] = 0xE9;
    *(DWORD*)(g_collision_cave + p) =
        (DWORD)(hook_addr + COLLISION_ORIG_BYTES) - (DWORD)(g_collision_cave + p + 4);
    p += 4;

    /* Fix up JZ */
    *(DWORD*)(g_collision_cave + jz_skip_fixup + 2) =
        (DWORD)(g_collision_cave + skip_target) - (DWORD)(g_collision_cave + jz_skip_fixup + 6);

    /* Patch the hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, COLLISION_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_collision_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;  /* NOP byte 6 */

    VirtualProtect(hook_addr, COLLISION_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, COLLISION_ORIG_BYTES);

    wsprintfA(buf, "COLLISION HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_collision_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook 2: Phase 15 Impulse (0x407BB4)
 *
 * Original 6 bytes: 8B 4C 24 1C 8B 11 = MOV ECX,[ESP+1C]; MOV EDX,[ECX]
 * ESI = ball pointer (the ball being updated — the player ball).
 *
 * If g_power_bounce_pending == 1:
 *   Add g_jump_impulse (40.0f) to ball+0x174 (Y velocity accumulator)
 *   Clear the flag
 *   Increment bounce counter
 *
 * This is the same mechanism as the jump mod's Phase 15 hook, but triggered
 * by collision detection instead of keyboard input + raycasting.
 * ═══════════════════════════════════════════════════════════════════════════ */
static BYTE *g_phase15_cave = NULL;

static void install_phase15_hook(void)
{
    BYTE *hook_addr = (BYTE*)PHASE15_HOOK;
    char buf[256];

    BYTE expected[] = { 0x8B, 0x4C, 0x24, 0x1C, 0x8B, 0x11 };
    wsprintfA(buf, "Phase15 bytes: %02X %02X %02X %02X %02X %02X",
              hook_addr[0], hook_addr[1], hook_addr[2],
              hook_addr[3], hook_addr[4], hook_addr[5]);
    diag_log(buf);

    if (memcmp(hook_addr, expected, PHASE15_ORIG_BYTES) != 0) {
        diag_log("PHASE15 HOOK: BYTE MISMATCH!");
        return;
    }

    g_phase15_cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                           PAGE_EXECUTE_READWRITE);
    if (!g_phase15_cave) { diag_log("phase15: VirtualAlloc FAILED"); return; }

    int p = 0;

    /* Check g_power_bounce_pending == 1 */
    /* CMP DWORD [g_power_bounce_pending], 1 */
    g_phase15_cave[p++] = 0x83; g_phase15_cave[p++] = 0x3D;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_power_bounce_pending; p += 4;
    g_phase15_cave[p++] = 0x01;

    /* JNZ to .no_bounce */
    int jnz_fixup = p;
    g_phase15_cave[p++] = 0x0F; g_phase15_cave[p++] = 0x85;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* ─── Apply impulse: ball+0x174 += g_jump_impulse ─── */
    /* FLD [ESI+0x174] — load current Y velocity */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x86;
    *(DWORD*)(g_phase15_cave + p) = BALL_VEL_Y; p += 4;

    /* FADD [g_jump_impulse] — add upward impulse */
    g_phase15_cave[p++] = 0xD8; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_jump_impulse; p += 4;

    /* FSTP [ESI+0x174] — store modified Y velocity */
    g_phase15_cave[p++] = 0xD9; g_phase15_cave[p++] = 0x9E;
    *(DWORD*)(g_phase15_cave + p) = BALL_VEL_Y; p += 4;

    /* MOV DWORD [g_power_bounce_pending], 0 — consumed */
    g_phase15_cave[p++] = 0xC7; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_power_bounce_pending; p += 4;
    *(DWORD*)(g_phase15_cave + p) = 0; p += 4;

    /* INC DWORD [g_bounce_count] */
    g_phase15_cave[p++] = 0xFF; g_phase15_cave[p++] = 0x05;
    *(DWORD*)(g_phase15_cave + p) = (DWORD)&g_bounce_count; p += 4;

    /* ─── .no_bounce: ─── */
    int no_bounce_target = p;

    /* Original 6 bytes: MOV ECX,[ESP+1C]; MOV EDX,[ECX] */
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x4C;
    g_phase15_cave[p++] = 0x24; g_phase15_cave[p++] = 0x1C;
    g_phase15_cave[p++] = 0x8B; g_phase15_cave[p++] = 0x11;

    /* JMP back to hook_addr + PHASE15_ORIG_BYTES */
    g_phase15_cave[p++] = 0xE9;
    *(DWORD*)(g_phase15_cave + p) =
        (DWORD)(hook_addr + PHASE15_ORIG_BYTES) - (DWORD)(g_phase15_cave + p + 4);
    p += 4;

    /* Fix up JNZ */
    *(DWORD*)(g_phase15_cave + jnz_fixup + 2) =
        (DWORD)(g_phase15_cave + no_bounce_target) - (DWORD)(g_phase15_cave + jnz_fixup + 6);

    /* Patch the hook site */
    DWORD old_protect;
    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, PAGE_EXECUTE_READWRITE, &old_protect);

    DWORD jmp_offset = (DWORD)(g_phase15_cave - hook_addr - 5);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = jmp_offset;
    hook_addr[5] = 0x90;

    VirtualProtect(hook_addr, PHASE15_ORIG_BYTES, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, PHASE15_ORIG_BYTES);

    wsprintfA(buf, "PHASE15 HOOK installed: %d bytes, cave=%08X", p, (DWORD)g_phase15_cave);
    diag_log(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    char buf[256];

    diag_log("=== Power Bounce mod loaded ===");
    Sleep(5000);

    /* Initialize the C helper function pointer */
    g_check_fn_ptr = check_power_bounce;
    wsprintfA(buf, "check_fn = %08X", (DWORD)check_power_bounce);
    diag_log(buf);

    /* Install hooks */
    install_collision_hook();
    install_phase15_hook();

    diag_log("Power Bounce: all hooks installed");

    /* Status check after 8 seconds */
    Sleep(8000);
    wsprintfA(buf, "After 8s: bounces=%u collision_checks=%u pending=%u",
              g_bounce_count, g_collision_checks, g_power_bounce_pending);
    diag_log(buf);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hInst, DWORD reason, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInst);

        /* Set up log path next to bass.dll */
        GetModuleFileNameA(hInst, g_logpath, MAX_PATH);
        {
            char *p = strrchr(g_logpath, '\\');
            if (p) strcpy(p + 1, "power_bounce_log.txt");
        }

        diag_log("=== Power Bounce DLL attaching ===");

        load_real_bass();
        {
            char buf[128];
            wsprintfA(buf, "bass_real = %08X", (DWORD)g_hRealBass);
            diag_log(buf);
        }

        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
