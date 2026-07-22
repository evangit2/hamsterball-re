/*
 * mesh_xor_decrypt — bass.dll proxy mod
 *
 * XOR-decrypts .MESH and .MESHWORLD files on the fly.
 * Key: 119 (0x77)
 *
 * How it works:
 *   1. Hooks CreateFileA via IAT to detect .mesh/.meshworld files
 *   2. Tracks those file handles in a table
 *   3. Hooks ReadFile via IAT to XOR-decrypt buffers after reading
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll mesh_xor_decrypt.c \
 *     bass_real_loader.c -lwinmm -Wl,--enable-stdcall-fixup -O2 -static \
 *     -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>

#define XOR_KEY 119  /* 0x77 */

/* ===== BASS proxy stubs (minimal — no audio forwarding needed) ===== */
/* The game imports 10 BASS functions. We stub them. BASS_Init returns FALSE
 * so the game knows audio isn't available (graceful degradation). */

typedef int (__stdcall *BASS_func_t)(void);

static HMODULE g_bass_real = NULL;

/* BASS function pointer types */
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Free_t)(DWORD);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD(__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_ChannelSetAttribute_t)(DWORD, DWORD, float);

static BASS_Init_t                real_BASS_Init = NULL;
static BASS_Free_t                real_BASS_Free = NULL;
static BASS_Start_t               real_BASS_Start = NULL;
static BASS_Stop_t                real_BASS_Stop = NULL;
static BASS_SetConfig_t           real_BASS_SetConfig = NULL;
static BASS_ErrorGetCode_t        real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t           real_BASS_MusicLoad = NULL;
static BASS_MusicPlayEx_t         real_BASS_MusicPlayEx = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_ChannelStop_t         real_BASS_ChannelStop = NULL;
static BASS_ChannelSetAttribute_t real_BASS_ChannelSetAttribute = NULL;

static void load_real_bass(void) {
    if (g_bass_real) return;
    /* Use a flag to avoid repeated LoadLibrary attempts */
    static int tried = 0;
    if (tried) return;
    tried = 1;
    g_bass_real = LoadLibraryA("bass_real.dll");
    if (!g_bass_real) return;
    real_BASS_Init = (BASS_Init_t)GetProcAddress(g_bass_real, "BASS_Init");
    real_BASS_Free = (BASS_Free_t)GetProcAddress(g_bass_real, "BASS_Free");
    real_BASS_Start = (BASS_Start_t)GetProcAddress(g_bass_real, "BASS_Start");
    real_BASS_Stop = (BASS_Stop_t)GetProcAddress(g_bass_real, "BASS_Stop");
    real_BASS_SetConfig = (BASS_SetConfig_t)GetProcAddress(g_bass_real, "BASS_SetConfig");
    real_BASS_ErrorGetCode = (BASS_ErrorGetCode_t)GetProcAddress(g_bass_real, "BASS_ErrorGetCode");
    real_BASS_MusicLoad = (BASS_MusicLoad_t)GetProcAddress(g_bass_real, "BASS_MusicLoad");
    real_BASS_MusicPlayEx = (BASS_MusicPlayEx_t)GetProcAddress(g_bass_real, "BASS_MusicPlayEx");
    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_bass_real, "BASS_ChannelSetAttributes");
    real_BASS_ChannelStop = (BASS_ChannelStop_t)GetProcAddress(g_bass_real, "BASS_ChannelStop");
    real_BASS_ChannelSetAttribute = (BASS_ChannelSetAttribute_t)GetProcAddress(g_bass_real, "BASS_ChannelSetAttribute");
}

/* BASS export stubs — lazy-load bass_real.dll on first call */
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void *e) {
    if (!real_BASS_Init) load_real_bass();
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 0; /* FALSE — no audio */
}
__declspec(dllexport) int __stdcall BASS_Free(DWORD a) {
    if (!real_BASS_Free) load_real_bass();
    if (real_BASS_Free) return real_BASS_Free(a);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (!real_BASS_Start) load_real_bass();
    if (real_BASS_Start) return real_BASS_Start();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (!real_BASS_Stop) load_real_bass();
    if (real_BASS_Stop) return real_BASS_Stop();
    return 0;
}
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (!real_BASS_SetConfig) load_real_bass();
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (!real_BASS_ErrorGetCode) load_real_bass();
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void *b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (!real_BASS_MusicLoad) load_real_bass();
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, DWORD c, DWORD d) {
    if (!real_BASS_MusicPlayEx) load_real_bass();
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
    return 0;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (!real_BASS_ChannelSetAttributes) load_real_bass();
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (!real_BASS_ChannelStop) load_real_bass();
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}
__declspec(dllexport) int __stdcall BASS_ChannelSetAttribute(DWORD a, DWORD b, float c) {
    if (!real_BASS_ChannelSetAttribute) load_real_bass();
    if (real_BASS_ChannelSetAttribute) return real_BASS_ChannelSetAttribute(a, b, c);
    return 1;
}


/* ===== File handle tracking for XOR decryption ===== */

#define MAX_TRACKED_HANDLES 64

static HANDLE g_tracked_handles[MAX_TRACKED_HANDLES];
static int g_handle_count = 0;
static CRITICAL_SECTION g_handle_lock;

static int is_mesh_file(const char *filename) {
    if (!filename) return 0;
    /* Case-insensitive check for .mesh or .meshworld extension */
    const char *p = filename;
    while (*p) {
        if (p[0] == '.' && 
            (p[1] == 'm' || p[1] == 'M') &&
            (p[2] == 'e' || p[2] == 'E') &&
            (p[3] == 's' || p[3] == 'S') &&
            (p[4] == 'h' || p[4] == 'H')) {
            /* Could be .mesh or .meshworld — check further */
            if (p[5] == '\0') return 1;  /* .mesh */
            if ((p[5] == 'w' || p[5] == 'W') &&
                (p[6] == 'o' || p[6] == 'O') &&
                (p[7] == 'r' || p[7] == 'R') &&
                (p[8] == 'l' || p[8] == 'L') &&
                (p[9] == 'd' || p[9] == 'D') &&
                p[10] == '\0') return 1;  /* .meshworld */
            return 1; /* .mesh* — accept it */
        }
        p++;
    }
    return 0;
}

static void track_handle(HANDLE h) {
    EnterCriticalSection(&g_handle_lock);
    if (g_handle_count < MAX_TRACKED_HANDLES) {
        g_tracked_handles[g_handle_count++] = h;
    }
    LeaveCriticalSection(&g_handle_lock);
}

static int is_tracked(HANDLE h) {
    int found = 0;
    EnterCriticalSection(&g_handle_lock);
    for (int i = 0; i < g_handle_count; i++) {
        if (g_tracked_handles[i] == h) {
            found = 1;
            break;
        }
    }
    LeaveCriticalSection(&g_handle_lock);
    return found;
}

static void untrack_handle(HANDLE h) {
    EnterCriticalSection(&g_handle_lock);
    for (int i = 0; i < g_handle_count; i++) {
        if (g_tracked_handles[i] == h) {
            g_tracked_handles[i] = g_tracked_handles[--g_handle_count];
            break;
        }
    }
    LeaveCriticalSection(&g_handle_lock);
}


/* ===== IAT hooks for CreateFileA and ReadFile ===== */

static HANDLE (WINAPI *real_CreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
                                          DWORD, DWORD, HANDLE) = NULL;
static BOOL (WINAPI *real_ReadFile)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED) = NULL;
static BOOL (WINAPI *real_CloseHandle)(HANDLE) = NULL;

static HANDLE WINAPI hook_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess,
    DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecAttr, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    HANDLE h = real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
                                lpSecAttr, dwCreationDisposition,
                                dwFlagsAndAttributes, hTemplateFile);
    
    if (h != INVALID_HANDLE_VALUE && is_mesh_file(lpFileName)) {
        track_handle(h);
    }
    return h;
}

static BOOL WINAPI hook_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    BOOL result = real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,
                                lpNumberOfBytesRead, lpOverlapped);
    
    if (result && lpNumberOfBytesRead && *lpNumberOfBytesRead > 0) {
        if (is_tracked(hFile)) {
            /* XOR-decrypt the buffer */
            BYTE *buf = (BYTE *)lpBuffer;
            DWORD count = *lpNumberOfBytesRead;
            for (DWORD i = 0; i < count; i++) {
                buf[i] ^= XOR_KEY;
            }
        }
    }
    return result;
}

static BOOL WINAPI hook_CloseHandle(HANDLE hObject) {
    untrack_handle(hObject);
    return real_CloseHandle(hObject);
}


/* ===== IAT hooking machinery ===== */

static void patch_iat(HMODULE target_module, const char *dll_name,
                      const char *func_name, void *new_func, void **old_func)
{
    if (!target_module || !dll_name || !func_name || !new_func) return;
    
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)target_module;
    if (IsBadReadPtr(dos, sizeof(*dos))) return;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;
    
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE *)target_module + dos->e_lfanew);
    if (IsBadReadPtr(nt, sizeof(*nt))) return;
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;
    
    DWORD import_rva = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    DWORD import_size = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
    if (import_rva == 0 || import_size == 0) return;
    
    PIMAGE_IMPORT_DESCRIPTOR imp = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)target_module + import_rva);
    if (IsBadReadPtr(imp, sizeof(*imp))) return;
    
    while (imp->Name) {
        const char *name = (const char *)((BYTE *)target_module + imp->Name);
        if (IsBadReadPtr(name, 1)) { imp++; continue; }
        
        if (_stricmp(name, dll_name) == 0) {
            if (imp->FirstThunk == 0) { imp++; continue; }
            PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((BYTE *)target_module + imp->FirstThunk);
            if (IsBadReadPtr(thunk, sizeof(*thunk))) { imp++; continue; }
            
            while (thunk->u1.Function) {
                if (!(thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
                    PIMAGE_IMPORT_BY_NAME imp_name = (PIMAGE_IMPORT_BY_NAME)
                        ((BYTE *)target_module + thunk->u1.AddressOfData);
                    if (!IsBadReadPtr(imp_name, sizeof(*imp_name)) && imp_name->Name) {
                        if (_stricmp((const char *)imp_name->Name, func_name) == 0) {
                            DWORD old_protect;
                            if (VirtualProtect(&thunk->u1.Function, sizeof(void *),
                                          PAGE_READWRITE, &old_protect)) {
                                if (old_func && !*old_func)
                                    *old_func = (void *)thunk->u1.Function;
                                thunk->u1.Function = (DWORD)new_func;
                                VirtualProtect(&thunk->u1.Function, sizeof(void *),
                                              old_protect, &old_protect);
                            }
                            return;
                        }
                    }
                }
                thunk++;
                if (IsBadReadPtr(thunk, sizeof(*thunk))) break;
            }
        }
        imp++;
        if (IsBadReadPtr(imp, sizeof(*imp))) break;
    }
}


/* ===== DLL entry point ===== */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_handle_lock);
        memset(g_tracked_handles, 0, sizeof(g_tracked_handles));
        
        /* DO NOT call load_real_bass() here — LoadLibrary in DllMain = loader lock deadlock */
        /* Audio will be lazily loaded on first BASS call */
        
        /* Patch IAT DIRECTLY in DllMain — runs before WinMain, no race condition. */
        HMODULE exe = GetModuleHandleA(NULL);
        patch_iat(exe, "kernel32.dll", "CreateFileA", hook_CreateFileA, (void **)&real_CreateFileA);
        patch_iat(exe, "kernel32.dll", "ReadFile",   hook_ReadFile,   (void **)&real_ReadFile);
        patch_iat(exe, "kernel32.dll", "CloseHandle",hook_CloseHandle,(void **)&real_CloseHandle);
    }
    return TRUE;
}
