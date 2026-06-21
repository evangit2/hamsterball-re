/*
 * unlimited_tris_mod.c — BASS.dll proxy that removes D3D8 vertex buffer size limits.
 *
 * Allows custom MESHWORLD levels with arbitrarily many triangles to render
 * in Hamsterball by intercepting D3D8 CreateVertexBuffer calls and
 * falling back to system-memory buffers when GPU allocation fails.
 *
 * Also splits DrawPrimitiveUP calls that exceed D3D8's 65535 primitive limit.
 *
 * Installation (Windows):
 *   1. In your Hamsterball game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll unlimited_tris_mod.c \
 *     bass_exports.def -lwinmm -ld3d8 -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================
 * BASS Proxy - forward all audio calls to bass_real.dll
 * ====================================================================== */

typedef void (__stdcall *BASS_Stop_t)(void);
typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, float, int, int);
typedef int  (__stdcall *BASS_Free_t)(void);
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
typedef int  (__stdcall *BASS_Start_t)(void);
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, BOOL);
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);

static HMODULE g_hRealBass = NULL;
static BASS_Stop_t              real_BASS_Stop = NULL;
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
static BASS_Free_t              real_BASS_Free = NULL;
static BASS_Init_t              real_BASS_Init = NULL;
static BASS_Start_t             real_BASS_Start = NULL;
static BASS_SetConfig_t         real_BASS_SetConfig = NULL;
static BASS_ChannelStop_t       real_BASS_ChannelStop = NULL;
static BASS_MusicPlayEx_t       real_BASS_MusicPlayEx = NULL;
static BASS_ErrorGetCode_t      real_BASS_ErrorGetCode = NULL;
static BASS_MusicLoad_t         real_BASS_MusicLoad = NULL;

/* ======================================================================
 * D3D8 Hook - intercept CreateVertexBuffer and DrawPrimitiveUP
 * ====================================================================== */

/* Hooked IDirect3DDevice8 vtable */
static IDirect3DDevice8 *g_pRealDevice = NULL;
static DWORD g_dwRealVTable[119]; /* IDirect3DDevice8 has 119 methods */

/* Statistics */
static int g_nCreateVBCalls = 0;
static int g_nCreateVBRetries = 0;
static int g_nDrawPrimitives = 0;
static int g_nDrawSplitCalls = 0;

/* ---- Our hooked CreateVertexBuffer ---- */
typedef HRESULT (__stdcall *CreateVertexBuffer_t)(IDirect3DDevice8*, UINT, DWORD, DWORD, D3DPOOL, IDirect3DVertexBuffer8**);

static HRESULT __stdcall Hooked_CreateVertexBuffer(
    IDirect3DDevice8 *pDev,
    UINT Length,
    DWORD Usage,
    DWORD FVF,
    D3DPOOL Pool,
    IDirect3DVertexBuffer8 **ppVertexBuffer)
{
    CreateVertexBuffer_t RealCreateVB = (CreateVertexBuffer_t)g_dwRealVTable[27]; /* vtable slot 27 */

    g_nCreateVBCalls++;

    /* Try the original call first */
    HRESULT hr = RealCreateVB(pDev, Length, Usage, FVF, Pool, ppVertexBuffer);
    if (SUCCEEDED(hr)) return hr;

    /* If it failed and we were using D3DPOOL_DEFAULT, try D3DPOOL_SYSTEMMEM */
    if (Pool == D3DPOOL_DEFAULT) {
        g_nCreateVBRetries++;
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;

        /* Try D3DPOOL_MANAGED as last resort */
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_MANAGED, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* Try with no usage flags and system memory */
    if (Usage != 0) {
        g_nCreateVBRetries++;
        hr = RealCreateVB(pDev, Length, 0, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* All attempts failed */
    return hr;
}

/* ---- Our hooked DrawPrimitiveUP ---- */
typedef HRESULT (__stdcall *DrawPrimitiveUP_t)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, const void*, UINT);

#define MAX_PRIMITIVES_PER_CALL 65535

static HRESULT __stdcall Hooked_DrawPrimitiveUP(
    IDirect3DDevice8 *pDev,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT PrimitiveCount,
    const void *pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    DrawPrimitiveUP_t RealDrawUP = (DrawPrimitiveUP_t)g_dwRealVTable[49]; /* vtable slot 49 */

    g_nDrawPrimitives++;

    /* If within limits, just pass through */
    if (PrimitiveCount <= MAX_PRIMITIVES_PER_CALL) {
        return RealDrawUP(pDev, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    /* Split into chunks */
    g_nDrawSplitCalls++;
    HRESULT finalHr = D3D_OK;
    const BYTE *pData = (const BYTE *)pVertexStreamZeroData;
    UINT remaining = PrimitiveCount;
    UINT verticesPerPrim = 3; /* D3DPT_TRIANGLELIST */
    UINT offset = 0;

    while (remaining > 0) {
        UINT chunk = (remaining > MAX_PRIMITIVES_PER_CALL) ? MAX_PRIMITIVES_PER_CALL : remaining;
        HRESULT hr = RealDrawUP(pDev, PrimitiveType, chunk,
                                pData + offset * verticesPerPrim * VertexStreamZeroStride,
                                VertexStreamZeroStride);
        if (FAILED(hr)) finalHr = hr;
        offset += chunk * verticesPerPrim;
        remaining -= chunk;
    }

    return finalHr;
}

/* ---- Our hooked DrawIndexedPrimitiveUP ---- */
typedef HRESULT (__stdcall *DrawIndexedPrimitiveUP_t)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT, UINT, const void*, D3DFORMAT, const void*, UINT);

static HRESULT __stdcall Hooked_DrawIndexedPrimitiveUP(
    IDirect3DDevice8 *pDev,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT MinVertexIndex,
    UINT NumVertexIndices,
    UINT PrimitiveCount,
    const void *pIndexData,
    D3DFORMAT IndexDataFormat,
    const void *pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    DrawIndexedPrimitiveUP_t RealDrawIdxUP = (DrawIndexedPrimitiveUP_t)g_dwRealVTable[51]; /* vtable slot 51 */

    /* If within limits, pass through */
    if (PrimitiveCount <= MAX_PRIMITIVES_PER_CALL) {
        return RealDrawIdxUP(pDev, PrimitiveType, MinVertexIndex, NumVertexIndices,
                            PrimitiveCount, pIndexData, IndexDataFormat,
                            pVertexStreamZeroData, VertexStreamZeroStride);
    }

    /* For indexed draws, we need to split carefully based on index data */
    /* For now, just try the call and let D3D handle it */
    return RealDrawIdxUP(pDev, PrimitiveType, MinVertexIndex, NumVertexIndices,
                        PrimitiveCount, pIndexData, IndexDataFormat,
                        pVertexStreamZeroData, VertexStreamZeroStride);
}

/* ---- Hook the D3D8 device vtable ---- */
static void HookD3D8Device(IDirect3DDevice8 *pDev)
{
    if (!pDev || g_pRealDevice) return;

    g_pRealDevice = pDev;

    /* Get the vtable pointer */
    DWORD *pVTable = *(DWORD **)pDev;
    if (!pVTable) return;

    /* Save original vtable entries */
    memcpy(g_dwRealVTable, pVTable, sizeof(g_dwRealVTable));

    /* Make vtable writable */
    DWORD oldProtect;
    if (!VirtualProtect(pVTable, sizeof(g_dwRealVTable), PAGE_READWRITE, &oldProtect))
        return;

    /* Hook CreateVertexBuffer (vtable slot 27) */
    pVTable[27] = (DWORD)Hooked_CreateVertexBuffer;

    /* Hook DrawPrimitiveUP (vtable slot 49) */
    pVTable[49] = (DWORD)Hooked_DrawPrimitiveUP;

    /* Hook DrawIndexedPrimitiveUP (vtable slot 51) */
    pVTable[51] = (DWORD)Hooked_DrawIndexedPrimitiveUP;

    VirtualProtect(pVTable, sizeof(g_dwRealVTable), oldProtect, &oldProtect);
}

/* ---- Hook Direct3DCreate8 to intercept device creation ---- */
typedef IDirect3D8* (__stdcall *Direct3DCreate8_t)(UINT);

static IDirect3D8 *g_pD3D8 = NULL;
static DWORD g_dwD3D8VTable[19]; /* IDirect3D8 has 19 methods */

typedef HRESULT (__stdcall *CreateDevice_t)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);

static HRESULT __stdcall Hooked_CreateDevice(
    IDirect3D8 *pD3D,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice8 **ppReturnedDeviceInterface)
{
    CreateDevice_t RealCreateDevice = (CreateDevice_t)g_dwD3D8VTable[15]; /* vtable slot 15 */

    /* Try with software vertex processing if hardware fails */
    HRESULT hr = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow,
                                   BehaviorFlags, pPresentationParameters,
                                   ppReturnedDeviceInterface);
    if (FAILED(hr) && (BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)) {
        /* Retry with software vertex processing */
        hr = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow,
                              D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                              pPresentationParameters, ppReturnedDeviceInterface);
    }

    if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
        HookD3D8Device(*ppReturnedDeviceInterface);
    }

    return hr;
}

/* Patch Direct3DCreate8 import in the game's IAT */
static void HookD3D8Import(void)
{
    HMODULE hExe = GetModuleHandleA(NULL);
    if (!hExe) return;

    /* Walk the import table to find Direct3DCreate8 */
    BYTE *base = (BYTE *)hExe;
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return;

    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return;

    DWORD importRVA = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if (!importRVA) return;

    IMAGE_IMPORT_DESCRIPTOR *imp = (IMAGE_IMPORT_DESCRIPTOR *)(base + importRVA);
    while (imp->Name) {
        const char *dllName = (const char *)(base + imp->Name);
        if (_stricmp(dllName, "d3d8.dll") == 0) {
            /* Found d3d8.dll import - walk the thunks */
            DWORD *pIAT = (DWORD *)(base + imp->FirstThunk);
            DWORD *pINT = (DWORD *)(base + imp->OriginalFirstThunk);
            int idx = 0;

            while (pIAT[idx]) {
                /* Check if this is a named import */
                if (pINT && (pINT[idx] & IMAGE_ORDINAL_FLAG) == 0) {
                    IMAGE_IMPORT_BY_NAME *name = (IMAGE_IMPORT_BY_NAME *)(base + pINT[idx]);
                    if (strcmp((const char *)name->Name, "Direct3DCreate8") == 0) {
                        /* Found it! Replace with our version */
                        Direct3DCreate8_t realD3DCreate8 = (Direct3DCreate8_t)pIAT[idx];

                        /* Store the real function pointer */
                        HMODULE hD3D8 = LoadLibraryA("d3d8.dll");
                        if (hD3D8) {
                            realD3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hD3D8, "Direct3DCreate8");
                        }

                        /* We need a global pointer to the real function */
                        static Direct3DCreate8_t g_realD3DCreate8 = NULL;
                        g_realD3DCreate8 = realD3DCreate8;

                        /* Patch IAT to point to our hook */
                        DWORD oldProtect;
                        if (VirtualProtect(&pIAT[idx], 4, PAGE_READWRITE, &oldProtect)) {
                            /* We can't easily replace a single function here
                             * because Direct3DCreate8 returns an interface,
                             * not a device. Instead, we'll use a different
                             * approach: hook the device after creation. */
                            VirtualProtect(&pIAT[idx], 4, oldProtect, &oldProtect);
                        }
                        break;
                    }
                }
                idx++;
            }
            break;
        }
        imp++;
    }
}

/* ======================================================================
 * Background Thread - applies hooks after game initialization
 * ====================================================================== */

static DWORD WINAPI HookThread(LPVOID lpParam)
{
    (void)lpParam;

    char log_path[MAX_PATH];
    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    char *p = strrchr(log_path, '.');
    if (p) strcpy(p, "_unlimited_tris.log");
    else strcat(log_path, "_unlimited_tris.log");

    FILE *f = fopen(log_path, "w");
    if (!f) f = stdout;

    fprintf(f, "Hamsterball Unlimited Tris Mod (bass.dll proxy)\n");
    fprintf(f, "=================================================\n");
    fprintf(f, "Waiting for D3D8 device creation...\n");

    /* Wait for the game to create its D3D8 device */
    Sleep(3000);

    /* Try to find the D3D8 device by looking for the d3d8.dll module */
    HMODULE hD3D8 = GetModuleHandleA("d3d8.dll");
    if (hD3D8) {
        fprintf(f, "d3d8.dll loaded at 0x%p\n", hD3D8);

        /* Find Direct3DCreate8 */
        typedef IDirect3D8* (__stdcall *D3DCreate8_fn)(UINT);
        D3DCreate8_fn pCreate8 = (D3DCreate8_fn)GetProcAddress(hD3D8, "Direct3DCreate8");
        if (pCreate8) {
            /* Create a temporary D3D8 interface to get the device */
            IDirect3D8 *pD3D = pCreate8(120); /* D3D_SDK_VERSION */
            if (pD3D) {
                DWORD *pVTable = *(DWORD **)pD3D;
                if (pVTable) {
                    memcpy(g_dwD3D8VTable, pVTable, sizeof(g_dwD3D8VTable));

                    /* Hook CreateDevice in the D3D8 interface */
                    DWORD oldProtect;
                    if (VirtualProtect(pVTable, sizeof(g_dwD3D8VTable), PAGE_READWRITE, &oldProtect)) {
                        pVTable[15] = (DWORD)Hooked_CreateDevice;
                        VirtualProtect(pVTable, sizeof(g_dwD3D8VTable), oldProtect, &oldProtect);
                        fprintf(f, "Hooked IDirect3D8::CreateDevice\n");
                    }
                }
                /* Release the temporary D3D8 interface */
                DWORD *pD3DVT = *(DWORD **)pD3D;
                typedef ULONG (__stdcall *Fn_Release)(IDirect3D8*);
                Fn_Release pRelease = (Fn_Release)pD3DVT[2]; /* vtable slot 2 = Release */
                pRelease(pD3D);
            }
        }
    } else {
        fprintf(f, "WARNING: d3d8.dll not loaded yet\n");
    }

    /* Also try IAT patching */
    HookD3D8Import();

    fprintf(f, "Hook setup complete. Monitoring D3D8 calls...\n");
    fprintf(f, "\n");

    /* Periodically log stats */
    while (1) {
        Sleep(10000);
        if (g_nCreateVBCalls > 0 || g_nDrawPrimitives > 0) {
            fprintf(f, "[Stats] CreateVB: %d calls, %d retries | DrawPrim: %d calls, %d splits\n",
                    g_nCreateVBCalls, g_nCreateVBRetries, g_nDrawPrimitives, g_nDrawSplitCalls);
            fflush(f);
        }
    }

    if (f != stdout) fclose(f);
    return 0;
}

/* ======================================================================
 * BASS Proxy Exports (__stdcall)
 * ====================================================================== */

__declspec(dllexport) void __stdcall BASS_Stop(void)
{ if (real_BASS_Stop) real_BASS_Stop(); }

__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD handle, float freq, int vol, int pan)
{ if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(handle, freq, vol, pan); return 0; }

__declspec(dllexport) int __stdcall BASS_Free(void)
{ if (real_BASS_Free) return real_BASS_Free(); return 0; }

__declspec(dllexport) int __stdcall BASS_Init(int device, DWORD freq, DWORD flags, HWND win, void *clsid)
{ if (real_BASS_Init) return real_BASS_Init(device, freq, flags, win, clsid); return 0; }

__declspec(dllexport) int __stdcall BASS_Start(void)
{ if (real_BASS_Start) return real_BASS_Start(); return 0; }

__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD option, DWORD value)
{ if (real_BASS_SetConfig) return real_BASS_SetConfig(option, value); return 0; }

__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD handle)
{ if (real_BASS_ChannelStop) return real_BASS_ChannelStop(handle); return 0; }

__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD handle, DWORD pos, BOOL seek)
{ if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(handle, pos, seek); return 0; }

__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void)
{ if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode(); return 0; }

__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int mem, void *file, DWORD offset, DWORD length, DWORD flags, DWORD freq)
{ if (real_BASS_MusicLoad) return real_BASS_MusicLoad(mem, file, offset, length, flags, freq); return 0; }

/* ======================================================================
 * DllMain
 * ====================================================================== */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        /* Load the real BASS library */
        g_hRealBass = LoadLibraryA("bass_real.dll");
        if (g_hRealBass == NULL) {
            char path[MAX_PATH];
            if (GetModuleFileNameA(hinstDLL, path, MAX_PATH)) {
                char *p = strrchr(path, '\\');
                if (p) { strcpy(p + 1, "bass_real.dll"); g_hRealBass = LoadLibraryA(path); }
            }
        }

        if (g_hRealBass != NULL) {
            real_BASS_Stop              = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
            real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
            real_BASS_Free              = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
            real_BASS_Init              = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
            real_BASS_Start             = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
            real_BASS_SetConfig         = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
            real_BASS_ChannelStop       = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
            real_BASS_MusicPlayEx       = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
            real_BASS_ErrorGetCode      = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
            real_BASS_MusicLoad         = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
        }

        /* Start the hook thread */
        CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_hRealBass) { FreeLibrary(g_hRealBass); g_hRealBass = NULL; }
        break;
    }
    return TRUE;
}
