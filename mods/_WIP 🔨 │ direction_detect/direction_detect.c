/*
 * direction_detect.c — BASS.dll proxy that detects the hamster ball's facing
 * direction and displays it as an on-screen compass.
 *
 * APPROACH:
 *   - Background thread polls Player 1's ball velocity (0x170=X, 0x178=Z)
 *     and computes heading angle via atan2f.
 *   - Also reads the engine's own facing_direction fields at ball+0x194 (cos)
 *     and ball+0x198 (sin) for cross-reference.
 *   - Displays a compass direction (N/NE/E/SE/S/SW/W/NW) + heading angle
 *     in degrees on-screen via a Graphics_PresentOrEnd render hook.
 *   - Also writes to direction_log.txt for debugging.
 *
 * STRUCT OFFSETS (byte offsets):
 *   ball+0x164 = position X (float)
 *   ball+0x168 = position Y (float)
 *   ball+0x16C = position Z (float)
 *   ball+0x170 = velocity X (float)
 *   ball+0x174 = velocity Y (float)
 *   ball+0x178 = velocity Z (float)
 *   ball+0x188 = speed (float, magnitude of velocity)
 *   ball+0x194 = facing_direction_cos (float, cos of heading angle)
 *   ball+0x198 = facing_direction_sin (float, sin of heading angle)
 *   ball+0x018 = player_index (int, 0=Player 1, -1=NPC)
 *
 * COORDINATE SYSTEM:
 *   Y is UP. X and Z are horizontal. The ball "faces" in the direction
 *   of its XZ velocity. atan2(velZ, velX) gives the heading angle.
 *   We convert to compass: 0°=East, 90°=North, 180°=West, 270°=South.
 *
 * BUILD:
 *   i686-w64-mingw32-gcc -shared -o bass.dll direction_detect.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc -Wl,--add-stdcall-alias
 *
 * INSTALL:
 *   1. Rename original bass.dll → bass_real.dll in the Hamsterball directory
 *   2. Copy this compiled bass.dll to the same directory
 *   3. Launch Hamsterball.exe
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
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
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
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
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
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
 * Direction Detection
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game struct offsets */
#define APP_PTR          0x005341E0
#define BALL_POS_X       0x164
#define BALL_POS_Y       0x168
#define BALL_POS_Z       0x16C
#define BALL_VEL_X       0x170
#define BALL_VEL_Y       0x174
#define BALL_VEL_Z       0x178
#define BALL_SPEED       0x188
#define BALL_FACING_COS   0x194   /* engine's own facing direction (cos) */
#define BALL_FACING_SIN   0x198   /* engine's own facing direction (sin) */
#define BALL_PLAYER_IDX  0x018

/* Render hook: Graphics_PresentOrEnd at 0x455A90
 * __thiscall: ECX = GraphicsDevice (App+0x174), [ESP+4] = char (1=fullscreen)
 * First bytes: 8A 44 24 04 83 EC 20 3C 01 56 8B F1 (12 bytes before we can safely JMP)
 * We need at least 5 bytes for a JMP. The first instruction is:
 *   8A 44 24 04 = MOV AL, [ESP+4]  (4 bytes)
 *   83 EC 20   = SUB ESP, 0x20     (3 bytes)
 * We'll overwrite the first 8 bytes (two instructions) with JMP+NOPs.
 * Original: 8A 44 24 04 83 EC 20 3C
 * Patch:    E9 xx xx xx xx 90 90 90
 */
#define HOOK_ADDR          0x00455A90
#define HOOK_ORIG_BYTES    8
static const BYTE HOOK_ORIG[] = { 0x8A, 0x44, 0x24, 0x04, 0x83, 0xEC, 0x20, 0x3C };

/* Font_DrawCentered (0x42C870) — __thiscall, 8 params + this, RET 0x20
 * void Font_DrawCentered(Font* font, char* text, int x, int y,
 *                        int unused, float r, float g, float b, float a) */
typedef void (__fastcall *FontDrawCentered_t)(
    void* font, void* edx, char* text, int x, int y,
    int unused, float r, float g, float b, float a);
static FontDrawCentered_t FontDrawCentered = (FontDrawCentered_t)0x0042C870;

/* UI_DrawTextShadow_Wrapper (0x409B90) — __thiscall, 15 params, RET 0x3C
 * void DrawTextShadow(Font* font, char* text, int x, int y,
 *                     int shadow_x, int shadow_y, void* ign1,
 *                     float tr, float tg, float tb, float ta,
 *                     void* ign2, float sr, float sg, float sb, float sa) */
typedef void (__fastcall *DrawTextShadow_t)(
    void* font, void* edx, char* text, int x, int y,
    int sx, int sy, void* ign1,
    float tr, float tg, float tb, float ta,
    void* ign2, float sr, float sg, float sb, float sa);
static DrawTextShadow_t DrawTextShadow = (DrawTextShadow_t)0x00409B90;

/* Shared state — updated by background thread, read by render hook */
static volatile float  g_heading_deg    = 0.0f;   /* heading in degrees [0,360) */
static volatile float  g_speed          = 0.0f;   /* ball speed */
static volatile float  g_facing_cos     = 0.0f;   /* engine's facing cos */
static volatile float  g_facing_sin     = 0.0f;   /* engine's facing sin */
static volatile float  g_vel_x          = 0.0f;
static volatile float  g_vel_z          = 0.0f;
static volatile DWORD  g_ball_ptr       = 0;       /* for debugging */
static volatile BOOL   g_ball_found     = FALSE;
static volatile BOOL   g_show_direction = TRUE;     /* toggle with D key */

/* Original function pointer (trampoline) */
static BYTE g_trampolib[16];
static DrawTextShadow_t g_orig_PresentEnd = NULL;

/* Forward declarations */
static DWORD WINAPI poll_thread(LPVOID param);
static void install_render_hook(void);

/* ── Compass direction text ──────────────────────────────────────────── */

static const char* get_compass_dir(float deg)
{
    /* Normalize to [0, 360) */
    while (deg < 0.0f) deg += 360.0f;
    while (deg >= 360.0f) deg -= 360.0f;

    /* Compass: 0°=E, 90°=N, 180°=W, 270°=S
     * With 8 sectors of 45° each, centered on cardinal directions */
    if (deg < 22.5f || deg >= 337.5f) return "E";
    if (deg < 67.5f) return "NE";
    if (deg < 112.5f) return "N";
    if (deg < 157.5f) return "NW";
    if (deg < 202.5f) return "W";
    if (deg < 247.5f) return "SW";
    if (deg < 292.5f) return "S";
    return "SE";
}

/* ── Log helper ─────────────────────────────────────────────────────── */

static void write_log(const char *msg, int len)
{
    HANDLE hFile = CreateFileA("direction_log.txt", FILE_APPEND_DATA,
        FILE_SHARE_READ, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, msg, (DWORD)len, &written, NULL);
        CloseHandle(hFile);
    }
}

/* ── Scene/Ball finder ──────────────────────────────────────────────── */

static DWORD find_scene(DWORD app)
{
    if (!app || app < 0x10000) return 0;
    if (IsBadReadPtr((void*)app, 0x300)) return 0;

    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
        /* Check for ball_list at Scene+0x29D4 (AthenaList: count at +4, array at +0x40C) */
        DWORD count = *(DWORD*)((BYTE*)candidate + 0x29D4 + 0x004);
        DWORD array = *(DWORD*)((BYTE*)candidate + 0x29D4 + 0x40C);
        if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
            return candidate;
        }
    }
    return 0;
}

static DWORD find_player_ball(DWORD scene)
{
    if (!scene) return 0;

    int bcount = *(int*)((BYTE*)scene + 0x29D4 + 0x004);
    int *barray = *(int**)((BYTE*)scene + 0x29D4 + 0x40C);

    if (bcount <= 0 || bcount > 50) return 0;
    if (!barray || IsBadReadPtr(barray, bcount * 4)) return 0;

    for (int i = 0; i < bcount; i++) {
        DWORD ball = barray[i];
        if (!ball || ball < 0x10000) continue;
        if (IsBadReadPtr((void*)ball, 0xD00)) continue;
        if (*(int*)((BYTE*)ball + BALL_PLAYER_IDX) == 0) {
            return ball;  /* Player 1 */
        }
    }
    return 0;
}

/* ── Polling thread ─────────────────────────────────────────────────── */

static DWORD WINAPI poll_thread(LPVOID param)
{
    (void)param;
    Sleep(3000);  /* Wait for game to fully load */

    write_log("[MOD] direction_detect DLL loaded\r\n", 34);

    int log_counter = 0;
    DWORD last_ball = 0;
    DWORD last_log_tick = 0;

    for (;;) {
        Sleep(16);  /* ~60fps poll */

        /* Read keyboard for toggle (D key = DIK_D = 0x20) */
        DWORD app = *(DWORD*)APP_PTR;
        if (app && !IsBadReadPtr((void*)app, 0x200)) {
            DWORD inh = *(DWORD*)((BYTE*)app + 0x180);
            if (inh && !IsBadReadPtr((void*)inh, 0x500)) {
                DWORD kbd = *(DWORD*)((BYTE*)inh + 0x434);
                if (kbd && !IsBadReadPtr((void*)kbd, 0x600)) {
                    BYTE d_state = *((BYTE*)kbd + 0xC + 0x20);
                    static BOOL d_was_down = FALSE;
                    if (d_state & 0x80) {
                        if (!d_was_down) {
                            g_show_direction = !g_show_direction;
                            d_was_down = TRUE;
                        }
                    } else {
                        d_was_down = FALSE;
                    }
                }
            }
        }

        /* Find the player's ball */
        app = *(DWORD*)APP_PTR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        DWORD scene = find_scene(app);
        if (!scene) continue;

        DWORD ball = find_player_ball(scene);
        if (!ball) {
            g_ball_found = FALSE;
            continue;
        }

        g_ball_found = TRUE;
        g_ball_ptr = ball;

        /* Read velocity (XZ plane) */
        float vx = *(float*)((BYTE*)ball + BALL_VEL_X);
        float vz = *(float*)((BYTE*)ball + BALL_VEL_Z);
        float speed = *(float*)((BYTE*)ball + BALL_SPEED);

        /* Read engine's own facing direction */
        float fcos = *(float*)((BYTE*)ball + BALL_FACING_COS);
        float fsin = *(float*)((BYTE*)ball + BALL_FACING_SIN);

        /* Compute heading from velocity:
         * atan2(vz, vx) gives angle in radians where:
         *   0 = +X (East), pi/2 = +Z, pi = -X (West), -pi/2 = -Z
         * Convert to degrees and map to compass:
         *   +Z = North, +X = East, -Z = South, -X = West
         * So heading_deg = atan2(vz, vx) * 180/pi, then normalize to [0,360) */
        float heading_rad = atan2f(vz, vx);
        float heading_deg = heading_rad * (180.0f / 3.14159265f);
        if (heading_deg < 0.0f) heading_deg += 360.0f;

        /* Update shared state for render hook */
        g_heading_deg = heading_deg;
        g_speed = speed;
        g_facing_cos = fcos;
        g_facing_sin = fsin;
        g_vel_x = vx;
        g_vel_z = vz;

        /* Log on direction change (every ~1s or on ball change) */
        log_counter++;
        if (log_counter % 60 == 0 || ball != last_ball) {
            DWORD tick = GetTickCount();
            if (ball != last_ball || tick - last_log_tick > 1000) {
                char buf[256];
                int n = wsprintfA(buf,
                    "[%lu] ball=0x%08X dir=%s(%d) vel=(%.1f,%.1f) spd=%.1f fcos=%.3f fsin=%.3f\r\n",
                    (unsigned long)tick, ball, get_compass_dir(heading_deg),
                    (int)heading_deg, vx, vz, speed, fcos, fsin);
                write_log(buf, n);
                last_ball = ball;
                last_log_tick = tick;
            }
        }
    }
    return 0;
}

/* ── Render hook: Graphics_PresentOrEnd ──────────────────────────────── */

/* The render hook draws text BEFORE calling the original PresentOrEnd.
 * This is the correct timing: after viewport clear (slot 9), after object
 * render (slot 10), but before Present (EndScene).
 *
 * We hook at the function entry (0x455A90) with a 5-byte JMP + 3 NOPs.
 * The cave executes the original 8 bytes, draws our text, then JMPs back.
 *
 * Cave layout:
 *   PUSHAD
 *   ; Execute original 8 bytes: MOV AL,[ESP+4]; SUB ESP,0x20; CMP AL,1
 *   DB 8A 44 24 04 83 EC 20 3C
 *   ; ... (call C draw function from cave)
 *   POPAD
 *   JMP back_to_hook+8
 *
 * But calling C from a code cave is dangerous (pitfall #16).
 * Instead, we use the IAT detour pattern: overwrite the first bytes with a
 * JMP to our C function. The C function must:
 *   1. Match the calling convention (__thiscall: ECX=this, [ESP+4]=param)
 *   2. Execute the original instructions before returning
 *   3. Draw text, then call the original (via trampoline)
 *
 * Actually, Graphics_PresentOrEnd uses __thiscall convention:
 *   ECX = GraphicsDevice (this)
 *   [ESP+4] = char fullscreen (1=fullscreen)
 *
 * With __fastcall(ECX, EDX), the first stack param is at [ESP+4] (after return addr).
 * But since we're hooking the function entry, our trampoline needs to:
 *   - Save ECX (this ptr)
 *   - Execute original prologue
 *   - Draw text
 *   - Continue original function
 *
 * The simplest safe approach: use a trampoline buffer.
 * Copy original 8 bytes to trampoline, append JMP back to hook+8.
 * Replace hook site with JMP to our wrapper.
 * Our wrapper draws text, then calls trampoline.
 */

/* Trampoline: stores original bytes + JMP back */
static BYTE g_trampoline[32];
static BOOL g_hook_installed = FALSE;

/* Wrapper function — called instead of Graphics_PresentOrEnd.
 * __fastcall: ECX=this (GraphicsDevice), EDX=unused, [ESP+4]=fullscreen char.
 * But wait — Graphics_PresentOrEnd's first instruction reads [ESP+4].
 * Since we're replacing the function entry, the stack layout when our
 * wrapper is called is: [ESP]=return_addr, [ESP+4]=fullscreen param.
 *
 * We need to match: void __fastcall wrapper(void* gfxDev, void* edx, char fullscreen)
 *
 * However, the calling convention is actually __thiscall which means
 * callee cleans stack. RET 4 means 1 stack param (4 bytes).
 * Let's check: the function starts with MOV AL,[ESP+4], SUB ESP,0x20, CMP AL,1
 * The [ESP+4] suggests 1 stack param after the return address.
 *
 * Actually __thiscall passes this in ECX and has 0 stack params for RET 0,
 * 1 param for RET 4, etc. We need to determine RET N.
 *
 * For safety, we'll use the code cave approach: a hand-assembled cave
 * that saves registers, calls a C function for text drawing, then
 * executes original bytes and returns.
 */

/* The code cave approach for the render hook:
 *
 * At hook entry (0x455A90), we overwrite 8 bytes with:
 *   E9 <offset>   ; JMP to cave (5 bytes)
 *   90 90 90      ; NOP (3 bytes)
 *
 * The cave:
 *   ; Stack at entry: [ESP]=return addr, [ESP+4]=fullscreen param
 *   ; ECX = GraphicsDevice (this)
 *   ; We need to preserve ECX and the stack param
 *
 *   PUSHAD                    ; save all regs
 *   PUSHFD                    ; save flags
 *
 *   ; Call our C draw function
 *   ; Pass GraphicsDevice (ECX is clobbered by PUSHAD, read from saved copy)
 *   ; Actually PUSHAD pushes ECX to [ESP+0x18] (order: EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI)
 *   ; So ECX is at [ESP+0x18] after PUSHAD
 *
 *   ; Simpler: save ECX before PUSHAD
 *   ; But we need ECX for the original function too...
 *
 *   ; Approach: just call the C function with no params. The C function
 *   ; reads the font pointer from App+0x318 directly.
 *
 *   CALL draw_direction_overlay  ; C function, cdecl
 *
 *   POPFD
 *   POPAD
 *
 *   ; Execute original 8 bytes
 *   DB 8A 44 24 04     ; MOV AL, [ESP+4]
 *   DB 83 EC 20        ; SUB ESP, 0x20
 *   DB 3C              ; CMP AL, 1 (first byte of CMP, rest is 01 56)
 *
 *   ; Wait, 3C is CMP AL, imm8. The full instruction is 3C 01.
 *   ; But we only copied 8 bytes (8A 44 24 04 83 EC 20 3C).
 *   ; That's only the CMP AL opcode — the 01 is at offset 8, which we
 *   ; didn't overwrite. So we need to NOT re-execute the 3C byte,
 *   ; or we'll have a duplicate CMP.
 *
 *   ; Actually: original bytes 0-7 = 8A 44 24 04 83 EC 20 3C
 *   ; Byte 8 (not overwritten) = 01
 *   ; So original instruction at byte 7 is "3C 01" = CMP AL, 1
 *   ; If we JMP back to hook+8, byte 8 (01) executes naturally.
 *   ; But we already wrote NOP at byte 5,6,7... so the CMP is broken.
 *
 *   ; Fix: overwrite only 5 bytes (JMP + nothing else needed).
 *   ; Original byte 5 = 83, byte 6 = EC, byte 7 = 20, byte 8 = 3C
 *   ; If we overwrite bytes 0-4 with JMP (5 bytes), bytes 5+ are intact.
 *   ; But the JMP target (cave) must handle the case where bytes 0-4
 *   ; are skipped. We need to execute those original 5 bytes in the cave.
 *   ; Original 5 bytes: 8A 44 24 04 83
 *   ; That's: MOV AL, [ESP+4] (4 bytes) + first byte of SUB ESP,0x20 (1 byte)
 *   ; This is a partial instruction! Can't split 83 EC 20.
 *
 *   ; So we need to overwrite complete instructions:
 *   ; Instruction 1: 8A 44 24 04 (4 bytes) = MOV AL, [ESP+4]
 *   ; Instruction 2: 83 EC 20 (3 bytes) = SUB ESP, 0x20
 *   ; Total: 7 bytes for 2 complete instructions.
 *   ; JMP is 5 bytes. We need 7 bytes minimum, so JMP + 2 NOPs.
 *   ; Overwrite bytes 0-6 with: E9 xx xx xx xx 90 90
 *   ; Cave executes original 7 bytes: 8A 44 24 04 83 EC 20
 *   ; Then JMPs back to hook+7 (where byte 7 = 3C = CMP AL continues)
 *
 *   JMP (HOOK_ADDR + 7)
 */

static void __attribute__((used, noinline)) draw_direction_overlay(void)
{
    /* This function is called from the code cave during the render hook.
     * It reads the current direction state and draws text on screen.
     * We're inside BeginScene/EndScene (after viewport clear), so text
     * drawn here will be visible.
     *
     * IMPORTANT: We must NOT use CRT functions that touch the FPU stack
     * (snprintf, etc.). Use wsprintfA (Win32 only, no FPU side-effects).
     */

    if (!g_show_direction) return;

    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)app, 0x400)) return;

    /* Get font pointer (App+0x318 = ShowcardGothic28, main UI font) */
    void* font = *(void**)((BYTE*)app + 0x318);
    if (!font) return;
    if (IsBadReadPtr(font, 0x500)) return;

    if (!g_ball_found) return;

    /* Format direction text */
    char dir_text[128];
    float heading = g_heading_deg;
    const char* compass = get_compass_dir(heading);

    /* Main line: compass + degrees */
    int n = wsprintfA(dir_text, "Facing: %s (%d deg)", compass, (int)heading);
    if (n <= 0) return;

    /* Draw with shadow for readability (white text, black shadow) */
    /* UI_DrawTextShadow_Wrapper (0x409B90): __thiscall, 15 params, RET 0x3C
     * Params: font(this), text, x, y, shadow_x, shadow_y, ign1,
     *         r,g,b,a, ign2, sr,sg,sb,sa
     * Params 6 and 11 are overwritten internally — pass 0.
     * Colors ignored at Font+0x428==1.0, but we set them anyway.
     * Screen coords: (0,0) = top-left, X→, Y↓, default res 1024×768 */
    DrawTextShadow(font, NULL, dir_text, 360, 20,
                   2, 2, NULL,
                   1.0f, 1.0f, 1.0f, 1.0f,
                   NULL, 0.0f, 0.0f, 0.0f, 1.0f);

    /* Second line: velocity and speed */
    char vel_text[128];
    n = wsprintfA(vel_text, "Vel: (%.0f, %.0f)  Spd: %.1f",
                  g_vel_x, g_vel_z, g_speed);
    if (n > 0) {
        DrawTextShadow(font, NULL, vel_text, 360, 48,
                       2, 2, NULL,
                       1.0f, 1.0f, 1.0f, 1.0f,
                       NULL, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    /* Third line: engine's own facing cos/sin */
    char facing_text[128];
    n = wsprintfA(facing_text, "Engine: cos=%.3f sin=%.3f",
                  g_facing_cos, g_facing_sin);
    if (n > 0) {
        DrawTextShadow(font, NULL, facing_text, 360, 76,
                       2, 2, NULL,
                       1.0f, 1.0f, 1.0f, 1.0f,
                       NULL, 0.0f, 0.0f, 0.0f, 1.0f);
    }
}

/* Install the render hook via code cave */
static void install_render_hook(void)
{
    BYTE *hook_addr = (BYTE*)HOOK_ADDR;

    /* Verify hook site has expected bytes */
    if (memcmp(hook_addr, HOOK_ORIG, HOOK_ORIG_BYTES) != 0) {
        char buf[128];
        int n = wsprintfA(buf, "[MOD] ERROR: Render hook bytes mismatch at 0x%08X\r\n"
                               "Expected: 8A 44 24 04 83 EC 20 3C\r\n"
                               "Got: %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
                HOOK_ADDR,
                hook_addr[0], hook_addr[1], hook_addr[2], hook_addr[3],
                hook_addr[4], hook_addr[5], hook_addr[6], hook_addr[7]);
        write_log(buf, n);
        return;
    }

    /* Allocate code cave */
    BYTE *cave = (BYTE*)VirtualAlloc(NULL, 256, MEM_COMMIT | MEM_RESERVE,
                                     PAGE_EXECUTE_READWRITE);
    if (!cave) {
        write_log("[MOD] ERROR: VirtualAlloc failed for render cave\r\n", 48);
        return;
    }

    /* We need to overwrite 7 bytes (2 complete instructions):
     *   8A 44 24 04 (MOV AL, [ESP+4])    — 4 bytes
     *   83 EC 20    (SUB ESP, 0x20)      — 3 bytes
     * Total = 7 bytes. JMP = 5 bytes, so 5 + 2 NOPs = 7.
     */
    int p = 0;

    /* Cave prologue: save registers and flags */
    cave[p++] = 0x9C;  /* PUSHFD */
    cave[p++] = 0x60;  /* PUSHAD */

    /* Call draw_direction_overlay (cdecl, no params)
     * E8 <relative offset> — CALL near relative
     * Offset = target - (cave + p + 4) */
    cave[p++] = 0xE8;
    DWORD call_target = (DWORD)&draw_direction_overlay;
    DWORD call_offset = call_target - (DWORD)(cave + p + 4);
    *(DWORD*)(cave + p) = call_offset;
    p += 4;

    /* Cave epilogue */
    cave[p++] = 0x61;  /* POPAD */
    cave[p++] = 0x9D;  /* POPFD */

    /* Execute original 7 bytes: MOV AL,[ESP+4]; SUB ESP,0x20 */
    cave[p++] = 0x8A; cave[p++] = 0x44; cave[p++] = 0x24; cave[p++] = 0x04;
    cave[p++] = 0x83; cave[p++] = 0xEC; cave[p++] = 0x20;

    /* JMP back to HOOK_ADDR + 7 */
    cave[p++] = 0xE9;
    DWORD jmp_back = (DWORD)(hook_addr + 7) - (DWORD)(cave + p + 4);
    *(DWORD*)(cave + p) = jmp_back;
    p += 4;

    /* Patch the hook site: JMP + 2 NOPs (7 bytes total) */
    DWORD old_protect;
    VirtualProtect(hook_addr, 7, PAGE_EXECUTE_READWRITE, &old_protect);
    hook_addr[0] = 0xE9;
    *(DWORD*)(hook_addr + 1) = (DWORD)cave - (DWORD)hook_addr - 5;
    hook_addr[5] = 0x90;
    hook_addr[6] = 0x90;
    VirtualProtect(hook_addr, 7, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), hook_addr, 7);

    g_hook_installed = TRUE;

    char buf[128];
    int n = wsprintfA(buf, "[MOD] Render hook installed at 0x%08X, cave=0x%08X\r\n",
                     HOOK_ADDR, (DWORD)cave);
    write_log(buf, n);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Patch Thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI patch_thread(LPVOID param)
{
    (void)param;
    Sleep(5000);  /* Wait for game to fully load */

    write_log("[MOD] direction_detect patch thread started\r\n", 44);

    /* Install the render hook for on-screen display */
    install_render_hook();

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════ ESS ENTRY ═════════════════════ */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    (void)hInst; (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInst);
        load_real_bass();
        /* Start polling thread (reads ball direction) */
        CreateThread(NULL, 0, poll_thread, NULL, 0, NULL);
        /* Start patch thread (installs render hook after delay) */
        CreateThread(NULL, 0, patch_thread, NULL, 0, NULL);
    }

    return TRUE;
}
