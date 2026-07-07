/*
 * rotator.c — Rotator behavior DLL (minimal)
 *
 * Simply writes rotation angle to EntityTransform.rotX every frame.
 * Even if the game caches the transform, we keep writing in case
 * any code path re-reads it.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o Rotator.dll rotator.c \
 *     -O2 -static -static-libgcc -msse2 -mfpmath=sse -lm -luser32
 */

#include <windows.h>
#include <math.h>
#include <stdio.h>

typedef struct {
    DWORD  vtable;      /* +0x00 */
    float  rotX;        /* +0x04 */
    float  rotY;        /* +0x08 */
    float  rotZ;        /* +0x0C */
    float  rotScale;    /* +0x10 */
    float  posX;        /* +0x14 */
    float  posY;        /* +0x18 */
    float  posZ;        /* +0x1C */
    float  posScale;    /* +0x20 */
    float  extra[12];  /* +0x24 - +0x4F */
} EntityTransform;

static float g_angle = 0.0f;
static int g_count = 0;

__declspec(dllexport) void __cdecl Behavior_Init(void* transform, void* board) {
    g_angle = 0.0f;
    g_count = 0;
}

__declspec(dllexport) void __cdecl Behavior_Update(void* transform, void* board) {
    EntityTransform* t = (EntityTransform*)transform;
    if (!t) return;

    g_angle += 0.02f;
    if (g_angle > 6.283185f) g_angle -= 6.283185f;

    /* Write rotation to every possible field */
    t->rotX = g_angle;
    t->rotScale = 1.0f;  /* keep scale at 1.0 */
}

__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
