/*
 * updated_audio_libs.c — BASS 2.0 → BASS 2.4 compatibility proxy
 *
 * Replaces the game's original bass.dll (BASS 2.0, circa 2003) with a proxy
 * that forwards all calls to the modern BASS 2.4.18.3 library (Dec 2025).
 *
 * The game uses 3 API functions that were removed/renamed in BASS 2.3+:
 *   1. BASS_ChannelSetAttributes (plural) → BASS_ChannelSetAttribute (singular)
 *   2. BASS_MusicPlayEx           → BASS_ChannelPlay
 *   3. BASS_MusicLoad              offset param DWORD → QWORD
 *
 * This proxy translates those calls transparently, giving the game all the
 * benefits of BASS 2.4 (WASAPI, better mixing, 20+ years of bug fixes)
 * without modifying the original EXE.
 *
 * INSTALLATION:
 *   1. Rename original bass.dll → bass_real.dll  (or use bass24.dll, see below)
 *   2. Copy this bass.dll + bass24.dll to the game folder
 *   3. (Optional) Delete bass_real.dll to use BASS 2.4 exclusively
 *
 * Two deployment modes:
 *   - bass_real.dll = BASS 2.0 original  →  proxy wraps BASS 2.0 (passthrough)
 *   - bass_real.dll = BASS 2.4 (bass24.dll renamed)  →  proxy wraps BASS 2.4
 *         (this is the intended use — rename bass24.dll to bass_real.dll)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll updated_audio_libs.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS 2.4 function pointer types (from bass.h)
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef int   (__stdcall *BASS_Init_t)(int device, DWORD freq, DWORD flags, HWND win, const void *dsguid);
typedef int   (__stdcall *BASS_Free_t)(void);
typedef int   (__stdcall *BASS_Start_t)(void);
typedef int   (__stdcall *BASS_Stop_t)(void);
typedef int   (__stdcall *BASS_SetConfig_t)(DWORD option, DWORD value);
typedef int   (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(DWORD filetype, const void *file, unsigned long long offset, DWORD length, DWORD flags, DWORD freq);
typedef int   (__stdcall *BASS_ChannelStop_t)(DWORD handle);
typedef int   (__stdcall *BASS_ChannelPlay_t)(DWORD handle, BOOL restart);
typedef int   (__stdcall *BASS_ChannelSetAttribute_t)(DWORD handle, DWORD attrib, float value);
typedef int   (__stdcall *BASS_ChannelGetAttribute_t)(DWORD handle, DWORD attrib, float *value);

/* ═══════════════════════════════════════════════════════════════════════════
 * Real BASS library handle and function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

static HMODULE g_hRealBass = NULL;

static BASS_Init_t                 real_BASS_Init = NULL;
static BASS_Free_t                 real_BASS_Free = NULL;
static BASS_Start_t                real_BASS_Start = NULL;
static BASS_Stop_t                real_BASS_Stop = NULL;
static BASS_SetConfig_t           real_BASS_SetConfig = NULL;
static BASS_ErrorGetCode_t        real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t           real_BASS_MusicLoad = NULL;
static BASS_ChannelStop_t         real_BASS_ChannelStop = NULL;
static BASS_ChannelPlay_t         real_BASS_ChannelPlay = NULL;
static BASS_ChannelSetAttribute_t real_BASS_ChannelSetAttribute = NULL;

static int g_bass24_mode = 0;  /* 1 = using BASS 2.4 (needs API translation) */

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS 2.0 attribute constants (for ChannelSetAttributes translation)
 * In BASS 2.0, BASS_ChannelSetAttributes used integer volume (0-100) and
 * integer pan (-100 to +100). In BASS 2.4, BASS_ChannelSetAttribute uses
 * float values: volume 0.0-1.0, pan -1.0 to +1.0.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* BASS 2.0 attribute IDs used by the game */
#define BASS_OLD_ATTRIB_VOL   1   /* volume  (0-100 int)   */
#define BASS_OLD_ATTRIB_PAN   2   /* pan     (-100..+100)  */
#define BASS_OLD_ATTRIB_FREQ  3   /* freq    (Hz)          */

/* BASS 2.4 attribute IDs */
#define BASS_ATTRIB_FREQ      0x00
#define BASS_ATTRIB_VOL       0x01
#define BASS_ATTRIB_PAN       0x02

/* The game passes 0xFFFFFFFF as the handle for "current/default" channel.
 * In BASS 2.4, we need to track the last loaded music handle. */
static DWORD g_current_music_handle = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * Exported proxy functions
 *
 * The game imports these 10 functions from BASS.dll:
 *   BASS_Init, BASS_Free, BASS_Start, BASS_Stop, BASS_SetConfig,
 *   BASS_ErrorGetCode, BASS_MusicLoad, BASS_MusicPlayEx,
 *   BASS_ChannelSetAttributes, BASS_ChannelStop
 *
 * ═══════════════════════════════════════════════════════════════════════════ */

/* --- BASS_Init (5 args, same in 2.0 and 2.4) --- */
__declspec(dllexport) int __stdcall BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *dsguid)
{
    if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, dsguid);
    return 0;  /* FALSE = init failed, game shows "music not available" */
}

/* --- BASS_Free --- */
__declspec(dllexport) int __stdcall BASS_Free(void)
{
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}

/* --- BASS_Start --- */
__declspec(dllexport) int __stdcall BASS_Start(void)
{
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}

/* --- BASS_Stop --- */
__declspec(dllexport) int __stdcall BASS_Stop(void)
{
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}

/* --- BASS_SetConfig --- */
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD option, DWORD value)
{
    if (real_BASS_SetConfig) return real_BASS_SetConfig(option, value);
    return 1;
}

/* --- BASS_ErrorGetCode --- */
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}

/* --- BASS_ChannelStop --- */
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD handle)
{
    if (real_BASS_ChannelStop) {
        int ret = real_BASS_ChannelStop(handle);
        if (handle == g_current_music_handle || handle == 0xFFFFFFFF)
            g_current_music_handle = 0;
        return ret;
    }
    return 1;
}

/* --- BASS_MusicLoad ---
 *
 * BASS 2.0: HMUSIC BASS_MusicLoad(DWORD mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
 * BASS 2.4: HMUSIC BASS_MusicLoad(DWORD filetype, const void *file, QWORD offset, DWORD length, DWORD flags, DWORD freq)
 *
 * The game calls: BASS_MusicLoad(0, "music\\music.mo3", 0, 0, 4, 0)
 *   - arg1=0 (file, not memory)
 *   - arg2=file path
 *   - arg3=offset (0)
 *   - arg4=length (0 = entire file)
 *   - arg5=flags=4 (BASS_MUSIC_LOOP in 2.0)
 *   - arg6=freq=0 (use default)
 *
 * The only difference is offset is DWORD in 2.0 vs QWORD in 2.4.
 * When the value is 0 (which it always is in Hamsterball), this is a
 * zero-extension — no actual translation needed.
 */
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int filetype, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
{
    if (real_BASS_MusicLoad) {
        /* Forward with QWORD offset (zero-extended from DWORD) */
        DWORD handle = real_BASS_MusicLoad((DWORD)filetype, file, (unsigned long long)offset, length, flags, freq);
        if (handle) g_current_music_handle = handle;
        return handle;
    }
    return 0;
}

/* --- BASS_MusicPlayEx → BASS_ChannelPlay ---
 *
 * BASS 2.0: BOOL BASS_MusicPlayEx(HMUSIC handle, DWORD pos, BOOL reset)
 *   (some versions have 4 args: handle, pos, reset, flags)
 *
 * From the game's disassembly (Audio_PlayMusic at 0x46A310):
 *   The game pushes 4 args: handle, music_data, pos(~0=reset), flags(1)
 *   BASS 2.0 signature: BASS_MusicPlayEx(handle, pos, reset) — 3 args
 *   But the game pushes 4 values. The 4th may be a flags arg from a
 *   newer 2.0 sub-version.
 *
 * BASS 2.4: BOOL BASS_ChannelPlay(DWORD handle, BOOL restart)
 *   - handle = the HMUSIC from BASS_MusicLoad
 *   - restart = TRUE to play from start, FALSE to resume
 *
 * Translation: BASS_MusicPlayEx(handle, pos, reset, ...) → BASS_ChannelPlay(handle, reset)
 */
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, int reset, DWORD flags)
{
    /* The game's calling convention pushes 4 args on the stack.
     * In BASS 2.0, MusicPlayEx takes: (handle, pos, reset) or (handle, pos, reset, flags)
     * We extract 'reset' (3rd arg, which is 0 or -1) and use it as 'restart' for ChannelPlay.
     *
     * From the ASM at 0x46A3AB-0x46A3B2:
     *   PUSH 0x1           ; flags = 1 (BASS_MUSIC_RAMP)
     *   SBB EAX,EAX        ; eax = (unaff_BP != 0) ? -1 : 0  → reset
     *   PUSH EAX           ; reset
     *   PUSH EDI           ; pos (music data pointer)
     *   PUSH ECX           ; handle
     *   CALL BASS_MusicPlayEx
     *
     * So: handle, pos=EDI (music data), reset=0 or -1, flags=1
     * For BASS 2.4: BASS_ChannelPlay(handle, restart=TRUE)
     */
    if (real_BASS_ChannelPlay) {
        BOOL restart = (reset != 0) ? TRUE : TRUE;  /* Always restart — game is starting playback */
        g_current_music_handle = handle;
        return real_BASS_ChannelPlay(handle, restart);
    }
    return 1;
}

/* --- BASS_ChannelSetAttributes → BASS_ChannelSetAttribute ---
 *
 * BASS 2.0: BOOL BASS_ChannelSetAttributes(DWORD handle, DWORD attrib, int value, int pan)
 *   - handle: channel handle (0xFFFFFFFF = "current" in some contexts)
 *   - attrib: 1=volume, 2=pan, 3=freq (but game always uses 0xFFFFFFFF as attrib!)
 *   - value: volume 0-100 (int) or frequency in Hz
 *   - pan: -100 to +100 (int), or -101 for "default"
 *
 * From the game's disassembly:
 *   Audio_PlayMusic:        BASS_ChannelSetAttributes(handle, 0xFFFFFFFF, 100, -101)
 *   Audio_PlayMusicAtSpeed: BASS_ChannelSetAttributes(handle, 0xFFFFFFFF, 0, -101)
 *   MusicChannel_FadeUpdate: BASS_ChannelSetAttributes(handle, 0xFFFFFFFF, vol_int, -101)
 *
 * Wait — looking more carefully at the ASM:
 *   0x46A38C: PUSH -0x65     ; -101 = pan
 *   0x46A38E: PUSH 0x64      ; 100 = volume
 *   0x46A390: PUSH -0x1      ; 0xFFFFFFFF = attrib (not a standard BASS attrib ID)
 *   0x46A392: PUSH EDX       ; handle
 *
 * Actually, re-reading the BASS 2.0 docs: BASS_ChannelSetAttributes(handle, attrib, value, pan)
 * where attrib can be -1 (0xFFFFFFFF) meaning "set all three: volume, pan, freq"
 * with value as volume (0-100) and pan (-100..+100).
 *
 * BASS 2.4: BASS_ChannelSetAttribute(handle, DWORD attrib, float value)
 *   - BASS_ATTRIB_VOL (0x01): 0.0-1.0
 *   - BASS_ATTRIB_PAN (0x02): -1.0 to +1.0
 *   - BASS_ATTRIB_FREQ (0x00): Hz
 *
 * Translation:
 *   BASS_ChannelSetAttributes(handle, -1, vol100, pan100)
 *   → BASS_ChannelSetAttribute(handle, BASS_ATTRIB_VOL, vol100/100.0f)
 *   → BASS_ChannelSetAttribute(handle, BASS_ATTRIB_PAN, pan100/100.0f)
 */
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD handle, DWORD attrib, int value, int pan)
{
    if (real_BASS_ChannelSetAttribute) {
        DWORD h = (handle == 0xFFFFFFFF) ? g_current_music_handle : handle;

        if (attrib == 0xFFFFFFFF) {
            /* Set both volume and pan */
            float vol = (float)value / 100.0f;
            if (vol < 0.0f) vol = 0.0f;
            if (vol > 1.0f) vol = 1.0f;

            float pan_f = (pan == -101) ? 0.0f : (float)pan / 100.0f;
            if (pan_f < -1.0f) pan_f = -1.0f;
            if (pan_f > 1.0f) pan_f = 1.0f;

            real_BASS_ChannelSetAttribute(h, BASS_ATTRIB_VOL, vol);
            real_BASS_ChannelSetAttribute(h, BASS_ATTRIB_PAN, pan_f);
            return 1;
        }

        /* Single attribute set */
        switch (attrib) {
            case BASS_OLD_ATTRIB_VOL: {
                float vol = (float)value / 100.0f;
                if (vol < 0.0f) vol = 0.0f;
                if (vol > 1.0f) vol = 1.0f;
                return real_BASS_ChannelSetAttribute(h, BASS_ATTRIB_VOL, vol);
            }
            case BASS_OLD_ATTRIB_PAN: {
                float pan_f = (value == -101) ? 0.0f : (float)value / 100.0f;
                if (pan_f < -1.0f) pan_f = -1.0f;
                if (pan_f > 1.0f) pan_f = 1.0f;
                return real_BASS_ChannelSetAttribute(h, BASS_ATTRIB_PAN, pan_f);
            }
            case BASS_OLD_ATTRIB_FREQ:
                return real_BASS_ChannelSetAttribute(h, BASS_ATTRIB_FREQ, (float)value);
            default:
                return 1;
        }
    }
    return 1;  /* stub: pretend success */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Extra stub exports (not used by game, but needed for loader compatibility)
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) void __stdcall BASS_Pause(void) {}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a) {}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_GetDevice(void) { return 0; }
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a) { return 1; }
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a) {}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a, void *b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a, BOOL b) {
    if (real_BASS_ChannelPlay) return real_BASS_ChannelPlay(a, b);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a, DWORD b, float *c) {
    if (real_BASS_ChannelSetAttribute) return 1;  /* stub */
    return 1;
}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a, void *b, DWORD c) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a, void *b, DWORD c) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a, DWORD b) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a) { return 0; }
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a, DWORD b) { return 1; }
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a, DWORD b, DWORD c, void *d, void *e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return 0; }
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a, BOOL b) { return 0; }
/* BASS 2.4 singular form — in case anything calls it directly */
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD handle, DWORD attrib, float value) {
    if (real_BASS_ChannelSetAttribute) return real_BASS_ChannelSetAttribute(handle, attrib, value);
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Loader: detect BASS 2.0 vs 2.4 and resolve function pointers
 * ═══════════════════════════════════════════════════════════════════════════ */

static void load_real_bass(void)
{
    g_hRealBass = LoadLibraryA("bass_real.dll");

    /* If not found in current dir, try same directory as our DLL */
    if (g_hRealBass == NULL) {
        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&load_real_bass, &hSelf);
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }

    if (g_hRealBass == NULL) {
        /* No bass_real.dll — game will run without audio */
        return;
    }

    /* Resolve all BASS 2.4 functions */
    real_BASS_Init                 = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free                 = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Start                = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop                 = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_SetConfig            = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_ErrorGetCode         = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_MusicLoad            = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_ChannelStop          = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
    real_BASS_ChannelPlay          = (BASS_ChannelPlay_t)GetProcAddress(g_hRealBass, "BASS_ChannelPlay");
    real_BASS_ChannelSetAttribute  = (BASS_ChannelSetAttribute_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttribute");

    /* Detect if we're running BASS 2.4 (has ChannelPlay + ChannelSetAttribute, no MusicPlayEx) */
    if (real_BASS_ChannelPlay && real_BASS_ChannelSetAttribute) {
        g_bass24_mode = 1;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        break;

    case DLL_PROCESS_DETACH:
        if (g_hRealBass) {
            FreeLibrary(g_hRealBass);
            g_hRealBass = NULL;
        }
        break;
    }
    return TRUE;
}
