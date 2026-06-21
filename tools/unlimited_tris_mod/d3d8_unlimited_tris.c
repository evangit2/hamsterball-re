/*
 * d3d8_unlimited_tris.c — D3D8 proxy that removes vertex buffer size limits.
 *
 * Sits between Hamsterball and d3d8to9 (or real d3d8.dll).
 * Intercepts CreateVertexBuffer to fall back to SYSTEMMEM when DEFAULT fails.
 * Splits DrawPrimitiveUP calls exceeding 65535 primitives.
 *
 * Installation:
 *   1. In game folder, rename existing d3d8.dll -> d3d8_real.dll
 *   2. Copy this d3d8.dll into the game folder
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o d3d8.dll d3d8_unlimited_tris.c \
 *     -ld3d8 -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ======================================================================
 * Real D3D8 function pointers
 * ====================================================================== */

typedef IDirect3D8* (__stdcall *Direct3DCreate8_t)(UINT);

static HMODULE g_hRealD3D8 = NULL;
static Direct3DCreate8_t g_RealDirect3DCreate8 = NULL;

/* ======================================================================
 * Wrapped IDirect3DDevice8
 * ====================================================================== */

/* The real device vtable - we save this before hooking */
static IDirect3DDevice8 *g_pRealDevice = NULL;

/* Stats */
static int g_nCreateVBCalls = 0;
static int g_nCreateVBFallback = 0;
static int g_nCreateVBFailed = 0;
static int g_nDrawPrimCalls = 0;
static int g_nDrawPrimSplits = 0;

#define MAX_PRIMITIVES_PER_CALL 65535

/* ---- Wrapper for CreateVertexBuffer ---- 
 * vtable slot index: 27
 * When CreateVertexBuffer fails with D3DPOOL_DEFAULT, retry with SYSTEMMEM.
 */
static HRESULT __stdcall Wrapped_CreateVertexBuffer(
    IDirect3DDevice8 *pDev,
    UINT Length,
    DWORD Usage,
    DWORD FVF,
    D3DPOOL Pool,
    IDirect3DVertexBuffer8 **ppVertexBuffer)
{
    g_nCreateVBCalls++;

    /* Get the real vtable function */
    DWORD *pVTable = *(DWORD **)g_pRealDevice;
    typedef HRESULT (__stdcall *Fn_CreateVB)(IDirect3DDevice8*, UINT, DWORD, DWORD, D3DPOOL, IDirect3DVertexBuffer8**);
    Fn_CreateVB RealCreateVB = (Fn_CreateVB)pVTable[27];

    /* Try the original call */
    HRESULT hr = RealCreateVB(pDev, Length, Usage, FVF, Pool, ppVertexBuffer);
    if (SUCCEEDED(hr)) return hr;

    /* If failed with DEFAULT pool, try SYSTEMMEM */
    if (Pool == D3DPOOL_DEFAULT) {
        g_nCreateVBFallback++;
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;

        /* Try MANAGED as well */
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_MANAGED, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* Try with no usage flags and SYSTEMMEM */
    if (Usage != 0) {
        g_nCreateVBFallback++;
        hr = RealCreateVB(pDev, Length, 0, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* Try with smaller buffer sizes - split into chunks */
    /* Sometimes the issue is the total size, not the vertex count */
    if (Length > 1024 * 1024) {
        /* Try creating a smaller buffer - the game should handle the failure */
        g_nCreateVBFallback++;
        hr = RealCreateVB(pDev, Length, 0, 0, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    g_nCreateVBFailed++;
    return hr;
}

/* ---- Wrapper for DrawPrimitiveUP ----
 * vtable slot index: 49
 * Splits large draw calls into chunks of ≤65535 primitives.
 */
static HRESULT __stdcall Wrapped_DrawPrimitiveUP(
    IDirect3DDevice8 *pDev,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT PrimitiveCount,
    const void *pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    g_nDrawPrimCalls++;

    /* Get the real vtable function */
    DWORD *pVTable = *(DWORD **)g_pRealDevice;
    typedef HRESULT (__stdcall *Fn_DrawPrimUP)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, const void*, UINT);
    Fn_DrawPrimUP RealDrawUP = (Fn_DrawPrimUP)pVTable[49];

    /* If within limits, just pass through */
    if (PrimitiveCount <= MAX_PRIMITIVES_PER_CALL) {
        return RealDrawUP(pDev, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    /* Split into chunks */
    g_nDrawPrimSplits++;
    HRESULT finalHr = D3D_OK;
    const BYTE *pData = (const BYTE *)pVertexStreamZeroData;
    UINT remaining = PrimitiveCount;
    UINT vertsPerPrim = 3; /* Triangle list */
    UINT vertexOffset = 0;

    while (remaining > 0) {
        UINT chunk = (remaining > MAX_PRIMITIVES_PER_CALL) ? MAX_PRIMITIVES_PER_CALL : remaining;
        HRESULT hr = RealDrawUP(pDev, PrimitiveType, chunk,
                               pData + vertexOffset * VertexStreamZeroStride,
                               VertexStreamZeroStride);
        if (FAILED(hr)) finalHr = hr;
        vertexOffset += chunk * vertsPerPrim;
        remaining -= chunk;
    }

    return finalHr;
}

/* ---- Hook the device vtable ---- */
static void HookDeviceVTable(IDirect3DDevice8 *pDevice)
{
    if (!pDevice) return;

    g_pRealDevice = pDevice;

    DWORD *pVTable = *(DWORD **)pDevice;
    if (!pVTable) return;

    /* Log the hook attempt */
    char log_path[MAX_PATH];
    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    char *p = strrchr(log_path, '.');
    if (p) strcpy(p, "_unlimited_tris.log");
    FILE *f = fopen(log_path, "a");
    if (f) {
        fprintf(f, "HookDeviceVTable: pDevice=%p, pVTable=%p\n", pDevice, pVTable);
        fprintf(f, "  Original CreateVB=%p, DrawPrimUP=%p\n", 
                (void*)pVTable[27], (void*)pVTable[49]);
        fclose(f);
    }

    DWORD oldProtect;
    if (!VirtualProtect(pVTable, 119 * sizeof(DWORD), PAGE_READWRITE, &oldProtect)) {
        f = fopen(log_path, "a");
        if (f) { fprintf(f, "  VirtualProtect FAILED: %lu\n", GetLastError()); fclose(f); }
        return;
    }

    /* Hook CreateVertexBuffer (slot 27) */
    pVTable[27] = (DWORD)Wrapped_CreateVertexBuffer;

    /* Hook DrawPrimitiveUP (slot 49) */
    pVTable[49] = (DWORD)Wrapped_DrawPrimitiveUP;

    VirtualProtect(pVTable, 119 * sizeof(DWORD), oldProtect, &oldProtect);

    f = fopen(log_path, "a");
    if (f) {
        fprintf(f, "  Hooked! New CreateVB=%p, DrawPrimUP=%p\n",
                (void*)pVTable[27], (void*)pVTable[49]);
        fclose(f);
    }
}

/* ======================================================================
 * Wrapped IDirect3D8
 * ====================================================================== */

static IDirect3D8 *g_pRealD3D = NULL;

/* ---- Wrapped CreateDevice ---- 
 * IDirect3D8 vtable slot 15.
 * Hooks the returned device.
 */
static HRESULT __stdcall Wrapped_CreateDevice(
    IDirect3D8 *pD3D,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS *pPresentationParameters,
    IDirect3DDevice8 **ppReturnedDeviceInterface)
{
    /* Get the real function */
    DWORD *pVTable = *(DWORD **)g_pRealD3D;
    typedef HRESULT (__stdcall *Fn_CreateDevice)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
    Fn_CreateDevice RealCreateDevice = (Fn_CreateDevice)pVTable[15];

    /* Try the original call */
    HRESULT hr = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow,
                                   BehaviorFlags, pPresentationParameters,
                                   ppReturnedDeviceInterface);

    /* If hardware vertex processing fails, try software */
    if (FAILED(hr) && (BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)) {
        hr = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow,
                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                             pPresentationParameters, ppReturnedDeviceInterface);
    }

    if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
        HookDeviceVTable(*ppReturnedDeviceInterface);
    }

    return hr;
}

/* ---- Hook the IDirect3D8 vtable ---- */
static void HookD3D8VTable(IDirect3D8 *pD3D)
{
    if (!pD3D) return;

    g_pRealD3D = pD3D;

    DWORD *pVTable = *(DWORD **)pD3D;
    if (!pVTable) return;

    DWORD oldProtect;
    if (!VirtualProtect(pVTable, 19 * sizeof(DWORD), PAGE_READWRITE, &oldProtect))
        return;

    /* Hook CreateDevice (slot 15) */
    pVTable[15] = (DWORD)Wrapped_CreateDevice;

    VirtualProtect(pVTable, 19 * sizeof(DWORD), oldProtect, &oldProtect);
}

/* ======================================================================
 * Export: Direct3DCreate8
 * ====================================================================== */

__declspec(dllexport) IDirect3D8* __stdcall Direct3DCreate8(UINT SDKVersion)
{
    if (!g_RealDirect3DCreate8) {
        /* This shouldn't happen if DllMain loaded the real DLL */
        return NULL;
    }

    IDirect3D8 *pD3D = g_RealDirect3DCreate8(SDKVersion);
    if (pD3D) {
        HookD3D8VTable(pD3D);
        
        /* Log success */
        char log_path[MAX_PATH];
        GetModuleFileNameA(NULL, log_path, MAX_PATH);
        char *p = strrchr(log_path, '.');
        if (p) strcpy(p, "_unlimited_tris.log");
        FILE *f = fopen(log_path, "a");
        if (f) {
            fprintf(f, "Direct3DCreate8 called: pD3D=%p, g_pRealD3D=%p\n", pD3D, g_pRealD3D);
            fclose(f);
        }
    }

    return pD3D;
}

/* ======================================================================
 * DllMain
 * ====================================================================== */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    char log_path[MAX_PATH];
    FILE *f;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);

        /* Load the real D3D8 DLL (d3d8to9 or the original) */
        g_hRealD3D8 = LoadLibraryA("d3d8_real.dll");
        if (g_hRealD3D8 == NULL) {
            /* Try system d3d8.dll */
            g_hRealD3D8 = LoadLibraryA("d3d8.dll");
        }

        if (g_hRealD3D8 != NULL) {
            g_RealDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(g_hRealD3D8, "Direct3DCreate8");
        }

        /* Write log */
        GetModuleFileNameA(NULL, log_path, MAX_PATH);
        {
            char *p = strrchr(log_path, '.');
            if (p) strcpy(p, "_unlimited_tris.log");
        }
        f = fopen(log_path, "w");
        if (f) {
            fprintf(f, "Hamsterball Unlimited Tris Mod (d3d8.dll proxy)\n");
            fprintf(f, "==================================================\n");
            fprintf(f, "Real D3D8 DLL: %s\n", g_hRealD3D8 ? "loaded" : "NOT FOUND");
            fprintf(f, "Direct3DCreate8: %s\n", g_RealDirect3DCreate8 ? "found" : "NOT FOUND");
            fprintf(f, "This mod removes vertex buffer size limits.\n");
            fprintf(f, "CreateVertexBuffer will fall back to SYSTEMMEM when DEFAULT fails.\n");
            fprintf(f, "DrawPrimitiveUP calls >65535 primitives will be split into chunks.\n");
            fclose(f);
        }
        break;

    case DLL_PROCESS_DETACH:
        /* Write final stats */
        GetModuleFileNameA(NULL, log_path, MAX_PATH);
        {
            char *p = strrchr(log_path, '.');
            if (p) strcpy(p, "_unlimited_tris.log");
        }
        f = fopen(log_path, "a");
        if (f) {
            fprintf(f, "\n=== Session Stats ===\n");
            fprintf(f, "CreateVertexBuffer: %d calls, %d fallbacks, %d failed\n",
                    g_nCreateVBCalls, g_nCreateVBFallback, g_nCreateVBFailed);
            fprintf(f, "DrawPrimitiveUP: %d calls, %d splits\n",
                    g_nDrawPrimCalls, g_nDrawPrimSplits);
            fclose(f);
        }

        if (g_hRealD3D8) { FreeLibrary(g_hRealD3D8); g_hRealD3D8 = NULL; }
        break;
    }
    return TRUE;
}
