/*
 * AnimatedTextures_MinGW.cpp — Cycle between numbered texture variants at a custom framerate.
 *
 * HB+ port of animated_textures bass.dll proxy mod.
 * Scans the Graphics texture cache for textures named baseNN.png (e.g. arrowanim01.png),
 * loads additional numbered frames, and swaps the D3D texture pointer at runtime.
 *
 * Config format (e.g. Textures/arrowanim.txt):
 *   framerate = 0.5    (seconds between frame swaps)
 *   looptype = 1       (0=play once, 1=loop, 2=ping-pong)
 *
 * Texture naming: baseNN.png (e.g. arrowanim01.png, arrowanim02.png)
 *
 * Uses nocrt + manual 17-entry vtable for HB+ v2.1 compatibility.
 */
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

/* ── Constants ──────────────────────────────────────────────────────── */

#define APP_PTR           0x005341E0
#define OFF_APP_GRAPHICS  0x174
#define GFX_TEX_COUNT     0x2E8
#define GFX_TEX_ARRAY     0x6F0
#define TEX_OBJ_D3D       0x04
#define TEX_OBJ_NAME      0x08

#define MAX_ANIMATIONS  16
#define MAX_FRAMES      32

/* LoadTexture: __thiscall(void* gfx, char* filename, char search_cache)
 * MinGW C lacks __thiscall, so use __fastcall with a dummy EDX param. */
typedef void* (__fastcall *LoadTexture_t)(void* gfx, void* dummy, const char* name, int search_cache);
static LoadTexture_t game_LoadTexture = (LoadTexture_t)0x00455C50;

/* ── Data structures ───────────────────────────────────────────────── */

typedef struct {
    DWORD d3dTexObjAddr;
    DWORD originalD3DTex;
    DWORD frameTextures[MAX_FRAMES];
    int frameCount;
    float framerate;
    int looptype;
    int currentFrame;
    int direction;
    double lastSwapTime;
    char baseName[64];
} AnimTexture;

static AnimTexture g_anims[MAX_ANIMATIONS];
static int g_animCount = 0;
static char g_gameDir[MAX_PATH] = "";
static bool g_pathReady = false;

/* Track whether we were in a level last frame */
static int g_wasInLevel = 0;
static int g_scanTimer = 0;

static void* g_storedApi = NULL;

/* ── Time helper ───────────────────────────────────────────────────── */

static double getTime(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

/* ── Config path: build relative to THIS DLL's folder ──────────────── */

static void buildGameDir(void) {
    MEMORY_BASIC_INFORMATION mbi;
    char dllPath[MAX_PATH];
    if (VirtualQuery((void*)buildGameDir, &mbi, sizeof(mbi)) > 0) {
        HMODULE hMod = (HMODULE)mbi.AllocationBase;
        if (hMod && GetModuleFileNameA(hMod, dllPath, MAX_PATH) > 0) {
            char* last = NULL;
            char* p = dllPath;
            while (*p) {
                if (*p == '\\' || *p == '/') last = p;
                p++;
            }
            if (last) {
                *(last + 1) = '\0';
                /* Go up one directory (from Mods\ to game root) */
                /* The DLL is in Mods\, game dir is parent */
                char* last2 = NULL;
                char* p2 = dllPath;
                while (*p2) {
                    if (*p2 == '\\' || *p2 == '/') last2 = p2;
                    p2++;
                }
                if (last2) {
                    *last2 = '\0';
                }
                nc_strncpy(g_gameDir, dllPath, MAX_PATH - 1);
                g_gameDir[MAX_PATH - 1] = '\0';
                g_pathReady = true;
                return;
            }
        }
    }
    GetCurrentDirectoryA(MAX_PATH, g_gameDir);
    g_pathReady = true;
}

/* ── Frame name parsing ───────────────────────────────────────────── */

static int nc_isdigit(char c) { return (c >= '0' && c <= '9'); }

static int nc_atoi(const char* s) {
    int val = 0;
    while (nc_isdigit(*s)) { val = val * 10 + (*s - '0'); s++; }
    return val;
}

static int nc_stricmp(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (*a >= 'A' && *a <= 'Z') ? (*a + 32) : *a;
        char cb = (*b >= 'A' && *b <= 'Z') ? (*b + 32) : *b;
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

/* Extract base name and frame number from "arrowanim01.png"
 * Returns frame number (1+), or 0 if not a frame name */
static int parseFrameName(const char* filename, char* outBase, int baseMaxLen) {
    const char* dot = filename;
    const char* p = filename;
    while (*p) { if (*p == '.') dot = p; p++; }
    if (dot == filename) return 0;
    int nameLen = (int)(dot - filename);
    if (nameLen < 3) return 0;
    int digitStart = nameLen - 1;
    while (digitStart >= 0 && nc_isdigit(filename[digitStart])) digitStart--;
    digitStart++;
    int digitCount = nameLen - digitStart;
    if (digitCount < 1 || digitCount > 4) return 0;
    int frameNum = nc_atoi(filename + digitStart);
    if (frameNum < 1) return 0;
    int baseLen = digitStart;
    if (baseLen == 0 || baseLen >= baseMaxLen) return 0;
    nc_strncpy(outBase, filename, baseLen);
    outBase[baseLen] = 0;
    return frameNum;
}

/* ── Config file loading ──────────────────────────────────────────── */

static void loadConfig(const char* baseName, float* outFramerate, int* outLooptype) {
    *outFramerate = 0.5f;
    *outLooptype = 1;
    char path[MAX_PATH];
    nc_snprintf(path, MAX_PATH, "%sTextures\\%s.txt", g_gameDir, baseName);
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    char buf[4096];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    /* Parse line by line */
    char* p = buf;
    while (*p) {
        char* nl = p;
        while (*nl && *nl != '\n') nl++;
        char saved = *nl;
        *nl = '\0';

        /* Parse "key = value" */
        char* eq = p;
        while (*eq && *eq != '=') eq++;
        if (*eq == '=') {
            *eq = '\0';
            char* key = p;
            char* val = eq + 1;
            /* Trim whitespace from key */
            while (*key == ' ' || *key == '\t') key++;
            char* kend = key + nc_strlen(key) - 1;
            while (kend > key && (*kend == ' ' || *kend == '\t' || *kend == '\r')) *kend-- = '\0';
            /* Trim whitespace from val */
            while (*val == ' ' || *val == '\t') val++;

            if (nc_stricmp(key, "framerate") == 0) {
                /* Manual float parse */
                float fv = 0.0f;
                int neg = 0;
                if (*val == '-') { neg = 1; val++; }
                int ip = 0;
                while (nc_isdigit(*val)) { ip = ip * 10 + (*val - '0'); val++; }
                float fr = 0.0f;
                if (*val == '.') {
                    val++;
                    float div = 10.0f;
                    while (nc_isdigit(*val)) { fr += (*val - '0') / div; div *= 10.0f; val++; }
                }
                fv = (float)ip + fr;
                if (neg) fv = -fv;
                *outFramerate = fv;
            } else if (nc_stricmp(key, "looptype") == 0) {
                *outLooptype = nc_atoi(val);
            }
        }

        *nl = saved;
        if (*nl == '\n') nl++;
        p = nl;
    }
}

/* ── Count frames for a base name ──────────────────────────────────── */

static int countFrames(const char* baseName) {
    char searchPath[MAX_PATH];
    nc_snprintf(searchPath, MAX_PATH, "%sTextures\\%s*.png", g_gameDir, baseName);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    int validCount = 0;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char dummyBase[64];
        if (parseFrameName(fd.cFileName, dummyBase, 64) > 0) {
            if (nc_stricmp(dummyBase, baseName) == 0) validCount++;
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    return validCount;
}

/* ── Setup animations: scan texture cache ──────────────────────────── */

static void trySetupAnimations(void) {
    DWORD appPtr = *(DWORD*)APP_PTR;
    if (!appPtr || appPtr < 0x10000) return;
    if (IsBadReadPtr((void*)appPtr, 4)) return;
    if (IsBadReadPtr((void*)(appPtr + OFF_APP_GRAPHICS), 4)) return;
    DWORD graphics = *(DWORD*)(appPtr + OFF_APP_GRAPHICS);
    if (!graphics || IsBadReadPtr((void*)graphics, 4)) return;
    if (IsBadReadPtr((void*)(graphics + GFX_TEX_COUNT), 4)) return;
    int cacheCount = *(int*)(graphics + GFX_TEX_COUNT);
    if (cacheCount < 1) return;
    if (IsBadReadPtr((void*)(graphics + GFX_TEX_ARRAY), 4)) return;
    DWORD arrPtr = *(DWORD*)(graphics + GFX_TEX_ARRAY);
    if (!arrPtr || IsBadReadPtr((void*)arrPtr, cacheCount * 4)) return;

    for (int i = 0; i < cacheCount && g_animCount < MAX_ANIMATIONS; i++) {
        DWORD entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x20)) continue;
        DWORD namePtr = *(DWORD*)(entry + TEX_OBJ_NAME);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) continue;
        const char* texName = (const char*)namePtr;

        char baseName[64];
        int frameNum = parseFrameName(texName, baseName, 64);
        if (frameNum != 1) continue;

        /* Dedup check */
        int already = 0;
        for (int j = 0; j < g_animCount; j++) {
            if (nc_stricmp(g_anims[j].baseName, baseName) == 0) { already = 1; break; }
        }
        if (already) continue;

        /* Only animate if a .txt config file exists for this base name */
        char configPath[MAX_PATH];
        nc_snprintf(configPath, MAX_PATH, "%sTextures\\%s.txt", g_gameDir, baseName);
        DWORD attr = GetFileAttributesA(configPath);
        if (attr == INVALID_FILE_ATTRIBUTES) continue;

        int totalFrames = countFrames(baseName);
        if (totalFrames < 2) continue;

        if (IsBadReadPtr((void*)(entry + TEX_OBJ_D3D), 4)) continue;
        DWORD originalTex = *(DWORD*)(entry + TEX_OBJ_D3D);
        if (!originalTex) continue;

        AnimTexture* a = &g_anims[g_animCount];
        a->d3dTexObjAddr = entry;
        a->originalD3DTex = originalTex;
        a->frameTextures[0] = originalTex;
        a->frameCount = 1;
        a->currentFrame = 0;
        a->direction = 1;
        a->lastSwapTime = getTime();
        nc_strncpy(a->baseName, baseName, 63);
        a->baseName[63] = 0;
        loadConfig(baseName, &a->framerate, &a->looptype);

        for (int f = 2; f <= totalFrames && f <= MAX_FRAMES; f++) {
            char frameName[128];
            nc_snprintf(frameName, sizeof(frameName), "%s%02d.png", baseName, f);
            void* texObj = game_LoadTexture((void*)graphics, NULL, frameName, 1);
            if (texObj && !IsBadReadPtr(texObj, 0x10)) {
                DWORD d3dTex = *(DWORD*)((char*)texObj + TEX_OBJ_D3D);
                if (d3dTex) {
                    a->frameTextures[a->frameCount] = d3dTex;
                    a->frameCount++;
                }
            }
        }

        if (a->frameCount < 2) continue;
        g_animCount++;
    }
}

static void restoreTextures(void) {
    for (int i = 0; i < g_animCount; i++) {
        AnimTexture* a = &g_anims[i];
        if (a->d3dTexObjAddr && !IsBadWritePtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4)) {
            *(DWORD*)(a->d3dTexObjAddr + TEX_OBJ_D3D) = a->originalD3DTex;
        }
    }
    g_animCount = 0;
}

static void doAnimationSwap(void) {
    if (g_animCount == 0) return;

    double now = getTime();
    for (int i = 0; i < g_animCount; i++) {
        AnimTexture* a = &g_anims[i];
        if (a->frameCount < 2) continue;
        if (!a->d3dTexObjAddr) continue;
        if (IsBadReadPtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4)) {
            restoreTextures();
            break;
        }
        double elapsed = now - a->lastSwapTime;
        if (elapsed < (double)a->framerate) continue;

        a->lastSwapTime = now;
        int next = a->currentFrame + a->direction;

        if (a->looptype == 0) {
            if (next >= a->frameCount) next = a->frameCount - 1;
            else if (next < 0) next = 0;
        } else if (a->looptype == 1) {
            if (next >= a->frameCount) next = 0;
            else if (next < 0) next = a->frameCount - 1;
        } else { /* ping-pong */
            if (next >= a->frameCount) {
                a->direction = -1;
                next = a->frameCount - 2;
                if (next < 0) next = 0;
            } else if (next < 0) {
                a->direction = 1;
                next = 1;
                if (next >= a->frameCount) next = a->frameCount - 1;
            }
        }

        a->currentFrame = next;
        DWORD newTex = a->frameTextures[next];
        if (newTex && !IsBadWritePtr((void*)(a->d3dTexObjAddr + TEX_OBJ_D3D), 4)) {
            *(DWORD*)(a->d3dTexObjAddr + TEX_OBJ_D3D) = newTex;
        }
    }
}

/* ── Check if in level via HB+ API ──────────────────────────────────── */

static int checkInLevel(void) {
    if (!g_storedApi) {
        /* Fallback: check App pointer directly */
        DWORD appPtr = *(DWORD*)APP_PTR;
        if (appPtr && appPtr > 0x10000 && !IsBadReadPtr((void*)appPtr, 0x200)) {
            DWORD scene = *(DWORD*)(appPtr + 0x178);
            return (scene && scene > 0x10000) ? 1 : 0;
        }
        return 0;
    }
    HBPlusAPI hb = { g_storedApi };
    Scene* scene = hb.GetScene();
    if (scene && !IsBadReadPtr(scene, 4)) return 1;
    return 0;
}

/* ── Vtable implementations ────────────────────────────────────────── */

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) nc_free(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Animated Textures"; }
static const char* __thiscall get_author(void*) { return "MAKYUNI"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return "Ported to HB+ by Hamsterbot"; }

static void __thiscall init_impl(void* thisptr, void* modApi) {
    *(void**)((char*)thisptr + 4) = modApi;
    g_storedApi = modApi;
    buildGameDir();
}

static void __thiscall game_update_impl(void* thisptr) {
    if (!g_pathReady) buildGameDir();

    int inLevel = checkInLevel();

    if (!inLevel && g_wasInLevel) {
        restoreTextures();
        g_wasInLevel = 0;
        g_scanTimer = 0;
    }
    if (inLevel && !g_wasInLevel) {
        g_wasInLevel = 1;
        g_scanTimer = 0;
    }

    /* Periodically try to set up animations after entering a level */
    if (inLevel && g_animCount == 0) {
        g_scanTimer++;
        if (g_scanTimer >= 60) {
            g_scanTimer = 0;
            trySetupAnimations();
        }
    }

    /* Animation swapping */
    doAnimationSwap();
}

/* No-op implementations for unused callbacks */
static void __thiscall ball_update_impl(void*, void*) {}
static void __thiscall render_apply_impl(void*, void*, float*) {}
static void __thiscall button_toggle_impl(void*, const char*, bool) {}
static void __thiscall slider_change_impl(void*, const char*, float) {}
static void __thiscall cycle_change_impl(void*, const char*, const char*) {}
static void __thiscall event_collide_impl(void*, void*, char*) {}
static void __thiscall text_render_impl(void*) {}
static void __thiscall ball_bump_impl(void*, void*, void*) {}
static void __thiscall level_start_impl(void*) {}
static void __thiscall scene_end_impl(void*) {
    restoreTextures();
    g_wasInLevel = 0;
    g_scanTimer = 0;
}

static void* g_vtable[17] = {
    (void*)sc_dtor,              // [0]
    (void*)get_mod_name,         // [1]
    (void*)get_author,           // [2]
    (void*)get_version,          // [3]
    (void*)get_contributors,     // [4]
    (void*)init_impl,            // [5]
    (void*)ball_update_impl,     // [6]
    (void*)render_apply_impl,    // [7]
    (void*)button_toggle_impl,   // [8]
    (void*)slider_change_impl,   // [9]
    (void*)cycle_change_impl,    // [10]
    (void*)game_update_impl,     // [11]
    (void*)event_collide_impl,   // [12]
    (void*)text_render_impl,     // [13]
    (void*)ball_bump_impl,       // [14]
    (void*)scene_end_impl,       // [15]
    (void*)level_start_impl,     // [16]
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = nc_malloc(8);
    if (!obj) return NULL;
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
