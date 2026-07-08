/*
 * d3d8_rotator.c — D3D8 proxy that rotates ONLY the Rotator entity mesh
 *
 * Strategy: Hook DrawIndexedPrimitive instead of SetTransform.
 * The Rotator mesh has exactly 272 vertices. When a draw call with
 * 272 vertices (or a sub-range of them) happens, we save the current
 * world matrix, set a rotated world matrix, draw, then restore.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o d3d8.dll d3d8_rotator.c \
 *     -ld3d8 -lkernel32 -luser32 -Wl,--enable-stdcall-fixup \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -lm \
 *     -Wl,--add-stdcall-alias -Wl,--export-all-symbols
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>
#include <d3d8.h>

/* ===== Shared state with bass.dll ===== */
#define SHM_NAME "Hamsterball_Rotator"

typedef struct {
    int   active;
    float angle;
    float center_x;
    float center_y;
    float center_z;
    int   frame_count;
    int   rotator_vertex_count;  /* number of vertices in Rotator mesh */
} RotatorState;

static RotatorState* g_state = NULL;
static HANDLE g_shm = NULL;

/* ===== Real d3d8.dll ===== */
static HMODULE g_real_dll = NULL;

/* ===== Saved original function pointers ===== */
typedef HRESULT (WINAPI *SetTransform_t)(IDirect3DDevice8*, D3DTRANSFORMSTATETYPE, const D3DMATRIX*);
static SetTransform_t g_orig_SetTransform = NULL;

typedef HRESULT (WINAPI *CreateDevice_t)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD,
                                          D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
static CreateDevice_t g_orig_CreateDevice = NULL;

typedef HRESULT (WINAPI *DrawIndexedPrimitive_t)(IDirect3DDevice8*, D3DPRIMITIVETYPE,
                                                  UINT, UINT, UINT, UINT);
static DrawIndexedPrimitive_t g_orig_DrawIndexedPrimitive = NULL;

typedef HRESULT (WINAPI *DrawPrimitive_t)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT);
static DrawPrimitive_t g_orig_DrawPrimitive = NULL;

/* ===== Matrix math ===== */
static void build_rotX(D3DMATRIX* m, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    ZeroMemory(m, sizeof(*m));
    m->_11 = 1.0f; m->_22 = c; m->_23 = s; m->_33 = -s; m->_34 = 0.0f;
    m->_44 = 1.0f; m->_12 = 0.0f; m->_13 = 0.0f; m->_14 = 0.0f;
    m->_21 = 0.0f; m->_24 = 0.0f; m->_31 = 0.0f; m->_32 = 0.0f;
    m->_41 = 0.0f; m->_42 = 0.0f; m->_43 = 0.0f;
}

static void mat_mul(D3DMATRIX* result, const D3DMATRIX* a, const D3DMATRIX* b) {
    D3DMATRIX tmp;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            tmp.m[i][j] = a->m[i][0]*b->m[0][j] +
                          a->m[i][1]*b->m[1][j] +
                          a->m[i][2]*b->m[2][j] +
                          a->m[i][3]*b->m[3][j];
        }
    }
    *result = tmp;
}

static void build_translate(D3DMATRIX* m, float x, float y, float z) {
    ZeroMemory(m, sizeof(*m));
    m->_11 = 1.0f; m->_22 = 1.0f; m->_33 = 1.0f; m->_44 = 1.0f;
    m->_41 = x; m->_42 = y; m->_43 = z;
}

/* ===== Hooked DrawIndexedPrimitive ===== */
/* When the game draws the Rotator mesh (identified by vertex count),
 * we save the world matrix, set a rotated one, draw, then restore. */
static HRESULT WINAPI Hooked_DrawIndexedPrimitive(
    IDirect3DDevice8* This,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT MinIndex,
    UINT NumVertices,
    UINT StartIndex,
    UINT PrimitiveCount)
{
    /* Check if this draw call is for the Rotator mesh.
     * The Rotator has 272 vertices. If NumVertices matches (or is close),
     * we apply rotation. */
    if (g_state && g_state->active && g_orig_SetTransform &&
        g_orig_DrawIndexedPrimitive && g_state->rotator_vertex_count > 0 &&
        NumVertices == (UINT)g_state->rotator_vertex_count) {

        /* Save current world matrix */
        D3DMATRIX savedWorld;
        g_orig_SetTransform(This, D3DTS_WORLD, &savedWorld);

        /* Build rotated world matrix: translate-rotate-translate back */
        D3DMATRIX rotX, transToCenter, transBack, tmp1, tmp2;
        build_translate(&transToCenter, -g_state->center_x, -g_state->center_y, -g_state->center_z);
        build_translate(&transBack, g_state->center_x, g_state->center_y, g_state->center_z);
        build_rotX(&rotX, g_state->angle);
        mat_mul(&tmp1, &transToCenter, &savedWorld);
        mat_mul(&tmp2, &rotX, &tmp1);
        mat_mul(&tmp2, &transBack, &tmp2);

        /* Set rotated world matrix */
        g_orig_SetTransform(This, D3DTS_WORLD, &tmp2);

        /* Draw with rotated matrix */
        HRESULT hr = g_orig_DrawIndexedPrimitive(This, PrimitiveType, MinIndex,
                                                  NumVertices, StartIndex, PrimitiveCount);

        /* Restore original world matrix */
        g_orig_SetTransform(This, D3DTS_WORLD, &savedWorld);

        return hr;
    }

    /* Not the Rotator — pass through */
    return g_orig_DrawIndexedPrimitive(This, PrimitiveType, MinIndex,
                                        NumVertices, StartIndex, PrimitiveCount);
}

/* ===== Hooked DrawPrimitive (same logic but for non-indexed draws) ===== */
static HRESULT WINAPI Hooked_DrawPrimitive(
    IDirect3DDevice8* This,
    D3DPRIMITIVETYPE PrimitiveType,
    UINT StartVertex,
    UINT PrimitiveCount)
{
    /* For non-indexed draws, we can't easily identify the mesh by vertex count.
     * The game primarily uses DrawIndexedPrimitive, so this is a fallback.
     * Just pass through for now. */
    return g_orig_DrawPrimitive(This, PrimitiveType, StartVertex, PrimitiveCount);
}

/* ===== Hooked SetTransform — pass through (no longer rotates everything) ===== */
static HRESULT WINAPI Hooked_SetTransform(IDirect3DDevice8* This,
                                           D3DTRANSFORMSTATETYPE State,
                                           const D3DMATRIX* pMatrix) {
    /* Just forward to the real SetTransform.
     * Rotation is now applied in Hooked_DrawIndexedPrimitive instead. */
    return g_orig_SetTransform(This, State, pMatrix);
}

/* ===== Hooked CreateDevice ===== */
static HRESULT WINAPI Hooked_CreateDevice(
    IDirect3D8* This,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice8** ppReturnedDevice)
{
    HRESULT hr = g_orig_CreateDevice(This, Adapter, DeviceType, hFocusWindow,
                                      BehaviorFlags, pPresentationParameters,
                                      ppReturnedDevice);

    if (SUCCEEDED(hr) && *ppReturnedDevice) {
        IDirect3DDevice8* dev = *ppReturnedDevice;
        void** real_vt = *(void***)dev;
        DWORD old_protect;

        /* Save originals before patching */
        g_orig_SetTransform = (SetTransform_t)real_vt[37];
        g_orig_DrawIndexedPrimitive = (DrawIndexedPrimitive_t)real_vt[71];
        g_orig_DrawPrimitive = (DrawPrimitive_t)real_vt[70];

        VirtualProtect(real_vt, 200 * sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
        /* Patch SetTransform [37], DrawIndexedPrimitive [71], DrawPrimitive [70] */
        real_vt[37] = (void*)Hooked_SetTransform;
        real_vt[71] = (void*)Hooked_DrawIndexedPrimitive;
        real_vt[70] = (void*)Hooked_DrawPrimitive;
        VirtualProtect(real_vt, 200 * sizeof(void*), old_protect, &old_protect);
    }

    return hr;
}

/* ===== Direct3DCreate8 export ===== */
typedef IDirect3D8* (WINAPI *Direct3DCreate8_t)(UINT);

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    if (!g_real_dll) {
        char sys_path[MAX_PATH];
        GetSystemDirectoryA(sys_path, MAX_PATH);
        lstrcatA(sys_path, "\\d3d8.dll");
        g_real_dll = LoadLibraryA(sys_path);
        if (!g_real_dll) return NULL;

        Direct3DCreate8_t real_fn = (Direct3DCreate8_t)GetProcAddress(g_real_dll, "Direct3DCreate8");
        if (!real_fn) return NULL;

        IDirect3D8* d3d = real_fn(SDKVersion);
        if (!d3d) return NULL;

        void** d3d_vt = *(void***)d3d;
        DWORD old_protect;
        VirtualProtect(d3d_vt, 20 * sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
        g_orig_CreateDevice = (CreateDevice_t)d3d_vt[15];
        d3d_vt[15] = (void*)Hooked_CreateDevice;
        VirtualProtect(d3d_vt, 20 * sizeof(void*), old_protect, &old_protect);

        return d3d;
    }

    Direct3DCreate8_t real_fn = (Direct3DCreate8_t)GetProcAddress(g_real_dll, "Direct3DCreate8");
    if (!real_fn) return NULL;
    return real_fn(SDKVersion);
}

/* ===== Shared memory ===== */
static void init_shared_memory(void) {
    g_shm = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                0, sizeof(RotatorState), SHM_NAME);
    if (!g_shm) return;
    g_state = (RotatorState*)MapViewOfFile(g_shm, FILE_MAP_ALL_ACCESS, 0, 0,
                                            sizeof(RotatorState));
    if (g_state && GetLastError() != ERROR_ALREADY_EXISTS) {
        ZeroMemory(g_state, sizeof(RotatorState));
        g_state->rotator_vertex_count = 272;  /* default for Rotator.MESHWORLD */
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        init_shared_memory();
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_state) UnmapViewOfFile(g_state);
        if (g_shm) CloseHandle(g_shm);
    }
    return TRUE;
}
