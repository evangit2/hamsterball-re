/*
 * rotator.c — Rotator behavior DLL for Custom Entities mod
 *
 * Makes the target entity constantly rotate on its X axis at a slow speed.
 *
 * Entity name: E:Rotator (or N:Rotator)
 * DLL file:    Behaviours/Rotator.dll
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse
 */

#include <windows.h>

/* EntityTransform — 0x50 bytes, same layout as in entity_api.h */
typedef struct {
    DWORD  vtable;      /* +0x00 */
    float  rotX;        /* +0x04 — Rotation X (radians) */
    float  rotY;        /* +0x08 — Rotation Y (radians) */
    float  rotZ;        /* +0x0C — Rotation Z (radians) */
    float  rotScale;    /* +0x10 */
    float  posX;        /* +0x14 */
    float  posY;        /* +0x18 */
    float  posZ;        /* +0x1C */
    float  posScale;    /* +0x20 */
    float  extra[12];  /* +0x24 - +0x4F */
} EntityTransform;

/* Rotation speed: ~0.02 radians per frame ≈ 1.15°/frame at 60fps ≈ 69°/sec */
#define ROTATION_SPEED 0.02f

/* Accumulated rotation (stored between Update calls) */
static float g_accumulated_angle = 0.0f;

/* Behavior_Init — called once when entity is detected */
__declspec(dllexport) void __cdecl Behavior_Init(EntityTransform* transform, void* board) {
    if (!transform) return;

    /* Initialize rotation to 0 */
    g_accumulated_angle = 0.0f;
    transform->rotX = 0.0f;
}

/* Behavior_Update — called every frame (~60Hz) */
__declspec(dllexport) void __cdecl Behavior_Update(EntityTransform* transform, void* board) {
    if (!transform) return;

    /* Increment rotation angle */
    g_accumulated_angle += ROTATION_SPEED;

    /* Wrap at 2π to avoid float precision loss after long rotation */
    if (g_accumulated_angle > 6.283185f) {
        g_accumulated_angle -= 6.283185f;
    }

    /* Write the new rotation angle to the entity's X-axis rotation */
    transform->rotX = g_accumulated_angle;
}

/* Behavior_Shutdown — called when level unloads */
__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    /* Nothing to clean up */
}

/* DllMain — required for DLL */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
