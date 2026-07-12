/*
 * animated_textures — Cycle between numbered texture variants at a custom framerate.
 *
 * Scans Textures/ for .txt config files containing "framerate". Each config's
 * base name (filename without .txt) defines an animation prefix. The mod then
 * scans the Graphics texture cache for textures named baseNN.png (e.g.
 * arrowanim01.png), loads additional numbered frames, and swaps the D3D
 * texture pointer at runtime.
 *
 * Config format (e.g. Textures/arrowanim.txt):
 *   framerate = 0.5    (seconds between frame swaps)
 *   looptype = 1       (0=play once, 1=loop, 2=ping-pong)
 *
 * Texture naming: baseNN.png (e.g. arrowanim01.png, arrowanim02.png)
 * NO underscore between base name and number.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── BASS proxy exports (forward to bass_real.dll) ─────────────────── */

static HMODULE g_hRealBass = NULL;

typedef int  (__stdcall *BASS_ChannelSetAttributes_t)(DWORD, DWORD, int, int);
static BASS_ChannelSetAttributes_t real_BASS_ChannelSetAttributes = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelSetAttributes(DWORD a, DWORD b, int c, int d) {
    if (real_BASS_ChannelSetAttributes) return real_BASS_ChannelSetAttributes(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_MusicPlayEx_t)(DWORD, DWORD, int, DWORD);
static BASS_MusicPlayEx_t real_BASS_MusicPlayEx = NULL;
__declspec(dllexport) int __stdcall BASS_MusicPlayEx(DWORD a, DWORD b, int c, DWORD d) {
    if (real_BASS_MusicPlayEx) return real_BASS_MusicPlayEx(a, b, c, d);
    return 1;
}
typedef int  (__stdcall *BASS_SetConfig_t)(DWORD, DWORD);
static BASS_SetConfig_t real_BASS_SetConfig = NULL;
__declspec(dllexport) int __stdcall BASS_SetConfig(DWORD a, DWORD b) {
    if (real_BASS_SetConfig) return real_BASS_SetConfig(a, b);
    return 1;
}
typedef int  (__stdcall *BASS_Init_t)(int, DWORD, DWORD, HWND, void*);
static BASS_Init_t real_BASS_Init = NULL;
__declspec(dllexport) int __stdcall BASS_Init(int a, DWORD b, DWORD c, HWND d, void* e) {
    if (real_BASS_Init) return real_BASS_Init(a, b, c, d, e);
    return 1;
}
typedef int  (__stdcall *BASS_Free_t)(void);
static BASS_Free_t real_BASS_Free = NULL;
__declspec(dllexport) int __stdcall BASS_Free(void) {
    if (real_BASS_Free) return real_BASS_Free();
    return 1;
}
typedef int  (__stdcall *BASS_Start_t)(void);
static BASS_Start_t real_BASS_Start = NULL;
__declspec(dllexport) int __stdcall BASS_Start(void) {
    if (real_BASS_Start) return real_BASS_Start();
    return 1;
}
typedef int  (__stdcall *BASS_Stop_t)(void);
static BASS_Stop_t real_BASS_Stop = NULL;
__declspec(dllexport) int __stdcall BASS_Stop(void) {
    if (real_BASS_Stop) return real_BASS_Stop();
    return 1;
}
typedef int  (__stdcall *BASS_ErrorGetCode_t)(void);
static BASS_ErrorGetCode_t real_BASS_ErrorGetCode = NULL;
__declspec(dllexport) int __stdcall BASS_ErrorGetCode(void) {
    if (real_BASS_ErrorGetCode) return real_BASS_ErrorGetCode();
    return 0;
}
typedef DWORD (__stdcall *BASS_MusicLoad_t)(int, void*, DWORD, DWORD, DWORD, DWORD);
static BASS_MusicLoad_t real_BASS_MusicLoad = NULL;
__declspec(dllexport) DWORD __stdcall BASS_MusicLoad(int a, void* b, DWORD c, DWORD d, DWORD e, DWORD f) {
    if (real_BASS_MusicLoad) return real_BASS_MusicLoad(a, b, c, d, e, f);
    return 1;
}
typedef int  (__stdcall *BASS_ChannelStop_t)(DWORD);
static BASS_ChannelStop_t real_BASS_ChannelStop = NULL;
__declspec(dllexport) int __stdcall BASS_ChannelStop(DWORD a) {
    if (real_BASS_ChannelStop) return real_BASS_ChannelStop(a);
    return 1;
}

static void init_bass_proxy(void) {
    g_hRealBass = LoadLibraryA("bass_real.dll");
    if (!g_hRealBass) return;
    real_BASS_ChannelSetAttributes = (BASS_ChannelSetAttributes_t)GetProcAddress(g_hRealBass, "BASS_ChannelSetAttributes");
    real_BASS_MusicPlayEx         = (BASS_MusicPlayEx_t)GetProcAddress(g_hRealBass, "BASS_MusicPlayEx");
    real_BASS_SetConfig           = (BASS_SetConfig_t)GetProcAddress(g_hRealBass, "BASS_SetConfig");
    real_BASS_Init                = (BASS_Init_t)GetProcAddress(g_hRealBass, "BASS_Init");
    real_BASS_Free                = (BASS_Free_t)GetProcAddress(g_hRealBass, "BASS_Free");
    real_BASS_Start               = (BASS_Start_t)GetProcAddress(g_hRealBass, "BASS_Start");
    real_BASS_Stop                = (BASS_Stop_t)GetProcAddress(g_hRealBass, "BASS_Stop");
    real_BASS_ErrorGetCode        = (BASS_ErrorGetCode_t)GetProcAddress(g_hRealBass, "BASS_ErrorGetCode");
    real_BASS_MusicLoad           = (BASS_MusicLoad_t)GetProcAddress(g_hRealBass, "BASS_MusicLoad");
    real_BASS_ChannelStop         = (BASS_ChannelStop_t)GetProcAddress(g_hRealBass, "BASS_ChannelStop");
}

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
static int g_running = 1;
static char g_gameDir[MAX_PATH] = "";

static double getTime(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

static void getGameDir(void) {
    HMODULE hSelf = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&getGameDir, &hSelf) && hSelf) {
        GetModuleFileNameA(hSelf, g_gameDir, MAX_PATH);
        char* slash = strrchr(g_gameDir, '\\');
        if (slash) { slash[1] = '\0'; return; }
    }
    GetCurrentDirectoryA(MAX_PATH, g_gameDir);
    size_t len = strlen(g_gameDir);
    if (len > 0 && g_gameDir[len - 1] != '\\') strcat(g_gameDir, "\\");
}

/* Extract base name and frame number from "arrowanim01.png"
 * Returns frame number (1+), or 0 if not a frame name */
static int parseFrameName(const char* filename, char* outBase, int baseMaxLen) {
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;
    int nameLen = (int)(dot - filename);
    if (nameLen < 3) return 0;
    int digitStart = nameLen - 1;
    while (digitStart >= 0 && isdigit((unsigned char)filename[digitStart])) digitStart--;
    digitStart++;
    int digitCount = nameLen - digitStart;
    if (digitCount < 1 || digitCount > 4) return 0;
    int frameNum = atoi(filename + digitStart);
    if (frameNum < 1) return 0;
    int baseLen = digitStart;
    if (baseLen == 0 || baseLen >= baseMaxLen) return 0;
    strncpy(outBase, filename, baseLen);
    outBase[baseLen] = 0;
    return frameNum;
}

static void loadConfig(const char* baseName, float* outFramerate, int* outLooptype) {
    *outFramerate = 0.5f;
    *outLooptype = 1;
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%sTextures\\%s.txt", g_gameDir, baseName);
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char key[128], val[128];
        if (sscanf(line, "%127[^=]=%127s", key, val) == 2) {
            char* k = key;
            while (*k == ' ' || *k == '\t') k++;
            char* end = k + strlen(k) - 1;
            while (end > k && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) *end-- = 0;
            if (_stricmp(k, "framerate") == 0) *outFramerate = (float)atof(val);
            else if (_stricmp(k, "looptype") == 0) *outLooptype = atoi(val);
        }
    }
    fclose(f);
}

static int countFrames(const char* baseName) {
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%sTextures\\%s*.png", g_gameDir, baseName);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    int validCount = 0;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char dummyBase[64];
        if (parseFrameName(fd.cFileName, dummyBase, 64) > 0) {
            if (_stricmp(dummyBase, baseName) == 0) validCount++;
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    return validCount;
}

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
            if (_stricmp(g_anims[j].baseName, baseName) == 0) { already = 1; break; }
        }
        if (already) continue;

        /* Only animate if a .txt config file exists for this base name.
         * This ensures only AT: prefixed objects (which have .txt configs)
         * are treated as animated — not stock textures like Title02.png */
        char configPath[MAX_PATH];
        snprintf(configPath, MAX_PATH, "%sTextures\\%s.txt", g_gameDir, baseName);
        DWORD attr = GetFileAttributesA(configPath);
        if (attr == INVALID_FILE_ATTRIBUTES) continue; /* no .txt = not animated */

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
        strncpy(a->baseName, baseName, 63);
        a->baseName[63] = 0;
        loadConfig(baseName, &a->framerate, &a->looptype);

        for (int f = 2; f <= totalFrames && f <= MAX_FRAMES; f++) {
            char frameName[128];
            snprintf(frameName, sizeof(frameName), "%s%02d.png", baseName, f);
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

/* ── Main thread ───────────────────────────────────────────────────── */

static DWORD WINAPI animThread(LPVOID param) {
    (void)param;
    Sleep(3000);
    int scanTimer = 0;
    /* Track whether we were in a level last frame */
    int wasInLevel = 0;

    while (g_running) {
        Sleep(16);

        /* Check if we're in a level by reading App+0x178 (scene ptr) */
        DWORD appPtr = *(DWORD*)APP_PTR;
        int inLevel = 0;
        if (appPtr && appPtr > 0x10000 && !IsBadReadPtr((void*)appPtr, 0x200)) {
            DWORD scene = *(DWORD*)(appPtr + 0x178);
            inLevel = (scene && scene > 0x10000) ? 1 : 0;
        }

        if (!inLevel && wasInLevel) {
            restoreTextures();
            wasInLevel = 0;
            scanTimer = 0;
        }
        if (inLevel && !wasInLevel) {
            wasInLevel = 1;
            scanTimer = 0;
        }

        /* Periodically try to set up animations after entering a level */
        if (inLevel && g_animCount == 0) {
            scanTimer++;
            if (scanTimer >= 60) {
                scanTimer = 0;
                trySetupAnimations();
            }
        }

        /* Animation swapping */
        if (g_animCount == 0) continue;

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
    return 0;
}

/* ── DLL entry ─────────────────────────────────────────────────────── */

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
    (void)hInst; (void)reserved;
    if (reason == DLL_PROCESS_ATTACH) {
        getGameDir();
        init_bass_proxy();
        CreateThread(NULL, 0, animThread, NULL, 0, NULL);
    }
    return TRUE;
}
