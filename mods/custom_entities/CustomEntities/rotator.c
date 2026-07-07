/*
 * rotator.c — Rotator behavior DLL for Custom Entities mod
 *
 * Makes the target entity constantly rotate on its X axis at a slow speed.
 *
 * Entity name: CE:Rotator (or E:Rotator, N:Rotator)
 * DLL file:    CustomEntities/Rotator.dll
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse
 */

#include <windows.h>

/* EntityTransform — 0x50 bytes, same layout as RenderContext in game memory.
 *
 * The material block's first 8 floats serve DUAL PURPOSE in the game:
 *   set1 (ambient): posX, posY, posZ, posScale   (+0x14 to +0x24)
 *   set2 (diffuse): rotX, rotY, rotZ, rotScale   (+0x04 to +0x14)
 *
 * The hasTransform flag at +0x4C is set during file loading:
 *   hasTransform = (rotScale != 1.0)
 * When hasTransform=FALSE (rotScale==1.0), the game ignores ALL transform data.
 * The mesh renders with its material colors but no position/rotation offset.
 *
 * To enable runtime rotation, Behavior_Init must:
 *   1. Force hasTransform=TRUE (write 1 to offset 0x4C)
 *   2. Clear position and rotation to (0,0,0,1.0) to prevent the file's
 *      transform values from being applied
 *   3. Set rotScale to 1.0 (scale=1, normal rendering)
 *
 * Then Behavior_Update writes rotX each frame, and the game applies it.
 */
typedef struct {
    DWORD  vtable;      /* +0x00 */
    float  rotX;        /* +0x04 — Rotation X (radians) */
    float  rotY;        /* +0x08 — Rotation Y (radians) */
    float  rotZ;        /* +0x0C — Rotation Z (radians) */
    float  rotScale;    /* +0x10 — Also diffuse alpha; !=1.0 → hasTransform=TRUE */
    float  posX;        /* +0x14 — Also ambient R */
    float  posY;        /* +0x18 — Also ambient G */
    float  posZ;        /* +0x1C — Also ambient B */
    float  posScale;    /* +0x20 — Also ambient alpha */
    float  specular[4]; /* +0x24 — Specular RGBA */
    float  emissive[4]; /* +0x34 — Emissive RGBA */
    float  power;       /* +0x44 — Material power/shininess */
    DWORD  texture;     /* +0x48 — Texture pointer */
    BYTE   hasTransform;/* +0x4C — TRUE = apply transform, FALSE = ignore */
    BYTE   has_refl;    /* +0x4D — Has reflection */
    BYTE   pad[2];      /* +0x4E — Padding to 0x50 */
} EntityTransform;

/* Rotation speed: ~0.02 radians per frame ≈ 1.15°/frame at 60fps ≈ 69°/sec */
#define ROTATION_SPEED 0.02f

/* Accumulated rotation (stored between Update calls) */
static float g_accumulated_angle = 0.0f;

/* Behavior_Init — called once when entity is detected */
__declspec(dllexport) void __cdecl Behavior_Init(EntityTransform* transform, void* board) {
    if (!transform) return;

    /* Force hasTransform=TRUE so the game applies our rotation */
    transform->hasTransform = 1;

    /* Clear position to (0,0,0) — no offset, vertices are already in world space */
    transform->posX = 0.0f;
    transform->posY = 0.0f;
    transform->posZ = 0.0f;
    transform->posScale = 1.0f;

    /* Clear rotation to (0,0,0) — no initial rotation */
    transform->rotX = 0.0f;
    transform->rotY = 0.0f;
    transform->rotZ = 0.0f;
    transform->rotScale = 1.0f;

    /* Reset accumulated angle */
    g_accumulated_angle = 0.0f;
}

/* Behavior_Update — called every frame (~60Hz) */
__declspec(dllexport) void __cdecl Behavior_Update(EntityTransform* transform, void* board) {
    if (!transform) return;

    /* Ensure hasTransform stays TRUE (in case game resets it) */
    transform->hasTransform = 1;

    /* Increment rotation angle */
    g_accumulated_angle += ROTATION_SPEED;

    /* Wrap at 2π to avoid float precision loss after long rotation */
    if (g_accumulated_angle > 6.283185f) {
        g_accumulated_angle -= 6.283185f;
    }

    /* Write the new rotation angle to the entity's X-axis rotation */
    transform->rotX = g_accumulated_angle;

    /* Keep Y and Z at zero — only rotate on X axis */
    transform->rotY = 0.0f;
    transform->rotZ = 0.0f;
    transform->rotScale = 1.0f;

    /* Keep position at origin — no offset */
    transform->posX = 0.0f;
    transform->posY = 0.0f;
    transform->posZ = 0.0f;
    transform->posScale = 1.0f;
}

/* Behavior_Shutdown — called when level unloads */
__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    /* Nothing to clean up */
}

/* DllMain — required for DLL */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
