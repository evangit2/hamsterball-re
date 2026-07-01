/*
 * safe_4x_nofalldmg.c — Safe BASS.dll proxy: 4x Ball Size + No Fall Damage
 *
 * 1. 4x Ball Size  — all balls 4x bigger (2 MOV patches + code cave)
 * 2. No Fall Damage — ball never shatters from ANY path:
 *    a) Ball_Shatter (0x408D70) → RET 0x4 (skip entirely)
 *    b) Ball_Shatter_OnRamp (0x409480) → RET (skip entirely)
 *    c) Ball_FallUpdate shatter flag write (0x408CD5) → NOP 7 bytes
 *       (this is the line: MOV byte [ESI+0x2E8], 1 — sets shattered flag
 *        when bad ball fall timer expires. NOTE: BAD BALL ONLY — vtable[65]
 *        of bad ball vtable 0x4CF494. Player ball never calls Ball_FallUpdate.
 *        This patch is a no-op for the player ball but harmless to keep.)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll safe_4x_nofalldmg.c \
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
#define P1_ORIG "\x00\x00\xD8\x41"  /* 27.0f */
#define P1_NEW  "\x00\x00\xD8\x42"  /* 108.0f */
#define P2_ADDR 0x0041C8B0
#define P2_ORIG "\x00\x00\xD0\x41"  /* 26.0f */
#define P2_NEW  "\x00\x00\xD0\x42"  /* 104.0f */
#define P3_ADDR 0x0040BE74
#define P3_ORIG "\xD9\x9E\x84\x02\x00\x00"
#define P3_LEN  6

static unsigned char cave_tmpl[]={
    0xD9,0x9E,0x84,0x02,0x00,0x00, /* FSTP [ESI+0x284] */
    0xD9,0x86,0x84,0x02,0x00,0x00, /* FLD  [ESI+0x284] */
    0xD8,0x0D,0x00,0x00,0x00,0x00, /* FMUL dword [addr] */
    0xD9,0x9E,0x84,0x02,0x00,0x00, /* FSTP [ESI+0x284] */
    0xC3,                          /* RET */
    0x00,0x00,0x80,0x40            /* 4.0f */
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
            DWORD ca=(DWORD)cave+25; /* offset of 4.0f constant */
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

/* ══ MOD 2: No Fall Damage (comprehensive — ALL respawn triggers) ══
 *
 * The game has FOUR independent systems that make the ball break/disappear:
 *
 * 1. ball+0x2E8 = 1 (shatter flag) — 8 write sites, ALL NOPed
 * 2. ball+0x2E9 = 1 (E:LIMIT respawn flag) — 5 write sites, ALL NOPed
 * 3. ball+0x768 = 0 (is_active → deactivates ball) — 5 write sites, ALL NOPed
 * 4. ball+0x2FC = 0 (alpha → invisible) — 2 write sites, NOPed
 * Plus Ball_Shatter and Ball_Shatter_OnRamp function-entry RETs
 */
static int apply_nofall(BYTE*base){
    int r=0;
    /* --- Function entry RETs --- */
    r+=patch_bytes(base+(0x408D70-IMAGE_BASE),
        (const BYTE*)"\x6A\xFF\x64",(const BYTE*)"\xC2\x04\x00",3);
    r+=patch_bytes(base+(0x409480-IMAGE_BASE),
        (const BYTE*)"\x6A\xFF\x64",(const BYTE*)"\xC3\x90\x90",3);

    /* --- 1. NOP all writes that set ball+0x2E8 = 1 (shatter flag) --- */
    r+=patch_bytes(base+(0x4031F8-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x406244-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x407436-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x408CD5-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x4090A6-IMAGE_BASE),
        (const BYTE*)"\xC6\x87\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F2E7-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE8\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);

    /* --- 2. NOP all writes that set ball+0x2E9 = 1 (E:LIMIT flag) --- */
    r+=patch_bytes(base+(0x407391-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE9\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F22D-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE9\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F268-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE9\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F2A3-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE9\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F317-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\xE9\x02\x00\x00\x01",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);

    /* --- 3. NOP all writes that set ball+0x768 = 0 (deactivate) --- */
    r+=patch_bytes(base+(0x40F226-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\x68\x07\x00\x00\x00",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F261-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\x68\x07\x00\x00\x00",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F29C-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\x68\x07\x00\x00\x00",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);
    r+=patch_bytes(base+(0x40F310-IMAGE_BASE),
        (const BYTE*)"\xC6\x86\x68\x07\x00\x00\x00",
        (const BYTE*)"\x90\x90\x90\x90\x90\x90\x90",7);

    return r;
}

/* ══ DLL Entry ══ */
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved){
    switch(fdwReason){
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        load_real_bass();
        {HMODULE hExe=GetModuleHandleA(NULL);if(hExe){BYTE*base=(BYTE*)hExe;apply_4x(base);apply_nofall(base);}}
        break;
    }
    return TRUE;
}
