// ============================================================
// Raptisoft Live Status Logger — bass.dll proxy v1
//
// Taps into Hamsterball's hidden in-memory status tracking system
// that Raptisoft built for crash diagnostics. The game continuously
// updates three char* fields on the App struct but never writes
// them to disk — this mod polls them and logs to live_status.txt.
//
// WHAT IT LOGS:
//   App+0x208 (char*) — Init/startup phase tag
//     e.g. "Startup(2)", "App::Initialize(5)", "Graphics::Initialize(10)",
//          "FinishLoad(OK)", "Initialize(Ok)"
//   App+0x20C (char*) — Current object being processed
//   App+0x210 (char*) — Current runtime operation
//     e.g. "Background" (message pump), "Update" (game logic frame)
//
// Also logs one-time system info at startup:
//   Product name, version, fullscreen mode, resolution, target FPS
//
// OUTPUT: live_status.txt in the game folder (next to Hamsterball.exe)
//
// BUILD:
//   i686-w64-mingw32-gcc -shared -o bass.dll raptisoft_live_log.c \
//       -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
//       -Wl,--add-stdcall-alias
//
// NOTE: On Wine/llvmpipe, file I/O in bass.dll can break rendering.
// This mod is designed for real Windows. If crash-testing on Wine,
// expect a black screen — the log file will still be written correctly.
// ============================================================

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <mmsystem.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports (stubs — Hamsterball only needs import resolution)
 * ═══════════════════════════════════════════════════════════════════════════ */

__declspec(dllexport) void BASS_Init(void) {}
__declspec(dllexport) void BASS_Free(void) {}
__declspec(dllexport) void BASS_Start(void) {}
__declspec(dllexport) void BASS_Stop(void) {}
__declspec(dllexport) void BASS_Pause(void) {}
__declspec(dllexport) void BASS_SetVolume(void) {}
__declspec(dllexport) void BASS_GetVolume(void) {}
__declspec(dllexport) void BASS_GetDevice(void) {}
__declspec(dllexport) void BASS_SetDevice(void) {}
__declspec(dllexport) void BASS_GetInfo(void) {}
__declspec(dllexport) void BASS_Update(void) {}
__declspec(dllexport) void BASS_ErrorGetCode(void) {}
__declspec(dllexport) void BASS_StreamCreateFile(void) {}
__declspec(dllexport) void BASS_MusicLoad(void) {}
__declspec(dllexport) void BASS_SampleLoad(void) {}
__declspec(dllexport) void BASS_ChannelPlay(void) {}
__declspec(dllexport) void BASS_ChannelStop(void) {}
__declspec(dllexport) void BASS_ChannelSetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetAttribute(void) {}
__declspec(dllexport) void BASS_ChannelGetData(void) {}
__declspec(dllexport) void BASS_ChannelGetLevel(void) {}
__declspec(dllexport) void BASS_ChannelSetPosition(void) {}
__declspec(dllexport) void BASS_ChannelGetPosition(void) {}
__declspec(dllexport) void BASS_ChannelIsActive(void) {}
__declspec(dllexport) void BASS_ChannelRemoveSync(void) {}
__declspec(dllexport) void BASS_ChannelSetSync(void) {}
__declspec(dllexport) void BASS_SampleCreate(void) {}
__declspec(dllexport) void BASS_SampleGetChannel(void) {}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game Constants & Struct Offsets
 * ═══════════════════════════════════════════════════════════════════════════ */

#define IMAGE_BASE        0x400000

// App global is at 0x5341E0 (verified from WinMain → &g_App)
#define APP_PTR ((unsigned char*)0x005341E0)

// Live status fields (all char* pointers, updated by the game)
#define APP_STATUS_INIT    0x208   // Init/startup phase tag
#define APP_STATUS_OBJECT  0x20C   // Current object name
#define APP_STATUS_OP      0x210   // Current operation ("Update", "Background")

// One-time info fields
#define APP_PRODUCT_NAME   0x020   // AthenaString* — "Hamsterball"
#define APP_VERSION        0x03C   // AthenaString* — "V3.6.c"
#define APP_FULLSCREEN     0x158   // byte — 1=fullscreen, 0=windowed
#define APP_WIN_WIDTH      0x15C   // dword — windowed width
#define APP_WIN_HEIGHT     0x160   // dword — windowed height
#define APP_TARGET_FPS     0x16C   // dword — target FPS (75)
#define APP_REFRESH_RATE   0x170   // dword — refresh rate
#define APP_GFX_DEVICE     0x174   // ptr — D3D graphics device
#define APP_SOUND_DEVICE   0x178   // ptr — sound device
#define APP_MUSIC_DEVICE   0x17C   // ptr — music device
#define APP_INPUT_DEVICE   0x180   // ptr — input device
#define APP_SCENE_MGR      0x184   // ptr — MeshWorld/scene manager
#define APP_FPS_COUNTER    0x194   // dword — frames this second
#define APP_FPS_DISPLAY    0x198   // char[] — FPS display string
#define APP_SHOW_FPS       0x1AC   // byte — show FPS flag
#define APP_INITIALIZED    0x200   // byte — initialized flag

/* ═══════════════════════════════════════════════════════════════════════════
 * Safe string read — copies up to maxLen chars from a game pointer
 * Returns 1 if string was valid and copied, 0 if pointer was bad
 * ═══════════════════════════════════════════════════════════════════════════ */
static int safe_read_str(char* dest, const void* src, int maxLen) {
    if (src == NULL) {
        dest[0] = '\0';
        return 0;
    }
    if (IsBadReadPtr(src, 1)) {
        dest[0] = '\0';
        return 0;
    }
    // Copy byte-by-byte with bounds checking
    int i;
    for (i = 0; i < maxLen - 1; i++) {
        // Check each byte for readability
        if (IsBadReadPtr((const char*)src + i, 1)) {
            dest[i] = '\0';
            return 1; // partial read is still valid
        }
        dest[i] = ((const char*)src)[i];
        if (dest[i] == '\0') break;
    }
    dest[i] = '\0';
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * AthenaString reader — reads the C-string content from an AthenaString
 * AthenaString layout (simplified): [vtable][data_ptr or inline][length][...]
 * If the string is short (<16 chars), data is stored inline at offset +4.
 * If longer, offset +4 contains a pointer to the actual string data.
 * We try both: read inline first, if that looks like a pointer, follow it.
 * ═══════════════════════════════════════════════════════════════════════════ */
static void read_athena_string(unsigned char* app, int offset, char* dest, int maxLen) {
    dest[0] = '\0';
    unsigned char* astr = app + offset;
    if (IsBadReadPtr(astr, 16)) return;

    // AthenaString at offset has: [vtable(4)] [ptr_or_inline(4)] [len(4)] [capacity(4)]
    // If capacity < 16, the string data is inline starting at astr+4
    // If capacity >= 16, astr+4 is a pointer to heap-allocated data

    // Try reading inline first (works for short strings like "Hamsterball", "V3.6.c")
    // But we need to check if the value at astr+4 is a valid string or a pointer
    unsigned int val = *(unsigned int*)(astr + 4);

    // Check if it looks like a direct string (printable ASCII chars packed into first 4 bytes)
    char* inlinePtr = (char*)(astr + 4);
    if (!IsBadReadPtr(inlinePtr, 1) && (inlinePtr[0] >= 0x20 && inlinePtr[0] < 0x7F)) {
        // Could be inline string — try to read it
        safe_read_str(dest, inlinePtr, maxLen);
        // If we got a valid string, use it
        if (dest[0] != '\0') return;
    }

    // Otherwise treat val as a pointer to the string data
    if (val > 0x10000 && val < 0x7FFFFFFF) {
        safe_read_str(dest, (const void*)val, maxLen);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Live status logger thread
 * ═══════════════════════════════════════════════════════════════════════════ */

static volatile int g_running = 1;
static FILE* g_log = NULL;
static char g_log_path[MAX_PATH] = {0};

static DWORD WINAPI status_thread(LPVOID param) {
    char prev_init[256]  = {0};
    char prev_obj[256]   = {0};
    char prev_op[256]    = {0};
    char curr_init[256]  = {0};
    char curr_obj[256]   = {0};
    char curr_op[256]    = {0};

    // Previous FPS string for FPS change detection
    char prev_fps[32] = {0};

    DWORD start_tick = GetTickCount();

    // Wait for App to be initialized (App global non-zero)
    int wait_count = 0;
    while (g_running) {
        if (!IsBadReadPtr(APP_PTR, 4) && *(unsigned int*)APP_PTR != 0) {
            // Check if App vtable is valid
            unsigned int vtable = *(unsigned int*)APP_PTR;
            if (vtable > 0x400000 && vtable < 0x500000) break;
        }
        Sleep(50);
        wait_count++;
        if (wait_count > 600) {
            // 30 second timeout — log anyway with whatever we have
            fprintf(g_log, "[WARNING] App struct not initialized after 30s — logging raw memory\n\n");
            break;
        }
    }

    // Write header
    unsigned char* app = APP_PTR;
    fprintf(g_log, "=======================================================================\n");
    fprintf(g_log, "          HAMSTERBALL LIVE STATUS LOG - Raptisoft Debug Tap          \n");
    fprintf(g_log, "=======================================================================\n\n");

    // Timestamp
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(g_log, "Log started: %04d-%02d-%02d %02d:%02d:%02d\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    fprintf(g_log, "Process PID: %lu\n\n", GetCurrentProcessId());

    // System info is written lazily — fields populate as the game initializes.
    // We'll log them as they become available in the main loop below.
    int sysinfo_logged = 0;

    fprintf(g_log, "--- Live Status (updates on change) ---\n");
    fprintf(g_log, "[tick]     STATUS                          OBJECT              OPERATION\n");
    fprintf(g_log, "-----------------------------------------------------------------------------\n");
    fflush(g_log);

    // Main polling loop
    DWORD frame_count = 0;
    DWORD last_fps_log = GetTickCount();
    DWORD last_dev_log = 0;
    int prev_gfx = 0, prev_snd = 0, prev_mus = 0, prev_inp = 0, prev_mw = 0;

    while (g_running) {
        // --- Lazy system info: log fields as they populate ---
        if (!sysinfo_logged) {
            // Check if graphics device is now active
            if (!IsBadReadPtr(app + APP_GFX_DEVICE, 4)) {
                DWORD gfx = *(DWORD*)(app + APP_GFX_DEVICE);
                if (gfx && gfx != prev_gfx) {
                    // Graphics just initialized — dump system info now
                    char product[64] = {0};
                    char version[64] = {0};
                    read_athena_string(app, APP_PRODUCT_NAME, product, sizeof(product));
                    read_athena_string(app, APP_VERSION, version, sizeof(version));

                    BYTE fullscreen = app[APP_FULLSCREEN];
                    DWORD win_w = *(DWORD*)(app + APP_WIN_WIDTH);
                    DWORD win_h = *(DWORD*)(app + APP_WIN_HEIGHT);
                    DWORD target_fps = *(DWORD*)(app + APP_TARGET_FPS);
                    DWORD refresh = *(DWORD*)(app + APP_REFRESH_RATE);

                    fprintf(g_log, "\n--- System Info ---\n");
                    fprintf(g_log, "  Product:     %s\n", product[0] ? product : "(unknown)");
                    fprintf(g_log, "  Version:     %s\n", version[0] ? version : "(unknown)");
                    fprintf(g_log, "  Fullscreen:  %s\n", fullscreen ? "YES" : "NO");
                    fprintf(g_log, "  Resolution:  %lux%lu\n", win_w, win_h);
                    fprintf(g_log, "  Target FPS:  %lu\n", target_fps);
                    fprintf(g_log, "  Refresh:     %lu Hz\n\n", refresh);

                    fprintf(g_log, "--- Device Pointers ---\n");
                    fprintf(g_log, "  Graphics:     0x%08X (active)\n", gfx);
                    if (!IsBadReadPtr(app + APP_SOUND_DEVICE, 4)) {
                        DWORD snd = *(DWORD*)(app + APP_SOUND_DEVICE);
                        fprintf(g_log, "  Sound:        0x%08X %s\n", snd, snd ? "(active)" : "(null)");
                        prev_snd = snd;
                    }
                    if (!IsBadReadPtr(app + APP_MUSIC_DEVICE, 4)) {
                        DWORD mus = *(DWORD*)(app + APP_MUSIC_DEVICE);
                        fprintf(g_log, "  Music:        0x%08X %s\n", mus, mus ? "(active)" : "(null)");
                        prev_mus = mus;
                    }
                    if (!IsBadReadPtr(app + APP_INPUT_DEVICE, 4)) {
                        DWORD inp = *(DWORD*)(app + APP_INPUT_DEVICE);
                        fprintf(g_log, "  Input:        0x%08X %s\n", inp, inp ? "(active)" : "(null)");
                        prev_inp = inp;
                    }
                    if (!IsBadReadPtr(app + APP_SCENE_MGR, 4)) {
                        DWORD mw = *(DWORD*)(app + APP_SCENE_MGR);
                        fprintf(g_log, "  Scene/MeshWorld: 0x%08X %s\n", mw, mw ? "(active)" : "(null)");
                        prev_mw = mw;
                    }
                    fprintf(g_log, "\n--- Live Status (updates on change) ---\n");
                    fprintf(g_log, "[tick]     STATUS                          OBJECT              OPERATION\n");
                    fprintf(g_log, "-----------------------------------------------------------------------------\n");
                    fflush(g_log);
                    sysinfo_logged = 1;
                    prev_gfx = gfx;
                    // Reset previous status strings so we re-log from the start
                    prev_init[0] = '\0';
                    prev_obj[0] = '\0';
                    prev_op[0] = '\0';
                }
            }
        } else {
            // After initial sysinfo, log device pointer changes
            DWORD now = GetTickCount();
            if (now - last_dev_log >= 1000) {
                if (!IsBadReadPtr(app + APP_SOUND_DEVICE, 4)) {
                    DWORD snd = *(DWORD*)(app + APP_SOUND_DEVICE);
                    if (snd != (DWORD)prev_snd) {
                        fprintf(g_log, "[%7lu]  [DEV] Sound device: 0x%08X -> 0x%08X\n",
                            now - start_tick, (DWORD)prev_snd, snd);
                        fflush(g_log);
                        prev_snd = snd;
                    }
                }
                if (!IsBadReadPtr(app + APP_MUSIC_DEVICE, 4)) {
                    DWORD mus = *(DWORD*)(app + APP_MUSIC_DEVICE);
                    if (mus != (DWORD)prev_mus) {
                        fprintf(g_log, "[%7lu]  [DEV] Music device: 0x%08X -> 0x%08X\n",
                            now - start_tick, (DWORD)prev_mus, mus);
                        fflush(g_log);
                        prev_mus = mus;
                    }
                }
                if (!IsBadReadPtr(app + APP_SCENE_MGR, 4)) {
                    DWORD mw = *(DWORD*)(app + APP_SCENE_MGR);
                    if (mw != (DWORD)prev_mw) {
                        fprintf(g_log, "[%7lu]  [DEV] Scene manager: 0x%08X -> 0x%08X\n",
                            now - start_tick, (DWORD)prev_mw, mw);
                        fflush(g_log);
                        prev_mw = mw;
                    }
                }
                last_dev_log = now;
            }
        }

        // --- Read the three live status fields ---
        char* init_ptr = NULL;
        char* obj_ptr = NULL;
        char* op_ptr = NULL;

        if (!IsBadReadPtr(app + APP_STATUS_INIT, 4))
            init_ptr = *(char**)(app + APP_STATUS_INIT);
        if (!IsBadReadPtr(app + APP_STATUS_OBJECT, 4))
            obj_ptr = *(char**)(app + APP_STATUS_OBJECT);
        if (!IsBadReadPtr(app + APP_STATUS_OP, 4))
            op_ptr = *(char**)(app + APP_STATUS_OP);

        // Safe-read the strings
        curr_init[0] = '\0';
        curr_obj[0] = '\0';
        curr_op[0] = '\0';

        if (init_ptr && !IsBadReadPtr(init_ptr, 1))
            safe_read_str(curr_init, init_ptr, sizeof(curr_init));
        if (obj_ptr && !IsBadReadPtr(obj_ptr, 1))
            safe_read_str(curr_obj, obj_ptr, sizeof(curr_obj));
        if (op_ptr && !IsBadReadPtr(op_ptr, 1))
            safe_read_str(curr_op, op_ptr, sizeof(curr_op));

        // Default to "(null)" for display if empty
        const char* disp_init = curr_init[0] ? curr_init : "(null)";
        const char* disp_obj  = curr_obj[0]  ? curr_obj  : "(null)";
        const char* disp_op   = curr_op[0]   ? curr_op   : "(null)";

        // Log on change of any field
        if (strcmp(disp_init, prev_init) != 0 ||
            strcmp(disp_obj, prev_obj) != 0 ||
            strcmp(disp_op, prev_op) != 0) {

            DWORD tick = GetTickCount() - start_tick;
            fprintf(g_log, "[%7lu]  %-30s  %-18s  %s\n",
                    tick, disp_init, disp_obj, disp_op);
            fflush(g_log);

            strncpy(prev_init, disp_init, sizeof(prev_init) - 1);
            strncpy(prev_obj, disp_obj, sizeof(prev_obj) - 1);
            strncpy(prev_op, disp_op, sizeof(prev_op) - 1);
            prev_init[sizeof(prev_init)-1] = '\0';
            prev_obj[sizeof(prev_obj)-1] = '\0';
            prev_op[sizeof(prev_op)-1] = '\0';
        }

        // Also log FPS string every second if it changes
        frame_count++;
        DWORD now2 = GetTickCount();
        if (now2 - last_fps_log >= 1000) {
            char fps_str[32] = {0};
            if (!IsBadReadPtr(app + APP_SHOW_FPS, 1) && app[APP_SHOW_FPS]) {
                safe_read_str(fps_str, (const void*)(app + APP_FPS_DISPLAY), sizeof(fps_str));
                if (fps_str[0] && strcmp(fps_str, prev_fps) != 0) {
                    fprintf(g_log, "[%7lu]  FPS: %s\n", now2 - start_tick, fps_str);
                    fflush(g_log);
                    strncpy(prev_fps, fps_str, sizeof(prev_fps) - 1);
                    prev_fps[sizeof(prev_fps)-1] = '\0';
                }
            }
            last_fps_log = now2;
        }

        Sleep(10); // ~100Hz polling
    }

    // Cleanup
    fprintf(g_log, "\n--- Log Ended ---\n");
    fprintf(g_log, "Total frames polled: %lu\n", frame_count);
    fflush(g_log);

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry Point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    static HANDLE g_thread = NULL;

    if (reason == DLL_PROCESS_ATTACH) {
        // Find the game directory for the log file
        char exe_path[MAX_PATH];
        if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) > 0) {
            // Replace .exe with _live_status.txt
            char* slash = strrchr(exe_path, '\\');
            if (slash) {
                strcpy(slash + 1, "live_status.txt");
            } else {
                strcpy(exe_path, "live_status.txt");
            }
            strncpy(g_log_path, exe_path, MAX_PATH - 1);
            g_log_path[MAX_PATH - 1] = '\0';
        } else {
            strcpy(g_log_path, "live_status.txt");
        }

        // Open log file
        fopen_s(&g_log, g_log_path, "w");
        if (g_log) {
            fprintf(g_log, "[MOD] Raptisoft Live Status Logger loaded\n");
            fprintf(g_log, "[MOD] Log file: %s\n", g_log_path);
            fflush(g_log);

            g_thread = CreateThread(NULL, 0, status_thread, NULL, 0, NULL);
            if (g_thread) {
                SetThreadPriority(g_thread, THREAD_PRIORITY_BELOW_NORMAL);
            }
        }
    }
    else if (reason == DLL_PROCESS_DETACH) {
        g_running = 0;
        if (g_thread) {
            WaitForSingleObject(g_thread, 2000);
            CloseHandle(g_thread);
        }
        if (g_log) {
            fclose(g_log);
        }
    }
    return TRUE;
}
