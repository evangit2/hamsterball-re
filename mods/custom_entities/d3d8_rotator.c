/*
 * d3d8_rotator.c — Minimal D3D8 proxy DLL that hooks SetTransform
 * to inject world matrix rotation for the Custom Entities Rotator.
 *
 * The proxy forwards ALL D3D8 calls to the real d3d8.dll, except
 * SetTransform(D3DTS_WORLD, &matrix) which gets a rotation matrix
 * multiplied in when the world position matches the Rotator entity.
 *
 * Communication with bass.dll:
 * - bass.dll writes the rotation angle + center position to a
 *   shared memory region (memory-mapped file "Hamsterball_Rotator")
 * - d3d8.dll reads it each frame and applies the rotation
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o d3d8.dll d3d8_rotator.c \
 *     -ld3d8 -lkernel32 -luser32 -Wl,--enable-stdcall-fixup \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -lm \
 *     -Wl,--add-stdcall-alias -Wl,--def,d3d8_rotator.def
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h>

/* D3D8 types we need */
#include <d3d8.h>
#include <stddef.h>

/* ===== Shared state with bass.dll ===== */

#define SHM_NAME "Hamsterball_Rotator"

typedef struct {
    int   active;       /* 1 = rotation enabled */
    float angle;        /* current rotation angle (radians) */
    float center_x;     /* rotation center X */
    float center_y;     /* rotation center Y */
    float center_z;     /* rotation center Z */
    int   frame_count;  /* incremented each SetTransform(D3DTS_WORLD) call */
} RotatorState;

static RotatorState* g_state = NULL;
static HANDLE g_shm = NULL;

/* ===== Real d3d8.dll loading ===== */
static HMODULE g_real_dll = NULL;
static void* g_real_Direct3DCreate8 = NULL;

/* Proxy device — wraps real IDirect3DDevice8 */
typedef struct {
    IDirect3DDevice8* real;
    void* vt[97];  /* IDirect3DDevice8 vtable (97 methods) */
} ProxyDevice;

static ProxyDevice* g_device = NULL;

/* ===== Matrix math ===== */

/* Build a rotation matrix around the X axis */
static void build_rotX(D3DMATRIX* m, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    ZeroMemory(m, sizeof(*m));
    m->_11 = 1.0f;  m->_12 = 0.0f;  m->_13 = 0.0f;  m->_14 = 0.0f;
    m->_21 = 0.0f;  m->_22 = c;     m->_23 = s;     m->_24 = 0.0f;
    m->_31 = 0.0f;  m->_32 = -s;    m->_33 = c;     m->_34 = 0.0f;
    m->_41 = 0.0f;  m->_42 = 0.0f;  m->_43 = 0.0f;  m->_44 = 1.0f;
}

/* Multiply two 4x4 matrices: result = a * b */
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

/* Build a translation matrix */
static void build_translate(D3DMATRIX* m, float x, float y, float z) {
    ZeroMemory(m, sizeof(*m));
    m->_11 = 1.0f; m->_22 = 1.0f; m->_33 = 1.0f; m->_44 = 1.0f;
    m->_41 = x; m->_42 = y; m->_43 = z;
}

/* ===== Hooked SetTransform ===== */
static HRESULT WINAPI Hooked_SetTransform(IDirect3DDevice8* This,
                                           D3DTRANSFORMSTATETYPE State,
                                           const D3DMATRIX* pMatrix) {
    if (g_state && g_state->active && State == D3DTS_WORLD && pMatrix) {
        /* Check if this world matrix has a translation (position) */
        float tx = pMatrix->_41;
        float ty = pMatrix->_42;
        float tz = pMatrix->_43;

        /* Check if this is our entity by comparing position to center */
        float dx = tx - g_state->center_x;
        float dy = ty - g_state->center_y;
        float dz = tz - g_state->center_z;
        float dist_sq = dx*dx + dy*dy + dz*dz;

        if (dist_sq < 100.0f) {
            /* This is our entity — inject X-axis rotation */
            D3DMATRIX rotX, transToCenter, transBack, tmp1, tmp2;

            build_translate(&transToCenter, -g_state->center_x, -g_state->center_y, -g_state->center_z);
            build_translate(&transBack, g_state->center_x, g_state->center_y, g_state->center_z);
            build_rotX(&rotX, g_state->angle);

            /* result = translateBack * rotX * translateToCenter * originalMatrix */
            mat_mul(&tmp1, &transToCenter, pMatrix);
            mat_mul(&tmp2, &rotX, &tmp1);
            mat_mul(&tmp2, &transBack, &tmp2);

            return g_device->real->lpVtbl->SetTransform(g_device->real, State, &tmp2);
        }

        g_state->frame_count++;
    }

    return g_device->real->lpVtbl->SetTransform(g_device->real, State, pMatrix);
}

/* ===== Proxy vtable setup ===== */

/* We need to forward all 97 vtable methods, but only SetTransform is hooked.
 * SetTransform is vtable[44] (index 44, offset 0xB0) in IDirect3DDevice8.
 * All others just forward to the real device. */

/* Generic forwarding thunks — we use a single function for all unhooked methods */
#define FORWARD0(name, idx) \
    static HRESULT WINAPI thunk_##name(IDirect3DDevice8* This) { \
        return ((HRESULT (WINAPI*)(IDirect3DDevice8*))g_device->vt[idx])(This); \
    }

/* Actually, with MinGW we can't easily generate 97 forwarding thunks.
 * Let's use a simpler approach: just copy the real vtable and replace
 * only the SetTransform entry. */

static void setup_proxy_vtable(ProxyDevice* p) {
    /* Copy all 97 entries from the real device's vtable */
    memcpy(p->vt, p->real->lpVtbl, 97 * sizeof(void*));

    /* Replace SetTransform (index 44) with our hooked version */
    p->vt[44] = (void*)Hooked_SetTransform;
}

/* ===== IDirect3D8 wrapper ===== */

/* We need to wrap IDirect3D8 to intercept CreateDevice and return our proxy */
typedef struct {
    IDirect3D8* real;
    void* vt[20]; /* IDirect3D8 vtable */
} ProxyD3D8;

/* Forward CreateDevice to create our proxy device */
static HRESULT WINAPI Hooked_CreateDevice(
    IDirect3D8* This,
    UINT Adapter,
    D3DDEVTYPE DeviceType,
    HWND hFocusWindow,
    DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice8** ppReturnedDevice)
{
    ProxyD3D8* proxy = (ProxyD3D8*)((char*)This - offsetof(ProxyD3D8, vt));
    HRESULT hr = proxy->real->lpVtbl->CreateDevice(proxy->real,
        Adapter, DeviceType, hFocusWindow, BehaviorFlags,
        pPresentationParameters, ppReturnedDevice);

    if (SUCCEEDED(hr) && *ppReturnedDevice) {
        /* Create our proxy device wrapper */
        g_device = (ProxyDevice*)HeapAlloc(GetProcessHeap(), 0, sizeof(ProxyDevice));
        if (g_device) {
            g_device->real = *ppReturnedDevice;
            setup_proxy_vtable(g_device);

            /* Return our proxy as the device — but we need to make it
             * look like an IDirect3DDevice8 to the game.
             * The game calls through the vtable, so we point it to our
             * vtable array. */
            /* Actually, the game expects a COM object. We need to create
             * a fake COM object whose vtable pointer points to our vtable. */
            /* The simplest approach: overwrite the returned device's vtable
             * pointer with our own. This is dirty but works. */
            /* Actually no — we can't overwrite the real device's vtable.
             * We need to return a wrapper. But the game expects IDirect3DDevice8*.
             *
             * Let's use a different approach: just hook the function in the
             * real device's vtable directly. */
            void** real_vt = *(void***)*ppReturnedDevice;
            DWORD old_protect;

            /* Make the vtable writable */
            VirtualProtect(real_vt, 97 * sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);

            /* Replace SetTransform (vtable index 37) */
            real_vt[37] = (void*)Hooked_SetTransform;

            /* Restore protection */
            VirtualProtect(real_vt, 97 * sizeof(void*), old_protect, &old_protect);

            HeapFree(GetProcessHeap(), 0, g_device);
            g_device = (ProxyDevice*)1; /* flag: hooked */
        }
    }

    return hr;
}

/* ===== Direct3DCreate8 export ===== */

typedef IDirect3D8* (WINAPI *Direct3DCreate8_t)(UINT);

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    if (!g_real_dll) {
        /* Load real d3d8.dll from system32 */
        char sys_path[MAX_PATH];
        GetSystemDirectoryA(sys_path, MAX_PATH);
        strcat_s(sys_path, MAX_PATH, "\\d3d8.dll");
        g_real_dll = LoadLibraryA(sys_path);
        if (!g_real_dll) return NULL;

        g_real_Direct3DCreate8 = GetProcAddress(g_real_dll, "Direct3DCreate8");
        if (!g_real_Direct3DCreate8) return NULL;
    }

    Direct3DCreate8_t real_fn = (Direct3DCreate8_t)g_real_Direct3DCreate8;
    IDirect3D8* d3d = real_fn(SDKVersion);
    if (!d3d) return NULL;

    /* Hook CreateDevice in the D3D8 vtable */
    void** d3d_vt = *(void***)d3d;
    DWORD old_protect;
    VirtualProtect(d3d_vt, 20 * sizeof(void*), PAGE_EXECUTE_READWRITE, &old_protect);
    d3d_vt[1] = (void*)Hooked_CreateDevice; /* CreateDevice is vtable[1] */
    VirtualProtect(d3d_vt, 20 * sizeof(void*), old_protect, &old_protect);

    return d3d;
}

/* ===== Shared memory setup ===== */

static void init_shared_memory(void) {
    g_shm = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                0, sizeof(RotatorState), SHM_NAME);
    if (!g_shm) return;

    g_state = (RotatorState*)MapViewOfFile(g_shm, FILE_MAP_ALL_ACCESS, 0, 0,
                                            sizeof(RotatorState));
    if (g_state) {
        /* Initialize if this is the first mapper */
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            ZeroMemory(g_state, sizeof(RotatorState));
        }
    }
}

/* ===== DLL entry point ===== */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        init_shared_memory();
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_state) UnmapViewOfFile(g_state);
        if (g_shm) CloseHandle(g_shm);
    }
    return TRUE;
}
