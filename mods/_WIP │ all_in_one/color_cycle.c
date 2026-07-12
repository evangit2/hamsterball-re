/*
 * color_cycle.c — Safe BASS.dll proxy: Press F2 to cycle ball color
 *
 * How it works:
 *   - Background thread polls ~20x/second
 *   - On F2 keypress: cycles to next color in a preset list of 10 colors
 *   - Applies color to ball+0x208 (render context material)
 *   - Sets gfx+0x7C0 = ball+0x208 (material override for 3D mesh)
 *   - No config file, no IAT hooks, no code caves
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll color_cycle.c -lwinmm \
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
#define APP_PTR_ADDR   0x005341E0
#define GFX_OFFSET     0x174
#define MATERIAL_OVERRIDE_OFFSET 0x7C0
#define BALL_PLAYER_INDEX  0x018
#define BALL_RENDER_CTX2   0x208
#define SCENE_BALL_LIST  0x29D4
#define ATHENA_COUNT_OFFSET  0x004
#define ATHENA_ARRAY_OFFSET  0x40C
#define VK_F2 0x71

/* 10 preset colors: R, G, B (0.0-1.0) */
static const float colors[][3] = {
    {1.0f, 0.0f, 0.0f},   /* 0: Red */
    {1.0f, 0.5f, 0.0f},   /* 1: Orange */
    {1.0f, 1.0f, 0.0f},   /* 2: Yellow */
    {0.0f, 1.0f, 0.0f},   /* 3: Green */
    {0.0f, 1.0f, 1.0f},   /* 4: Cyan */
    {0.0f, 0.5f, 1.0f},   /* 5: Blue */
    {0.5f, 0.0f, 1.0f},   /* 6: Purple */
    {1.0f, 0.0f, 1.0f},   /* 7: Pink */
    {0.5f, 0.5f, 0.5f},   /* 8: Gray */
    {1.0f, 1.0f, 1.0f},   /* 9: White (default) */
};
static int g_color_idx = 9;  /* Start at white (default) */
static int g_f2_was_pressed = 0;

static void set_ball_material_color(DWORD ball, float r, float g, float b)
{
    DWORD rc = ball + BALL_RENDER_CTX2;
    if (IsBadWritePtr((void*)(rc + 0x04), 4)) return;
    /* Diffuse RGBA */
    *(float*)(rc + 0x04) = r; *(float*)(rc + 0x08) = g;
    *(float*)(rc + 0x0C) = b; *(float*)(rc + 0x10) = 1.0f;
    /* Ambient RGBA */
    *(float*)(rc + 0x14) = r; *(float*)(rc + 0x18) = g;
    *(float*)(rc + 0x1C) = b; *(float*)(rc + 0x20) = 1.0f;
    /* Specular RGBA (keep white for highlights) */
    *(float*)(rc + 0x24) = 1.0f; *(float*)(rc + 0x28) = 1.0f;
    *(float*)(rc + 0x2C) = 1.0f; *(float*)(rc + 0x30) = 1.0f;
    /* Emissive RGBA (slight glow) */
    *(float*)(rc + 0x34) = r * 0.3f; *(float*)(rc + 0x38) = g * 0.3f;
    *(float*)(rc + 0x3C) = b * 0.3f; *(float*)(rc + 0x40) = 1.0f;
    /* Power */
    *(float*)(rc + 0x44) = 20.0f;
}

static DWORD WINAPI color_thread(LPVOID param)
{
    Sleep(3000);  /* Wait for game to load */

    for (;;) {
        Sleep(50);  /* 20x/second poll */

        /* Check F2 keypress (edge detect — only cycle on press, not hold) */
        SHORT keyState = GetAsyncKeyState(VK_F2);
        int is_pressed = (keyState & 0x8000) ? 1 : 0;
        if (is_pressed && !g_f2_was_pressed) {
            g_color_idx = (g_color_idx + 1) % 10;
        }
        g_f2_was_pressed = is_pressed;

        /* Find App */
        DWORD app = *(DWORD*)APP_PTR_ADDR;
        if (!app || app < 0x10000) continue;
        if (IsBadReadPtr((void*)app, 0x300)) continue;

        /* Get Graphics object */
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
        if (!scene) continue;

        /* Find player 1's ball */
        DWORD list_base = scene + SCENE_BALL_LIST;
        int bcount = *(int*)(list_base + ATHENA_COUNT_OFFSET);
        DWORD *barray = *(DWORD**)(list_base + ATHENA_ARRAY_OFFSET);

        for (int i = 0; i < bcount; i++) {
            DWORD ball = barray[i];
            if (!ball || ball < 0x10000) continue;
            if (IsBadReadPtr((void*)ball, 0xD00)) continue;
            int pidx = *(int*)((BYTE*)ball + BALL_PLAYER_INDEX);
            if (pidx == 0) {
                float r = colors[g_color_idx][0];
                float g = colors[g_color_idx][1];
                float b = colors[g_color_idx][2];
                set_ball_material_color(ball, r, g, b);
                /* Do NOT set gfx+0x7C0 — that's a GLOBAL override that tints
                 * the entire scene. By only writing to ball+0x208, only the
                 * ball's own render context material changes. */
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
        HANDLE hThread = CreateThread(NULL, 0, color_thread, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
        break;
    }
    return TRUE;
}
