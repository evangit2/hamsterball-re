/*
 * rotator.c — Rotator behavior DLL (diagnostic build)
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -lm -luser32
 */

#include <windows.h>
#include <math.h>
#include <stdio.h>

#define BOARD_SCENE      0x878
#define SCENE_MESHWORLD  0x08
#define MW_VTX_COUNT     0x438
#define MW_VTX_ARRAY     0x440
#define VERTEX_SIZE      32
#define ENTITY_VTX_COUNT 272
#define ROTATION_SPEED   0.02f

static float* g_vtx_array = NULL;
static int    g_entity_vtx_start = 0;
static float  g_center_y = 0.0f;
static float  g_center_z = 0.0f;
static float  g_angle = 0.0f;
static float* g_base_pos = NULL;
static int    g_initialized = 0;
static int    g_update_count = 0;

static DWORD safe_read_dword(DWORD addr) {
    if (!addr || addr < 0x10000) return 0;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(DWORD*)addr;
}

__declspec(dllexport) void __cdecl Behavior_Init(void* transform, void* board_ptr) {
    char msg[512];
    DWORD board = (DWORD)board_ptr;

    snprintf(msg, sizeof(msg), "Behavior_Init\nboard=0x%08X", (unsigned)board);
    MessageBoxA(NULL, msg, "Rotator Init", MB_OK);

    if (!board) return;

    DWORD scene = safe_read_dword(board + BOARD_SCENE);
    DWORD mw = safe_read_dword(scene + SCENE_MESHWORLD);

    snprintf(msg, sizeof(msg),
        "scene=0x%08X  mw=0x%08X", (unsigned)scene, (unsigned)mw);
    MessageBoxA(NULL, msg, "Rotator Init", MB_OK);

    if (!mw) return;

    if (IsBadReadPtr((void*)(mw + MW_VTX_COUNT), 4)) return;
    int total_vtx = *(int*)(mw + MW_VTX_COUNT);

    if (IsBadReadPtr((void*)(mw + MW_VTX_ARRAY), 4)) return;
    float* vtx = *(float**)(mw + MW_VTX_ARRAY);

    snprintf(msg, sizeof(msg),
        "total_vtx=%d  vtx_ptr=0x%08X\nentity_start=%d",
        total_vtx, (unsigned)vtx, total_vtx - ENTITY_VTX_COUNT);
    MessageBoxA(NULL, msg, "Rotator Init", MB_OK);

    if (!vtx || total_vtx < ENTITY_VTX_COUNT) return;
    if (IsBadReadPtr(vtx, total_vtx * VERTEX_SIZE)) return;

    g_entity_vtx_start = total_vtx - ENTITY_VTX_COUNT;
    g_vtx_array = vtx;

    /* Compute centroid */
    float sum_y = 0.0f, sum_z = 0.0f;
    int i;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = vtx + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);
        sum_y += v[1];
        sum_z += v[2];
    }
    g_center_y = sum_y / ENTITY_VTX_COUNT;
    g_center_z = sum_z / ENTITY_VTX_COUNT;

    /* Save base positions */
    g_base_pos = (float*)malloc(ENTITY_VTX_COUNT * 3 * sizeof(float));
    if (!g_base_pos) return;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = vtx + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);
        g_base_pos[i * 3 + 0] = v[0];
        g_base_pos[i * 3 + 1] = v[1];
        g_base_pos[i * 3 + 2] = v[2];
    }

    g_angle = 0.0f;
    g_initialized = 1;
    g_update_count = 0;

    snprintf(msg, sizeof(msg),
        "INIT OK!\ncenter=(%.1f, %.1f)\nbase_pos[0]=(%.1f, %.1f, %.1f)",
        g_center_y, g_center_z,
        g_base_pos[0], g_base_pos[1], g_base_pos[2]);
    MessageBoxA(NULL, msg, "Rotator Init", MB_OK);
}

__declspec(dllexport) void __cdecl Behavior_Update(void* transform, void* board_ptr) {
    if (!g_initialized || !g_vtx_array || !g_base_pos) return;

    g_update_count++;

    if (g_update_count == 60) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "Update called 60 times!\nangle=%.3f\nvtx[0] Y=%.1f Z=%.1f",
            g_angle,
            g_vtx_array[(g_entity_vtx_start) * (VERTEX_SIZE/4) + 1],
            g_vtx_array[(g_entity_vtx_start) * (VERTEX_SIZE/4) + 2]);
        MessageBoxA(NULL, msg, "Rotator Update", MB_OK);
    }

    if (IsBadReadPtr(g_vtx_array, (g_entity_vtx_start + ENTITY_VTX_COUNT) * VERTEX_SIZE)) {
        g_initialized = 0;
        return;
    }

    g_angle += ROTATION_SPEED;
    if (g_angle > 6.283185f) g_angle -= 6.283185f;

    float cos_a = cosf(g_angle);
    float sin_a = sinf(g_angle);

    int i;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = g_vtx_array + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);
        float by = g_base_pos[i * 3 + 1] - g_center_y;
        float bz = g_base_pos[i * 3 + 2] - g_center_z;
        v[1] = g_center_y + by * cos_a - bz * sin_a;
        v[2] = g_center_z + by * sin_a + bz * cos_a;
    }
}

__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    if (g_base_pos) { free(g_base_pos); g_base_pos = NULL; }
    g_initialized = 0;
    g_vtx_array = NULL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH)
        MessageBoxA(NULL, "Rotator.dll loaded!", "Rotator", MB_OK);
    return TRUE;
}
