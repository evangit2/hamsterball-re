/*
 * rotator.c — Rotator behavior DLL for Custom Entities mod
 *
 * Rotates entity vertices directly in system memory each frame.
 * The game's EntityTransform is read once during loading and cached —
 * modifying it at runtime has no effect. Instead, we modify the actual
 * vertex positions in the MeshWorld vertex array.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse
 */

#include <windows.h>
#include <math.h>

/* Game struct offsets (verified via Ghidra decompilation) */
#define BOARD_SCENE      0x878   /* Board+0x878 → Scene* */
#define SCENE_MESHWORLD  0x08    /* Scene+0x08 → MeshWorld* */
#define MW_VTX_COUNT     0x438   /* MeshWorld+0x438 → u32 vertex count */
#define MW_VTX_ARRAY     0x440   /* MeshWorld+0x440 → float* vertex array */

/* Each vertex is 32 bytes: pos(3f=12) + normal(3f=12) + uv(2f=8) */
#define VERTEX_SIZE      32

/* Entity vertex count (from Rotator.MESHWORLD: 272 vertices) */
#define ENTITY_VTX_COUNT 272

/* Rotation speed: ~0.02 rad/frame ≈ 69°/sec at 60fps */
#define ROTATION_SPEED   0.02f

/* State */
static float* g_vtx_array = NULL;      /* Pointer to MeshWorld vertex array */
static int    g_entity_vtx_start = 0;  /* Index of first entity vertex */
static float  g_center_y = 0.0f;       /* Rotation center Y (centroid) */
static float  g_center_z = 0.0f;       /* Rotation center Z (centroid) */
static float  g_angle = 0.0f;          /* Current rotation angle */
static float* g_base_pos = NULL;        /* Saved base positions (x,y,z per vertex) */
static int    g_initialized = 0;

/* Safe read helper */
static DWORD safe_read_dword(DWORD addr) {
    if (!addr || addr < 0x10000) return 0;
    if (IsBadReadPtr((void*)addr, 4)) return 0;
    return *(DWORD*)addr;
}

/* Behavior_Init — called once when entity is detected */
__declspec(dllexport) void __cdecl Behavior_Init(void* transform, void* board_ptr) {
    DWORD board = (DWORD)board_ptr;
    if (!board) return;

    /* Navigate: board → scene → meshworld */
    DWORD scene = safe_read_dword(board + BOARD_SCENE);
    if (!scene) return;
    DWORD mw = safe_read_dword(scene + SCENE_MESHWORLD);
    if (!mw) return;

    /* Read vertex count and array pointer */
    if (IsBadReadPtr((void*)(mw + MW_VTX_COUNT), 4)) return;
    int total_vtx = *(int*)(mw + MW_VTX_COUNT);
    if (total_vtx < ENTITY_VTX_COUNT) return;

    if (IsBadReadPtr((void*)(mw + MW_VTX_ARRAY), 4)) return;
    float* vtx = *(float**)(mw + MW_VTX_ARRAY);
    if (!vtx || IsBadReadPtr(vtx, total_vtx * VERTEX_SIZE)) return;

    /* Entity vertices are the last ENTITY_VTX_COUNT in the array
     * (the merger appends them after the level's original vertices) */
    g_entity_vtx_start = total_vtx - ENTITY_VTX_COUNT;
    g_vtx_array = vtx;

    /* Calculate centroid of entity vertices (rotation center) */
    float sum_y = 0.0f, sum_z = 0.0f;
    int i;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = vtx + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);
        sum_y += v[1];  /* Y at offset 4 (float index 1) */
        sum_z += v[2];  /* Z at offset 8 (float index 2) */
    }
    g_center_y = sum_y / ENTITY_VTX_COUNT;
    g_center_z = sum_z / ENTITY_VTX_COUNT;

    /* Save base positions */
    g_base_pos = (float*)malloc(ENTITY_VTX_COUNT * 3 * sizeof(float));
    if (!g_base_pos) return;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = vtx + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);
        g_base_pos[i * 3 + 0] = v[0];  /* X */
        g_base_pos[i * 3 + 1] = v[1];  /* Y */
        g_base_pos[i * 3 + 2] = v[2];  /* Z */
    }

    g_angle = 0.0f;
    g_initialized = 1;
}

/* Behavior_Update — called every frame (~60Hz) */
__declspec(dllexport) void __cdecl Behavior_Update(void* transform, void* board_ptr) {
    if (!g_initialized || !g_vtx_array || !g_base_pos) return;

    /* Validate vertex array is still accessible */
    if (IsBadReadPtr(g_vtx_array, (g_entity_vtx_start + ENTITY_VTX_COUNT) * VERTEX_SIZE)) {
        g_initialized = 0;
        return;
    }

    /* Increment rotation angle */
    g_angle += ROTATION_SPEED;
    if (g_angle > 6.283185f) g_angle -= 6.283185f;

    float cos_a = cosf(g_angle);
    float sin_a = sinf(g_angle);

    /* Apply X-axis rotation around the entity center */
    int i;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = g_vtx_array + (g_entity_vtx_start + i) * (VERTEX_SIZE / 4);

        /* Base position relative to center */
        float by = g_base_pos[i * 3 + 1] - g_center_y;
        float bz = g_base_pos[i * 3 + 2] - g_center_z;

        /* Rotated position */
        v[1] = g_center_y + by * cos_a - bz * sin_a;  /* Y */
        v[2] = g_center_z + by * sin_a + bz * cos_a;  /* Z */
        /* X stays the same (rotation around X axis) */
    }
}

/* Behavior_Shutdown — called when level unloads */
__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    if (g_base_pos) {
        free(g_base_pos);
        g_base_pos = NULL;
    }
    g_initialized = 0;
    g_vtx_array = NULL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
