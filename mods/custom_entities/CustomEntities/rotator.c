/*
 * rotator.c — Rotator behavior DLL for Custom Entities mod
 *
 * Makes the target entity constantly rotate on its X axis at a slow speed.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -luser32
 */

#include <windows.h>
#include <stdio.h>

/* EntityTransform — 0x50 bytes (RenderContext in game memory) */
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
    BYTE   hasTransform;/* +0x4C — TRUE = apply transform */
    BYTE   has_refl;    /* +0x4D — Has reflection */
    BYTE   pad[2];      /* +0x4E */
} EntityTransform;

#define ROTATION_SPEED 0.02f
static float g_accumulated_angle = 0.0f;
static int g_update_count = 0;
static EntityTransform* g_transform = NULL;

/* Behavior_Init — called once when entity is detected */
__declspec(dllexport) void __cdecl Behavior_Init(EntityTransform* transform, void* board) {
    if (!transform) return;
    g_transform = transform;

    char msg[256];
    snprintf(msg, sizeof(msg),
        "Rotator Behavior_Init\n\n"
        "transform = 0x%08X\n"
        "Before: rotX=%.3f rotY=%.3f rotZ=%.3f rotScale=%.3f\n"
        "        posX=%.3f posY=%.3f posZ=%.3f posScale=%.3f\n"
        "        hasTransform=%d  has_refl=%d\n"
        "        texture=0x%08X",
        (unsigned)transform,
        transform->rotX, transform->rotY, transform->rotZ, transform->rotScale,
        transform->posX, transform->posY, transform->posZ, transform->posScale,
        transform->hasTransform, transform->has_refl,
        transform->texture);
    MessageBoxA(NULL, msg, "Rotator Behavior_Init", MB_OK | MB_ICONINFORMATION);

    /* Force hasTransform=TRUE */
    transform->hasTransform = 1;

    /* Clear position and rotation */
    transform->posX = 0.0f;
    transform->posY = 0.0f;
    transform->posZ = 0.0f;
    transform->posScale = 1.0f;
    transform->rotX = 0.0f;
    transform->rotY = 0.0f;
    transform->rotZ = 0.0f;
    transform->rotScale = 1.0f;

    g_accumulated_angle = 0.0f;
    g_update_count = 0;
}

/* Behavior_Update — called every frame (~60Hz) */
__declspec(dllexport) void __cdecl Behavior_Update(EntityTransform* transform, void* board) {
    if (!transform) return;

    g_update_count++;

    /* Show a MessageBox after ~60 updates (~1 second) to confirm it's being called */
    if (g_update_count == 60) {
        char msg[256];
        snprintf(msg, sizeof(msg),
            "Rotator Behavior_Update called %d times!\n\n"
            "Current: rotX=%.3f hasTransform=%d\n"
            "transform=0x%08X",
            g_update_count, transform->rotX, transform->hasTransform,
            (unsigned)transform);
        MessageBoxA(NULL, msg, "Rotator Update OK", MB_OK | MB_ICONINFORMATION);
    }

    /* Ensure hasTransform stays TRUE */
    transform->hasTransform = 1;

    /* Increment rotation */
    g_accumulated_angle += ROTATION_SPEED;
    if (g_accumulated_angle > 6.283185f) {
        g_accumulated_angle -= 6.283185f;
    }

    transform->rotX = g_accumulated_angle;
    transform->rotY = 0.0f;
    transform->rotZ = 0.0f;
    transform->rotScale = 1.0f;
    transform->posX = 0.0f;
    transform->posY = 0.0f;
    transform->posZ = 0.0f;
    transform->posScale = 1.0f;
}

/* Behavior_Shutdown — called when level unloads */
__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
    MessageBoxA(NULL, "Rotator Behavior_Shutdown", "Rotator", MB_OK);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        MessageBoxA(NULL, "Rotator.dll loaded!", "Rotator DLL", MB_OK | MB_ICONINFORMATION);
    }
    return TRUE;
}
