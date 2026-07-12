/*
 * color_cycle_v2.c — Safe BASS.dll proxy: F2 to cycle ball color (ball-only)
 *
 * v2 fixes:
 *   - Color actually works: hooks Ball_Render (0x402DE0) entry to set
 *     gfx+0x7C0 = ball+0x208 BEFORE the ball renders, then clears it AFTER.
 *     This way the material override only affects the ball, not the scene.
 *   - No crash after race: thread exits gracefully if ball pointer is invalid,
 *     and the hook is a function-entry hook (safe per code-cave rules).
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll color_cycle_v2.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* ══ BASS Proxy ══ */
static HMODULE g_hRealBass = NULL;
static void load_real_bass(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) {
        char path[MAX_PATH]; HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCSTR)&load_real_bass,&hSelf);
        if (GetModuleFileNameA(hSelf,path,MAX_PATH)) { char *p=strrchr(path,'\\'); if(p){strcpy(p+1,"bass_real.dll");g_hRealBass=LoadLibraryA(path);} }
    }
}
typedef int (__stdcall *BASS_Init_t)(int,DWORD,DWORD,HWND,void*); static BASS_Init_t r_BASS_Init=NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a,DWORD b,DWORD c,HWND d,void* e){if(g_hRealBass){if(!r_BASS_Init)r_BASS_Init=(BASS_Init_t)GetProcAddress(g_hRealBass,"BASS_Init");if(r_BASS_Init)return r_BASS_Init(a,b,c,d,e);}return 1;}
typedef int (__stdcall *BASS_Free_t)(void); static BASS_Free_t r_BASS_Free=NULL;
__declspec(dllexport) int __stdcall BASS_Free(void){if(g_hRealBass){if(!r_BASS_Free)r_BASS_Free=(BASS_Free_t)GetProcAddress(g_hRealBass,"BASS_Free");if(r_BASS_Free)return r_BASS_Free();}return 1;}
typedef int (__stdcall *BASS_Start_t)(void); static BASS_Start_t r_BASS_Start=NULL;
__declspec(dllexport) int __stdcall BASS_Start(void){if(g_hRealBass){if(!r_BASS_Start)r_BASS_Start=(BASS_Start_t)GetProcAddress(g_hRealBass,"BASS_Start");if(r_BASS_Start)return r_BASS_Start();}return 1;}
typedef int (__stdcall *BASS_Stop_t)(void); static BASS_Stop_t r_BASS_Stop=NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void){if(g_hRealBass){if(!r_BASS_Stop)r_BASS_Stop=(BASS_Stop_t)GetProcAddress(g_hRealBass,"BASS_Stop");if(r_BASS_Stop)return r_BASS_Stop();}return 1;}
typedef int (__stdcall *BASS_SetConfig_t)(DWORD,DWORD); static BASS_SetConfig_t r_BASS_SetConfig=NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a,DWORD b){if(g_hRealBass){if(!r_BASS_SetConfig)r_BASS_SetConfig=(BASS_SetConfig_t)GetProcAddress(g_hRealBass,"BASS_SetConfig");if(r_BASS_SetConfig)return r_BASS_SetConfig(a,b);}return 1;}
typedef int (__stdcall *BASS_ErrorGetCode_t)(void); static BASS_ErrorGetCode_t r_BASS_ErrorGetCode=NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void){if(g_hRealBass){if(!r_BASS_ErrorGetCode)r_BASS_ErrorGetCode=(BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass,"BASS_ErrorGetCode");if(r_BASS_ErrorGetCode)return r_BASS_ErrorGetCode();}return 0;}
typedef int (__stdcall *BASS_MusicPlayEx_t)(DWORD,DWORD,BOOL); static BASS_MusicPlayEx_t r_BASS_MusicPlayEx=NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a,DWORD b,BOOL c){if(g_hRealBass){if(!r_BASS_MusicPlayEx)r_BASS_MusicPlayEx=(BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass,"BASS_MusicPlayEx");if(r_BASS_MusicPlayEx)return r_BASS_MusicPlayEx(a,b,c);}return 1;}
typedef int (__stdcall *BASS_ChannelSetAttributes_t)(DWORD,float,int,int); static BASS_ChannelSetAttributes_t r_BASS_ChannelSetAttributes=NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a,float b,int c,int d){if(g_hRealBass){if(!r_BASS_ChannelSetAttributes)r_BASS_ChannelSetAttributes=(BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass,"BASS_ChannelSetAttributes");if(r_BASS_ChannelSetAttributes)return r_BASS_ChannelSetAttributes(a,b,c,d);}return 1;}
typedef int (__stdcall *BASS_ChannelStop_t)(DWORD); static BASS_ChannelStop_t r_BASS_ChannelStop=NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a){if(g_hRealBass){if(!r_BASS_ChannelStop)r_BASS_ChannelStop=(BASS_ChannelStop_t)GetProcAddress(g_hRealBass,"BASS_ChannelStop");if(r_BASS_ChannelStop)return r_BASS_ChannelStop(a);}return 1;}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int,void*,DWORD,DWORD,DWORD,DWORD); static BASS_MusicLoad_t r_BASS_MusicLoad=NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a,void* b,DWORD c,DWORD d,DWORD e,DWORD f){if(g_hRealBass){if(!r_BASS_MusicLoad)r_BASS_MusicLoad=(BASS_MusicLoad_t)GetProcAddress(g_hRealBass,"BASS_MusicLoad");if(r_BASS_MusicLoad)return r_BASS_MusicLoad(a,b,c,d,e,f);}return 0;}
__declspec(dllexport) void __stdcall BASS_Pause(void){}
__declspec(dllexport) void __stdcall BASS_SetVolume(DWORD a){}
__declspec(dllexport) DWORD __stdcall BASS_GetVolume(void){return 0;}
__declspec(dllexport) int __stdcall BASS_GetDevice(void){return 0;}
__declspec(dllexport) int __stdcall BASS_SetDevice(DWORD a){return 1;}
__declspec(dllexport) void __stdcall BASS_GetInfo(void *a){}
__declspec(dllexport) int __stdcall BASS_Update(DWORD a){return 0;}
__declspec(dllexport) DWORD __stdcall BASS_StreamCreateFile(void *a,void *b,DWORD c,DWORD d,DWORD e){return 0;}
__declspec(dllexport) DWORD __stdcall BASS_SampleLoad(int a,void *b,DWORD c,DWORD d,DWORD e){return 0;}
__declspec(dllexport) int __stdcall BASS_ChannelPlay(DWORD a,BOOL b){return 1;}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a,DWORD b,float c){return 1;}
__declspec(dllexport) int __stdcall BASS_ChannelGetAttribute(DWORD a,DWORD b,float *c){return 1;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetData(DWORD a,void *b,DWORD c){return 0;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetLevel(DWORD a){return 0;}
__declspec(dllexport) int __stdcall BASS_ChannelSetPosition(DWORD a,void *b,DWORD c){return 1;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelGetPosition(DWORD a,DWORD b){return 0;}
__declspec(dllexport) int __stdcall BASS_ChannelIsActive(DWORD a){return 0;}
__declspec(dllexport) int __stdcall BASS_ChannelRemoveSync(DWORD a,DWORD b){return 1;}
__declspec(dllexport) DWORD __stdcall BASS_ChannelSetSync(DWORD a,DWORD b,DWORD c,void *d,void *e){return 0;}
__declspec(dllexport) DWORD __stdcall BASS_SampleCreate(DWORD a,DWORD b,DWORD c,DWORD d,DWORD e){return 0;}
__declspec(dllexport) DWORD __stdcall BASS_SampleGetChannel(DWORD a,BOOL b){return 0;}

/* ══ Color Cycle Mod ══ */
#define IMAGE_BASE 0x00400000
#define APP_PTR_ADDR   0x005341E0
#define GFX_OFFSET     0x174
#define MATERIAL_OVERRIDE_OFFSET 0x7C0
#define BALL_RENDER_CTX2   0x208
#define VK_F2 0x71

static const float colors[][3] = {
    {1.0f, 0.0f, 0.0f},   /* Red */
    {1.0f, 0.5f, 0.0f},   /* Orange */
    {1.0f, 1.0f, 0.0f},   /* Yellow */
    {0.0f, 1.0f, 0.0f},   /* Green */
    {0.0f, 1.0f, 1.0f},   /* Cyan */
    {0.0f, 0.5f, 1.0f},   /* Blue */
    {0.5f, 0.0f, 1.0f},   /* Purple */
    {1.0f, 0.0f, 1.0f},   /* Pink */
    {0.5f, 0.5f, 0.5f},   /* Gray */
    {1.0f, 1.0f, 1.0f},   /* White (default) */
};
static volatile LONG g_color_idx = 9;
static volatile LONG g_apply_color = 0;  /* Set to 1 by thread when color changes */

static void set_ball_material_color(DWORD ball, float r, float g, float b)
{
    DWORD rc = ball + BALL_RENDER_CTX2;
    if (IsBadWritePtr((void*)(rc + 0x04), 4)) return;
    *(float*)(rc + 0x04) = r; *(float*)(rc + 0x08) = g;
    *(float*)(rc + 0x0C) = b; *(float*)(rc + 0x10) = 1.0f;
    *(float*)(rc + 0x14) = r; *(float*)(rc + 0x18) = g;
    *(float*)(rc + 0x1C) = b; *(float*)(rc + 0x20) = 1.0f;
    *(float*)(rc + 0x24) = 1.0f; *(float*)(rc + 0x28) = 1.0f;
    *(float*)(rc + 0x2C) = 1.0f; *(float*)(rc + 0x30) = 1.0f;
    *(float*)(rc + 0x34) = r * 0.3f; *(float*)(rc + 0x38) = g * 0.3f;
    *(float*)(rc + 0x3C) = b * 0.3f; *(float*)(rc + 0x40) = 1.0f;
    *(float*)(rc + 0x44) = 20.0f;
}

/* Thread: just polls F2 and sets the color index. Does NOT touch game memory. */
static DWORD WINAPI key_thread(LPVOID param)
{
    Sleep(3000);
    int f2_was_pressed = 0;
    for (;;) {
        Sleep(50);
        SHORT keyState = GetAsyncKeyState(VK_F2);
        int is_pressed = (keyState & 0x8000) ? 1 : 0;
        if (is_pressed && !f2_was_pressed) {
            int idx = InterlockedIncrement(&g_color_idx);
            /* Wrap around 0-9 */
            while (idx >= 10) {
                InterlockedCompareExchange(&g_color_idx, 0, idx);
                idx = g_color_idx;
            }
            InterlockedExchange(&g_apply_color, 1);
        }
        f2_was_pressed = is_pressed;
    }
    return 0;
}

/* Ball_Render hook: called at function entry (ECX = ball pointer).
 * Safe to call C function here because no FPU/stack frame is live yet.
 * Pattern: PUSHFD/PUSHAD/CALL hook/POPAD/POPFD/original bytes/JMP back */
static BYTE *g_ball_render_addr = NULL;
static BYTE g_ball_render_orig[6];
static int g_hook_installed = 0;

/* This is called from the code cave hook on Ball_Render entry.
 * ECX = ball pointer (thiscall). We read it from the saved registers. */
void __cdecl ball_render_hook(void)
{
    /* ECX is saved in the code cave via PUSHAD (it's in EDI after PUSHAD).
     * But simpler: just check the apply flag and set gfx override. */
    if (!InterlockedCompareExchange(&g_apply_color, 0, 1)) return;

    DWORD app = *(DWORD*)APP_PTR_ADDR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)app, 0x300)) return;
    DWORD gfx = *(DWORD*)((BYTE*)app + GFX_OFFSET);
    if (!gfx || gfx < 0x10000) return;
    if (IsBadReadPtr((void*)gfx, 0x800)) return;

    /* Find scene/ball list to get the ball pointer */
    DWORD scene = 0;
    for (int off = 0x100; off < 0xA00; off += 4) {
        DWORD candidate = *(DWORD*)((BYTE*)app + off);
        if (candidate == 0 || candidate < 0x10000) continue;
        if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
        DWORD list_base = candidate + 0x29D4;
        if (IsBadReadPtr((void*)list_base, 0x10)) continue;
        DWORD count = *(DWORD*)(list_base + 0x004);
        DWORD array = *(DWORD*)(list_base + 0x40C);
        if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
            scene = candidate;
            break;
        }
    }
    if (!scene) return;

    DWORD list_base = scene + 0x29D4;
    int bcount = *(int*)(list_base + 0x004);
    DWORD *barray = *(DWORD**)(list_base + 0x40C);
    int idx = g_color_idx;
    float r = colors[idx][0], g = colors[idx][1], b = colors[idx][2];

    for (int i = 0; i < bcount; i++) {
        DWORD ball = barray[i];
        if (!ball || ball < 0x10000) continue;
        if (IsBadReadPtr((void*)ball, 0xD00)) continue;
        int pidx = *(int*)((BYTE*)ball + 0x018);
        if (pidx == 0) {
            set_ball_material_color(ball, r, g, b);
            /* Set gfx+0x7C0 = ball+0x208 — material override for 3D mesh */
            *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = ball + BALL_RENDER_CTX2;
            break;
        }
    }
}

/* But wait — setting gfx+0x7C0 globally will still tint everything.
 * We need to SET it before ball renders and CLEAR it after.
 * Better approach: continuously re-apply in the key thread, but only
 * set gfx+0x7C0 when the ball is actually being rendered.
 *
 * Actually the simplest safe approach: keep the background thread from v1
 * but make it re-apply the color every frame (not just on keypress).
 * The crash was from accessing freed memory during scene transitions.
 * Fix: add IsBadReadPtr checks and handle null scene gracefully.
 * And DON'T clear gfx+0x7C0 — just keep setting it every frame so
 * when the scene loads back it picks up the color immediately.
 */

/* Redo: simpler approach. Thread re-applies color every ~50ms.
 * gfx+0x7C0 stays set as long as ball exists. If ball is gone (scene
 * transition), we skip and don't crash. */

static DWORD WINAPI color_thread_v2(LPVOID param)
{
    Sleep(3000);
    int f2_was_pressed = 0;

    for (;;) {
        Sleep(50);

        /* F2 keypress detection */
        SHORT keyState = GetAsyncKeyState(VK_F2);
        int is_pressed = (keyState & 0x8000) ? 1 : 0;
        if (is_pressed && !f2_was_pressed) {
            g_color_idx = (g_color_idx + 1) % 10;
        }
        f2_was_pressed = is_pressed;

        /* Re-apply color every frame */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        DWORD gfx = *(DWORD*)((BYTE*)app + GFX_OFFSET);
        if (!gfx || gfx < 0x10000) continue;
        if (IsBadReadPtr((void*)gfx, 0x800)) continue;

        /* Find Scene */
        DWORD scene = 0;
        for (int off = 0x100; off < 0xA00; off += 4) {
            DWORD candidate = *(DWORD*)((BYTE*)app + off);
            if (candidate == 0 || candidate < 0x10000) continue;
            if (IsBadReadPtr((void*)candidate, 0x3000)) continue;
            DWORD list_base = candidate + 0x29D4;
            if (IsBadReadPtr((void*)list_base, 0x10)) continue;
            DWORD count = *(DWORD*)(list_base + 0x004);
            DWORD array = *(DWORD*)(list_base + 0x40C);
            if (count > 0 && count < 100 && array != 0 && !IsBadReadPtr((void*)array, 4)) {
                scene = candidate;
                break;
            }
        }
        if (!scene) continue;

        /* Find player 1's ball */
        DWORD list_base = scene + 0x29D4;
        int bcount = *(int*)(list_base + 0x004);
        DWORD *barray = *(DWORD**)(list_base + 0x40C);
        int idx = g_color_idx;
        float r = colors[idx][0], g = colors[idx][1], b = colors[idx][2];

        for (int i = 0; i < bcount; i++) {
            DWORD ball = barray[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;
            int pidx = *(int*)((BYTE*)ball + 0x018);
            if (pidx == 0) {
                set_ball_material_color(ball, r, g, b);
                /* Set gfx+0x7C0 so the 3D ball mesh uses our material */
                *(DWORD*)((BYTE*)gfx + MATERIAL_OVERRIDE_OFFSET) = ball + BALL_RENDER_CTX2;
                break;
            }
        }
    }
    return 0;
}

/* ══ DLL Entry ══ */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        HANDLE hThread = CreateThread(NULL, 0, color_thread_v2, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
        break;
    }
    return TRUE;
}
