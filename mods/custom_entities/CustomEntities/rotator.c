/*
 * rotator.c — Rotator behavior DLL for Custom Entities mod
 *
 * Calls the game's Gfx_RotateY function directly to apply rotation.
 * The game is loaded at base 0x400000 (no ASLR), so we can call
 * any game function by its absolute address.
 *
 * Gfx_RotateY (0x457bb0) builds a rotation matrix and stores it
 * in the game's world transform. The game applies this matrix
 * when rendering the next frame.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -lm -luser32
 */

#include <windows.h>
#include <math.h>
#include <stdio.h>

/* Game function addresses (base 0x400000, no ASLR) */
#define GAME_BASE        0x400000
#define GFX_ROTATEY      0x457bb0  /* void Gfx_RotateY(float angle, float y, float z) */
#define GFX_SETPOSITION  0x457b50  /* void Gfx_SetPosition(float x, float y, float z) */
#define GFX_SCALEX       0x457b00  /* void Gfx_ScaleX(float scale) — approximate */

/* Game struct offsets */
#define BOARD_SCENE      0x878
#define SCENE_MESHWORLD  0x08
#define MW_VTX_COUNT     0x438
#define MW_VTX_ARRAY     0x440
#define VERTEX_SIZE      32
#define ENTITY_VTX_COUNT 272

#define ROTATION_SPEED   0.02f

/* Function pointer type for Gfx_RotateY */
typedef void (__cdecl *Gfx_RotateY_t)(float, float, float);
typedef void (__cdecl *Gfx_SetPosition_t)(float, float, float);

static float g_angle = 0.0f;
static float g_center_x = 0.0f;
static float g_center_y = 0.0f;
static float g_center_z = 0.0f;
static int g_initialized = 0;
static int g_update_count = 0;

/* Behavior_Init — called once when entity is detected */
__declspec(dllexport) void __cdecl Behavior_Init(void* transform, void* board_ptr) {
    DWORD board = (DWORD)board_ptr;
    if (!board) return;

    /* Get entity position from the S1 ref point (stored in transform by custom_entities.c) */
    /* The transform parameter is the EntityTransform — but we stored pos there */
    /* Actually, let's read the position from the vertex data instead */

    /* Navigate to MeshWorld vertex array to find entity center */
    if (IsBadReadPtr((void*)(board + BOARD_SCENE), 4)) return;
    DWORD scene = *(DWORD*)(board + BOARD_SCENE);
    if (!scene || scene < 0x10000) return;

    if (IsBadReadPtr((void*)(scene + SCENE_MESHWORLD), 4)) return;
    DWORD mw = *(DWORD*)(scene + SCENE_MESHWORLD);
    if (!mw || mw < 0x10000) return;

    if (IsBadReadPtr((void*)(mw + MW_VTX_COUNT), 4)) return;
    int total_vtx = *(int*)(mw + MW_VTX_COUNT);
    if (total_vtx < ENTITY_VTX_COUNT) return;

    if (IsBadReadPtr((void*)(mw + MW_VTX_ARRAY), 4)) return;
    float* vtx = *(float**)(mw + MW_VTX_ARRAY);
    if (!vtx || IsBadReadPtr(vtx, total_vtx * VERTEX_SIZE)) return;

    int start = total_vtx - ENTITY_VTX_COUNT;

    /* Compute centroid */
    float sx = 0, sy = 0, sz = 0;
    int i;
    for (i = 0; i < ENTITY_VTX_COUNT; i++) {
        float* v = vtx + (start + i) * (VERTEX_SIZE / 4);
        sx += v[0]; sy += v[1]; sz += v[2];
    }
    g_center_x = sx / ENTITY_VTX_COUNT;
    g_center_y = sy / ENTITY_VTX_COUNT;
    g_center_z = sz / ENTITY_VTX_COUNT;

    g_angle = 0.0f;
    g_initialized = 1;
    g_update_count = 0;

    char msg[256];
    snprintf(msg, sizeof(msg),
        "Rotator Init OK\n\ncenter=(%.1f, %.1f, %.1f)\ntotal_vtx=%d  start=%d",
        g_center_x, g_center_y, g_center_z, total_vtx, start);
    MessageBoxA(NULL, msg, "Rotator", MB_OK | MB_ICONINFORMATION);
}

/* Behavior_Update — called every frame (~60Hz) */
__declspec(dllexport) void __cdecl Behavior_Update(void* transform, void* board_ptr) {
    if (!g_initialized) return;

    g_update_count++;

    /* Increment rotation angle */
    g_angle += ROTATION_SPEED;
    if (g_angle > 6.283185f) g_angle -= 6.283185f;

    /* Call the game's Gfx_RotateY function directly.
     * This sets the world rotation matrix that the game uses for rendering.
     * Gfx_RotateY(angle, centerY, centerZ) builds a look-at matrix. */
    Gfx_RotateY_t rotateY = (Gfx_RotateY_t)GFX_ROTATEY;
    rotateY(g_angle, g_center_y, g_center_z);

    if (g_update_count == 60) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "Update #60\nangle=%.3f\ncenter=(%.1f, %.1f, %.1f)",
            g_angle, g_center_x, g_center_y, g_center_z);
        MessageBoxA(NULL, msg, "Rotator Update", MB_OK | MB_ICONINFORMATION);
    }
}

/* Behavior_Shutdown */
__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    g_initialized = 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
