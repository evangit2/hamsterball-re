/*
 * cerotator.c — CErotator behavior DLL (v4)
 *
 * Writes rotation angle to EntityTransform.rotY every frame.
 * The game's render path reads EntityTransform fields and applies
 * them as a D3D world matrix via SetTransform.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o CErotator.dll cerotator.c \
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

static float g_speed = 0.02f;
static int g_axis = 1; /* 0=x, 1=y, 2=z */

static void load_config(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                      | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCSTR)&load_config, &hSelf);
    if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH) > 0) {
        char *p = strrchr(path, '\\');
        if (p) {
            *p = '\0';
            char *p2 = strrchr(path, '\\');
            if (p2) {
                *p2 = '\0';
                snprintf(path, MAX_PATH, "%s\\cerotator_config.txt", path);
            }
        }
    }

    FILE *f = NULL;
    fopen_s(&f, path, "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (sscanf(line, "speed=%f", &g_speed) == 1) {}
            else if (sscanf(line, "axis=%c", (char*)&g_axis) == 1) {
                if (g_axis == 'x' || g_axis == 'X') g_axis = 0;
                else if (g_axis == 'y' || g_axis == 'Y') g_axis = 1;
                else if (g_axis == 'z' || g_axis == 'Z') g_axis = 2;
            }
        }
        fclose(f);
    }
}

__declspec(dllexport) void __cdecl Behavior_Init(void* transform, void* board) {
    g_angle = 0.0f;
    g_count = 0;
    load_config();
}

__declspec(dllexport) void __cdecl Behavior_Update(void* transform, void* board) {
    EntityTransform* t = (EntityTransform*)transform;
    if (!t) return;
    if (IsBadReadPtr(t, sizeof(EntityTransform))) return;

    g_angle += g_speed;
    if (g_angle > 6.283185f) g_angle -= 6.283185f;

    switch (g_axis) {
        case 0: t->rotX = g_angle; break;
        case 1: t->rotY = g_angle; break;
        case 2: t->rotZ = g_angle; break;
    }

    t->rotScale = 1.0f;
    t->posScale = 1.0f;

    g_count++;
}

__declspec(dllexport) void __cdecl Behavior_Shutdown(void) {
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
