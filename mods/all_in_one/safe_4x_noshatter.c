/*
 * safe_4x_noshatter.c — Safe BASS.dll proxy: 4x Ball Size + No Shatter Effect
 *
 * What this does:
 *   1. 4x Ball Size — all balls 4x bigger
 *   2. No Shatter — Ball_Shatter and Ball_Shatter_OnRamp are RETed at entry,
 *      skipping the visual breaking animation AND the sound effect.
 *      But the RESPAWN SYSTEM is left untouched — if the ball falls off
 *      an edge, it still respawns at the last checkpoint normally.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_4x_noshatter.c \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <string.h>

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

/* ══ Helpers ══ */
#define IMAGE_BASE 0x00400000
static int patch_bytes(BYTE *addr,const BYTE *exp,const BYTE *rep,SIZE_T len){DWORD op;if(memcmp(addr,exp,len)!=0)return 0;if(!VirtualProtect(addr,len,PAGE_EXECUTE_READWRITE,&op))return 0;memcpy(addr,rep,len);VirtualProtect(addr,len,op,&op);FlushInstructionCache(GetCurrentProcess(),addr,len);return 1;}
static int write_bytes(BYTE *addr,const BYTE *data,SIZE_T len){DWORD op;if(!VirtualProtect(addr,len,PAGE_EXECUTE_READWRITE,&op))return 0;memcpy(addr,data,len);VirtualProtect(addr,len,op,&op);FlushInstructionCache(GetCurrentProcess(),addr,len);return 1;}

/* ══ MOD 1: 4x Ball Size ══ */
#define P1_ADDR 0x00403C91
#define P1_ORIG "\x00\x00\xD8\x41"
#define P1_NEW  "\x00\x00\xD8\x42"
#define P2_ADDR 0x0041C8B0
#define P2_ORIG "\x00\x00\xD0\x41"
#define P2_NEW  "\x00\x00\xD0\x42"
#define P3_ADDR 0x0040BE74
#define P3_ORIG "\xD9\x9E\x84\x02\x00\x00"
#define P3_LEN  6

static unsigned char cave_tmpl[]={
    0xD9,0x9E,0x84,0x02,0x00,0x00,
    0xD9,0x86,0x84,0x02,0x00,0x00,
    0xD8,0x0D,0x00,0x00,0x00,0x00,
    0xD9,0x9E,0x84,0x02,0x00,0x00,
    0xC3,
    0x00,0x00,0x80,0x40
};
static int apply_4x(BYTE*base){
    int r=0;
    r+=patch_bytes(base+(P1_ADDR-IMAGE_BASE),(const BYTE*)P1_ORIG,(const BYTE*)P1_NEW,4);
    r+=patch_bytes(base+(P2_ADDR-IMAGE_BASE),(const BYTE*)P2_ORIG,(const BYTE*)P2_NEW,4);
    BYTE*p3=base+(P3_ADDR-IMAGE_BASE);
    if(memcmp(p3,P3_ORIG,P3_LEN)==0){
        SYSTEM_INFO si;GetSystemInfo(&si);
        SIZE_T as=((sizeof(cave_tmpl)+si.dwPageSize-1)/si.dwPageSize)*si.dwPageSize;
        void*cave=VirtualAlloc((void*)(IMAGE_BASE+0xF8000),as,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
        if(!cave)cave=VirtualAlloc(NULL,as,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
        if(cave){
            unsigned char buf[sizeof(cave_tmpl)];
            memcpy(buf,cave_tmpl,sizeof(cave_tmpl));
            DWORD ca=(DWORD)cave+25;
            memcpy(&buf[14],&ca,4);
            if(write_bytes((BYTE*)cave,buf,sizeof(cave_tmpl))){
                BYTE cn[6];cn[0]=0xE8;cn[5]=0x90;
                ptrdiff_t rel=(ptrdiff_t)cave-((ptrdiff_t)p3+5);
                if(rel<=0x7FFFFFFF&&rel>=(ptrdiff_t)0x80000000){
                    int32_t r32=(int32_t)rel;memcpy(&cn[1],&r32,4);
                    if(write_bytes(p3,cn,6))r++;
                }
            }
        }
    }
    return r;
}

/* ══ MOD 2: No Shatter (visual + sound skipped, respawn still works) ══
 *
 * Ball_Shatter (0x408D70): Contains the shatter animation + sound effect.
 *   __thiscall, 1 stack param → RET 0x4 skips the entire function.
 *   The shatter visual, particle effects, and sound never play.
 *
 * Ball_Shatter_OnRamp (0x409480): Shatter when hitting ramps at wrong angle.
 *   __thiscall, 0 stack params → RET skips the entire function.
 *
 * IMPORTANT: The respawn system (ball+0x2E8, +0x2E9, +0x768) is LEFT ALONE.
 * If the ball falls off an edge, it still respawns at the last checkpoint.
 * Only the shatter visual + sound are suppressed.
 */
static int apply_noshatter(BYTE*base){
    int r=0;
    r+=patch_bytes(base+(0x408D70-IMAGE_BASE),
        (const BYTE*)"\x6A\xFF\x64",(const BYTE*)"\xC2\x04\x00",3);
    r+=patch_bytes(base+(0x409480-IMAGE_BASE),
        (const BYTE*)"\x6A\xFF\x64",(const BYTE*)"\xC3\x90\x90",3);
    return r;
}

/* ══ DLL Entry ══ */
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved){
    switch(fdwReason){
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        {HMODULE hExe=GetModuleHandleA(NULL);if(hExe){BYTE*base=(BYTE*)hExe;apply_4x(base);apply_noshatter(base);}}
        break;
    }
    return TRUE;
}
