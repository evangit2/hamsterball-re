/*
 * ball_color_cycle.c — BASS.dll proxy mod (v3 — Android-safe)
 *
 * Cycles player 1's ball through 10 colors with F2 key.
 *
 * CRASH FIX (v2→v3):
 *   v2 crashed after finishing a race because gfx+0x7C0 was set to
 *   ball+0x208 but NEVER cleared. When the ball was destroyed at
 *   race end, gfx+0x7C0 became a dangling pointer to freed memory.
 *   The render thread then read from freed memory → crash.
 *
 *   v3 fix: CLEAR gfx+0x7C0 = 0 when ball is not found (during scene
 *   transitions). This makes the game use its default material (no crash).
 *
 * ANDROID SAFETY:
 *   - No IAT hooks (GetTickCount hook crashes on Android)
 *   - No code caves / mid-function hooks
 *   - Background thread only does memory reads/writes (no D3D API calls)
 *   - All pointer accesses guarded by IsBadReadPtr
 *   - gfx+0x7C0 cleared to 0 when ball not found
 *
 * Controls:
 *   F2 = cycle to next color (10 colors)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ball_color_cycle.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * BASS Proxy Exports
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
        GetModuleFileNameA(hSelf, path, MAX_PATH);
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "bass_real.dll");
            g_hRealBass = LoadLibraryA(path);
        }
    }
    if (g_hRealBass) {
        #define LOAD(name) real_##name = (name##_t)GetProcAddress(g_hRealBass, #name)
        LOAD(BASS_ChannelSetAttributes);
        LOAD(BASS_MusicPlayEx);
        LOAD(BASS_SetConfig);
        LOAD(BASS_Init);
        LOAD(BASS_Free);
        LOAD(BASS_Start);
        LOAD(BASS_Stop);
        LOAD(BASS_ErrorGetCode);
        LOAD(BASS_MusicLoad);
        LOAD(BASS_ChannelStop);
        #undef LOAD
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Ball Color Cycle Mod
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Game addresses (static) */
#define APP_PTR_ADDR   0x005341E0   /* Global pointer to App struct */
#define GFX_OFFSET     0x174        /* App+0x174 = Graphics object pointer */
#define MATERIAL_OVERRIDE_OFFSET 0x7C0  /* gfx+0x7C0 = material override pointer */

/* Ball struct offsets */
#define BALL_PLAYER_INDEX  0x018   /* ball+0x18 = player index (0=player 1) */
#define BALL_RENDER_CTX2   0x208   /* ball+0x208 = render context (sphere/sprite material) */

/* Scene struct offsets for finding balls */
#define SCENE_BALL_LIST     0x29D4
#define ATHENA_COUNT_OFFSET 0x004
#define ATHENA_ARRAY_OFFSET 0x40C

/* 10 preset colors (R, G, B as floats 0.0-1.0) */
static const float g_colors[10][3] = {
    { 1.0f, 1.0f, 1.0f },   /* 0: White (default) */
    { 1.0f, 0.42f, 0.21f }, /* 1: Orange */
    { 0.29f, 0.56f, 0.85f },/* 2: Blue */
    { 0.18f, 0.80f, 0.44f },/* 3: Green */
    { 0.87f, 0.20f, 0.60f },/* 4: Pink */
    { 1.0f, 0.84f, 0.0f },  /* 5: Yellow */
    { 0.56f, 0.18f, 0.87f },/* 6: Purple */
    { 0.0f, 0.81f, 0.82f }, /* 7: Cyan */
    { 1.0f, 0.13f, 0.13f }, /* 8: Red */
    { 0.98f, 0.55f, 0.0f }, /* 9: Dark Orange */
};

static volatile int g_color_idx = 0;      /* Current color index (0-9) */
static volatile BOOL g_color_active = FALSE; /* TRUE when user has pressed F2 at least once */

/* Write RGBA floats into ball render context material */
static void set_ball_material_color(DWORD ball, float r, float g, float b)
{
    DWORD rc = ball + BALL_RENDER_CTX2;

    if (IsBadWritePtr((void*)(rc + 0x04), 4)) return;

    /* Diffuse RGBA (rc+0x04..0x10) */
    *(float*)(rc + 0x04) = r;
    *(float*)(rc + 0x08) = g;
    *(float*)(rc + 0x0C) = b;
    *(float*)(rc + 0x10) = 1.0f;

    /* Ambient RGBA (rc+0x14..0x20) */
    *(float*)(rc + 0x14) = r;
    *(float*)(rc + 0x18) = g;
    *(float*)(rc + 0x1C) = b;
    *(float*)(rc + 0x20) = 1.0f;

    /* Specular RGBA (rc+0x24..0x30) — keep white for highlights */
    *(float*)(rc + 0x24) = 1.0f;
    *(float*)(rc + 0x28) = 1.0f;
    *(float*)(rc + 0x2C) = 1.0f;
    *(float*)(rc + 0x30) = 1.0f;

    /* Emissive RGBA (rc+0x34..0x40) — slight glow tint */
    *(float*)(rc + 0x34) = r * 0.3f;
    *(float*)(rc + 0x38) = g * 0.3f;
    *(float*)(rc + 0x3C) = b * 0.3f;
    *(float*)(rc + 0x40) = 1.0f;

    /* Power (rc+0x44) */
    *(float*)(rc + 0x44) = 20.0f;
}

/* Background thread: poll F2 key and apply color */
static DWORD WINAPI color_thread(LPVOID param)
{
    Sleep(3000);  /* Wait for game to fully load */

    BOOL f2_was_down = FALSE;

    for (;;) {
        Sleep(50);  /* 20fps poll */

        /* Check F2 key (VK_F2 = 0x71) */
        SHORT key_state = GetAsyncKeyState(0x71);
        BOOL f2_down = (key_state & 0x8000) != 0;

        if (f2_down && !f2_was_down) {
            /* F2 pressed — cycle to next color */
            g_color_idx = (g_color_idx + 1) % 10;
            g_color_active = TRUE;
        }
        f2_was_down = f2_down;

        /* If color not activated yet, do nothing */
        if (!g_color_active) continue;

        /* Get current color */
        int idx = g_color_idx;
        float r = g_colors[idx][0];
        float g = g_colors[idx][1];
        float b = g_colors[idx][2];

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Get Graphics object (App+0x174) */
        DWORD gfx = *(DWORD*)((BYTE*)app + GFX_OFFSET);
        if (!gfx || gfx < 0x10000) continue;
        if (IsBadReadPtr((void*)gfx, 0x800)) continue;

        /* Find Scene by scanning App for ball list */
        DWORD scene = 0;
        for (int off = 0x100; off < 0xA00; off += 4) {
            DWORD candidate = *(DWORD*)((BYTE*)app + off);
            if (candidate == 0 || candidate < 0x10000) continue;
            if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
            DWORD list_base = candidate + SCENE_BALL_LIST;
            if (IsBadReadPtr((void*)list_base, 0x10)) continue;
            DWORD count = *(DWORD*)(list_base + ATHENA_COUNT_OFFSET);
            DWORD array = *(DWORD*)(list_base + ATHENA_ARRAY_OFFSET);
            if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
                scene = candidate;
                break;
            }
        }

        if (!scene) {
            /* No scene found — CLEAR gfx+0x7C0 to prevent dangling pointer crash!
             * This is the critical fix. v2 left gfx+0x7C0 pointing at freed ball
             * memory after race end, causing the render thread to read freed memory. */
            if (IsBadWritePtr((void*)(gfx + MATERIAL_OVERRIDE_OFFSET), 4)) continue;
            *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = 0;
            continue;
        }

        /* Find player 1's ball (player_index == 0) */
        DWORD list_base = scene + SCENE_BALL_LIST;
        int bcount = *(int*)(list_base + ATHENA_COUNT_OFFSET);
        DWORD *barray = *(DWORD**)(list_base + ATHENA_ARRAY_OFFSET);

        BOOL ball_found = FALSE;

        for (int i = 0; i < bcount && i < 100; i++) {
            if (IsBadReadPtr((void*)&barray[i], 4)) break;
            DWORD ball = barray[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;

            int pidx = *(int*)((BYTE*)ball + BALL_PLAYER_INDEX);
            if (pidx == 0) {
                /* Found player 1's ball — set color */
                set_ball_material_color(ball, r, g, b);

                /* Set gfx+0x7C0 = ball+0x208 (material override)
                 * This makes the game use OUR material for the 3D sphere */
                if (!IsBadWritePtr((void*)(gfx + MATERIAL_OVERRIDE_OFFSET), 4)) {
                    *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = ball + BALL_RENDER_CTX2;
                }

                ball_found = TRUE;
                break;
            }
        }

        /* CRITICAL FIX: If ball not found (scene transition, race end),
         * CLEAR gfx+0x7C0 = 0 so the game uses its default material.
         * v2 didn't do this — the dangling pointer caused the crash. */
        if (!ball_found) {
            if (!IsBadWritePtr((void*)(gfx + MATERIAL_OVERRIDE_OFFSET), 4)) {
                *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = 0;
            }
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DLL Entry
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        load_real_bass();
        {
            HANDLE hThread = CreateThread(NULL, 0, color_thread, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        break;
    }
    return TRUE;
}
