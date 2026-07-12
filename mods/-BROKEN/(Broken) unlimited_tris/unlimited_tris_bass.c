/*
 * unlimited_tris_bass.c — BASS.dll proxy that removes D3D8 vertex buffer size limits.
 *
 * Uses the proven bass.dll proxy pattern (same as FPS mod).
 * After game init, walks App->Graphics->D3DDevice pointer chain,
 * hooks CreateVertexBuffer in the device vtable to fall back to
 * D3DPOOL_SYSTEMMEM when D3DPOOL_DEFAULT fails, and splits
 * DrawPrimitiveUP calls exceeding 65535 primitives.
 *
 * Installation:
 *   1. In game folder, rename bass.dll -> bass_real.dll
 *   2. Copy this proxy bass.dll into the game folder
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll unlimited_tris_bass.c \
 *     bass_exports.def -lwinmm -Wl,--enable-stdcall-fixup -O2 \
 *     -static -static-libgcc -Wl,--add-stdcall-alias
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
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
 * D3D8 Device Hook
 * ====================================================================== */

/* Pointer chain: App (0x005341E0) -> +0x174 -> Graphics -> +0x154 -> D3DDevice */
#define APP_PTR_ADDR   0x005341E0
#define APP_GRAPHICS   0x174
#define GFX_D3DDEVICE  0x154

/* D3D8 constants */
#ifndef D3DPOOL_DEFAULT
#define D3DPOOL_DEFAULT       0
#define D3DPOOL_MANAGED       1
#define D3DPOOL_SYSTEMMEM     2
#endif

#ifndef D3D_OK
#define D3D_OK                S_OK
#endif

#ifndef D3DCREATE_HARDWARE_VERTEXPROCESSING
#define D3DCREATE_HARDWARE_VERTEXPROCESSING 0x20
#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x08
#endif

#define MAX_PRIMITIVES_PER_CALL 65535

static void *g_pRealDevice = NULL;
static DWORD g_OriginalVTable[119]; /* IDirect3DDevice8 has 119 vtable methods */

/* Stats */
static int g_nCreateVBCalls = 0;
static int g_nCreateVBFallback = 0;
static int g_nCreateVBFailed = 0;
static int g_nDrawPrimCalls = 0;
static int g_nDrawPrimSplits = 0;

/* ---- Safe pointer read ---- */
static int SafeReadPtr(void *addr, void **out)
{
    if (IsBadReadPtr(addr, 4)) return 0;
    *out = *(void **)addr;
    return (*out != NULL);
}

/* ---- Our hooked CreateVertexBuffer ----
 * vtable slot 27 for IDirect3DDevice8
 * Signature: HRESULT CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVB)
 * __thiscall via ECX = device, but D3D8 uses stdcall with device as first param on stack
 */
typedef HRESULT (__stdcall *Fn_CreateVB)(void*, UINT, DWORD, DWORD, DWORD /*D3DPOOL*/, void**);

static HRESULT __stdcall Hooked_CreateVertexBuffer(
    void *pDev,
    UINT Length,
    DWORD Usage,
    DWORD FVF,
    DWORD Pool,
    void **ppVertexBuffer)
{
    Fn_CreateVB RealCreateVB = (Fn_CreateVB)g_OriginalVTable[27];

    g_nCreateVBCalls++;

    /* Log the call */
    {
        char lp[MAX_PATH];
        GetModuleFileNameA(NULL, lp, MAX_PATH);
        char *pp = strrchr(lp, '.');
        if (pp) strcpy(pp, "_unlimited_tris.log");
        FILE *lf = fopen(lp, "a");
        if (lf) {
            fprintf(lf, "[CreateVB] Length=%u Usage=0x%X FVF=0x%X Pool=%d\n",
                    Length, Usage, FVF, Pool);
            fflush(lf);
            fclose(lf);
        }
    }

    /* Try the original call first */
    HRESULT hr = RealCreateVB(pDev, Length, Usage, FVF, Pool, ppVertexBuffer);
    if (SUCCEEDED(hr)) return hr;

    /* If it failed and we were using D3DPOOL_DEFAULT, try D3DPOOL_SYSTEMMEM */
    if (Pool == D3DPOOL_DEFAULT) {
        g_nCreateVBFallback++;
        {
            char lp[MAX_PATH]; GetModuleFileNameA(NULL, lp, MAX_PATH);
            char *pp = strrchr(lp, '.'); if (pp) strcpy(pp, "_unlimited_tris.log");
            FILE *lf = fopen(lp, "a");
            if (lf) { fprintf(lf, "[CreateVB] Fallback: trying SYSTEMMEM (hr=0x%lX)\n", hr); fflush(lf); fclose(lf); }
        }
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) {
            char lp[MAX_PATH]; GetModuleFileNameA(NULL, lp, MAX_PATH);
            char *pp = strrchr(lp, '.'); if (pp) strcpy(pp, "_unlimited_tris.log");
            FILE *lf = fopen(lp, "a");
            if (lf) { fprintf(lf, "[CreateVB] SYSTEMMEM succeeded!\n"); fflush(lf); fclose(lf); }
            return hr;
        }

        /* Try D3DPOOL_MANAGED */
        g_nCreateVBFallback++;
        hr = RealCreateVB(pDev, Length, Usage, FVF, D3DPOOL_MANAGED, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* Try with no usage flags and SYSTEMMEM */
    if (Usage != 0) {
        g_nCreateVBFallback++;
        hr = RealCreateVB(pDev, Length, 0, FVF, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
        if (SUCCEEDED(hr)) return hr;
    }

    /* Last resort: zero FVF, zero usage, SYSTEMMEM */
    g_nCreateVBFallback++;
    hr = RealCreateVB(pDev, Length, 0, 0, D3DPOOL_SYSTEMMEM, ppVertexBuffer);
    if (SUCCEEDED(hr)) return hr;

    g_nCreateVBFailed++;
    return hr;
}

/* ---- Our hooked DrawPrimitiveUP ----
 * vtable slot 49
 * Splits large draw calls into chunks of <=65535 primitives.
 */
typedef HRESULT (__stdcall *Fn_DrawPrimUP)(void*, DWORD /*D3DPRIMITIVETYPE*/, UINT, const void*, UINT);

static HRESULT __stdcall Hooked_DrawPrimitiveUP(
    void *pDev,
    DWORD PrimitiveType,
    UINT PrimitiveCount,
    const void *pVertexStreamZeroData,
    UINT VertexStreamZeroStride)
{
    Fn_DrawPrimUP RealDrawUP = (Fn_DrawPrimUP)g_OriginalVTable[49];

    g_nDrawPrimCalls++;

    /* Log first few calls */
    if (g_nDrawPrimCalls <= 5) {
        char lp[MAX_PATH]; GetModuleFileNameA(NULL, lp, MAX_PATH);
        char *pp = strrchr(lp, '.'); if (pp) strcpy(pp, "_unlimited_tris.log");
        FILE *lf = fopen(lp, "a");
        if (lf) {
            fprintf(lf, "[DrawPrimUP] Type=%d Count=%u Stride=%u\n",
                    PrimitiveType, PrimitiveCount, VertexStreamZeroStride);
            fflush(lf); fclose(lf);
        }
    }

    /* If within limits, just pass through */
    if (PrimitiveCount <= MAX_PRIMITIVES_PER_CALL) {
        return RealDrawUP(pDev, PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
    }

    /* Split into chunks */
    g_nDrawPrimSplits++;
    HRESULT finalHr = D3D_OK;
    const BYTE *pData = (const BYTE *)pVertexStreamZeroData;
    UINT remaining = PrimitiveCount;
    UINT vertsPerPrim = 3; /* D3DPT_TRIANGLELIST = 3 verts per tri */
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

/* ---- Hook the D3D8 device vtable ---- */
static int HookDevice(void *pDevice, FILE *log)
{
    if (!pDevice || g_pRealDevice) return 0;

    g_pRealDevice = pDevice;

    DWORD *pVTable = *(DWORD **)pDevice;
    if (!pVTable) {
        fprintf(log, "  ERROR: Device vtable is NULL\n");
        return 0;
    }

    fprintf(log, "  Device: %p, VTable: %p\n", pDevice, pVTable);
    fprintf(log, "  Original CreateVB: %p (slot 27)\n", (void*)pVTable[27]);
    fprintf(log, "  Original DrawPrimUP: %p (slot 49)\n", (void*)pVTable[49]);

    /* Save original vtable entries */
    memcpy(g_OriginalVTable, pVTable, sizeof(g_OriginalVTable));

    /* Make vtable writable */
    DWORD oldProtect;
    if (!VirtualProtect(pVTable, sizeof(g_OriginalVTable), PAGE_READWRITE, &oldProtect)) {
        fprintf(log, "  ERROR: VirtualProtect failed: %lu\n", GetLastError());
        return 0;
    }

    /* Hook CreateVertexBuffer (slot 27) */
    pVTable[27] = (DWORD)Hooked_CreateVertexBuffer;

    /* Hook DrawPrimitiveUP (slot 49) */
    pVTable[49] = (DWORD)Hooked_DrawPrimitiveUP;

    VirtualProtect(pVTable, sizeof(g_OriginalVTable), oldProtect, &oldProtect);

    fprintf(log, "  Hooked CreateVB: %p -> %p\n", (void*)g_OriginalVTable[27], (void*)pVTable[27]);
    fprintf(log, "  Hooked DrawPrimUP: %p -> %p\n", (void*)g_OriginalVTable[49], (void*)pVTable[49]);
    fprintf(log, "  HOOK SUCCESS\n");

    return 1;
}

static void apply_unlimited_tris_patches(FILE *log)
{
    /* Patch all "mov byte ptr [reg+0x459], 0" to "mov byte ptr [reg+0x459], 1"
     * This prevents the game from disabling rendering when CreateVertexBuffer fails.
     *
     * There are 4 locations where +0x459 is set to 0:
     *   0x4652BC: C6 80 59 04 00 00 00  (mov byte ptr [eax+0x459], 0)
     *   0x4658B9: C6 80 59 04 00 00 00  (mov byte ptr [eax+0x459], 0)
     *   0x4658EA: C6 80 59 04 00 00 00  (mov byte ptr [eax+0x459], 0)
     *   0x46F2BC: C6 86 59 04 00 00 00  (mov byte ptr [esi+0x459], 0) -- different register!
     */

    /* Pattern for [eax+0x459] = 0 -> [eax+0x459] = 1 */
    BYTE pat_eax[] = { 0xC6, 0x80, 0x59, 0x04, 0x00, 0x00, 0x00 };
    BYTE rep_eax[] = { 0xC6, 0x80, 0x59, 0x04, 0x00, 0x00, 0x01 };

    /* Pattern for [esi+0x459] = 0 -> [esi+0x459] = 1 */
    BYTE pat_esi[] = { 0xC6, 0x86, 0x59, 0x04, 0x00, 0x00, 0x00 };
    BYTE rep_esi[] = { 0xC6, 0x86, 0x59, 0x04, 0x00, 0x00, 0x01 };

    int total = 0;

    /* Patch at specific addresses */
    DWORD eax_addrs[] = { 0x4652BC, 0x4658B9, 0x4658EA };
    for (int i = 0; i < 3; i++) {
        DWORD va = eax_addrs[i];
        DWORD oldProtect;
        BYTE *addr = (BYTE *)va;

        /* Verify the pattern */
        if (memcmp(addr, pat_eax, sizeof(pat_eax)) == 0) {
            if (VirtualProtect(addr, sizeof(pat_eax), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(addr, rep_eax, sizeof(rep_eax));
                VirtualProtect(addr, sizeof(pat_eax), oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(pat_eax));
                fprintf(log, "  Patched +0x459=0 -> =1 at 0x%08lX\n", va);
                total++;
            } else {
                fprintf(log, "  FAILED to patch at 0x%08lX (VirtualProtect error %lu)\n", va, GetLastError());
            }
        } else {
            fprintf(log, "  SKIP 0x%08lX: byte mismatch (expected C6 80 59 04 00 00 00)\n", va);
        }
    }

    /* Patch esi variant at 0x46F2BC */
    {
        DWORD va = 0x46F2BC;
        BYTE *addr = (BYTE *)va;
        DWORD oldProtect;

        if (memcmp(addr, pat_esi, sizeof(pat_esi)) == 0) {
            if (VirtualProtect(addr, sizeof(pat_esi), PAGE_EXECUTE_READWRITE, &oldProtect)) {
                memcpy(addr, rep_esi, sizeof(rep_esi));
                VirtualProtect(addr, sizeof(pat_esi), oldProtect, &oldProtect);
                FlushInstructionCache(GetCurrentProcess(), addr, sizeof(pat_esi));
                fprintf(log, "  Patched +0x459=0 -> =1 at 0x%08lX (esi variant)\n", va);
                total++;
            }
        } else {
            fprintf(log, "  SKIP 0x%08lX: byte mismatch (expected C6 86 59 04 00 00 00)\n", va);
        }
    }

    fprintf(log, "Total +0x459 patches applied: %d\n", total);
}

/* ======================================================================
 * Background Thread - finds D3D8 device and hooks it
 * ====================================================================== */

static DWORD WINAPI HookThread(LPVOID lpParam)
{
    (void)lpParam;

    char log_path[MAX_PATH];
    GetModuleFileNameA(NULL, log_path, MAX_PATH);
    char *p = strrchr(log_path, '.');
    if (p) strcpy(p, "_unlimited_tris.log");

    FILE *f = fopen(log_path, "w");
    if (!f) f = stdout;

    fprintf(f, "Hamsterball Unlimited Tris Mod (bass.dll proxy)\n");
    fprintf(f, "==================================================\n");

    /* Apply binary patches IMMEDIATELY (before game init) */
    fprintf(f, "Applying unlimited tris patches...\n");
    apply_unlimited_tris_patches(f);
    fflush(f);

    fprintf(f, "Waiting for game initialization...\n");
    fflush(f);

    /* Wait a minimal amount for App to be constructed */
    Sleep(500);

    /* Poll for App pointer to become valid - very aggressively */
    void *pApp = NULL;
    int attempts = 0;
    while (attempts < 60) {
        if (SafeReadPtr((void *)APP_PTR_ADDR, &pApp)) {
            fprintf(f, "App pointer at 0x%p: 0x%p\n", (void*)APP_PTR_ADDR, pApp);
            break;
        }
        Sleep(100);
        attempts++;
    }

    if (!pApp) {
        fprintf(f, "ERROR: Could not find App pointer after %d attempts\n", attempts);
        if (f != stdout) fclose(f);
        return 1;
    }

    /* Read Graphics pointer from App+0x174 */
    void *pGraphics = NULL;
    attempts = 0;
    while (attempts < 60) {
        if (SafeReadPtr((void *)((BYTE *)pApp + APP_GRAPHICS), &pGraphics)) {
            fprintf(f, "Graphics pointer at App+0x%X: 0x%p\n", APP_GRAPHICS, pGraphics);
            break;
        }
        Sleep(100);
        attempts++;
    }

    if (!pGraphics) {
        fprintf(f, "ERROR: Could not find Graphics pointer after %d attempts\n", attempts);
        if (f != stdout) fclose(f);
        return 1;
    }

    /* Read D3DDevice pointer from Graphics+0x154 */
    void *pDevice = NULL;
    attempts = 0;
    while (attempts < 60) {
        if (SafeReadPtr((void *)((BYTE *)pGraphics + GFX_D3DDEVICE), &pDevice)) {
            fprintf(f, "D3DDevice pointer at Graphics+0x%X: 0x%p\n", GFX_D3DDEVICE, pDevice);
            break;
        }
        Sleep(100);
        attempts++;
    }

    if (!pDevice) {
        fprintf(f, "ERROR: Could not find D3DDevice pointer after %d attempts\n", attempts);
        if (f != stdout) fclose(f);
        return 1;
    }

    /* Hook the device */
    fprintf(f, "Hooking D3D8 device...\n");
    int ok = HookDevice(pDevice, f);
    if (!ok) {
        fprintf(f, "ERROR: Hook failed\n");
    }
    fflush(f);

    /* Periodically log stats */
    while (1) {
        Sleep(15000);
        if (g_nCreateVBCalls > 0 || g_nDrawPrimCalls > 0) {
            fprintf(f, "[Stats] CreateVB: %d calls, %d fallbacks, %d failed | DrawPrim: %d calls, %d splits\n",
                    g_nCreateVBCalls, g_nCreateVBFallback, g_nCreateVBFailed,
                    g_nDrawPrimCalls, g_nDrawPrimSplits);
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
