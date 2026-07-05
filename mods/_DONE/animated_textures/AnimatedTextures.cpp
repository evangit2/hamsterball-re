#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct AnimTexture {
    DWORD d3dTexObjAddr;
    DWORD originalD3DTex;
    DWORD frameTextures[32];
    int frameCount;
    float framerate;
    int looptype;
    int currentFrame;
    int direction;
    double lastSwapTime;
    char baseName[256];
};

static AnimTexture g_animTextures[16];
static int g_animCount = 0;
static bool g_running = true;
static bool g_levelActive = false;
static HANDLE g_thread = NULL;
static IModAPI* g_api = NULL;

static float g_defaultFramerate = 0.5f;
static int g_defaultLooptype = 1;

static void Log(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

static double GetTime() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

static int ParseFrameName(const char* filename, char* outBase, int baseMaxLen) {
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
    strncpy_s(outBase, baseMaxLen, filename, baseLen);
    outBase[baseLen] = 0;
    return frameNum;
}

static void LoadConfigForBase(const char* baseName, float* outFramerate, int* outLooptype) {
    *outFramerate = g_defaultFramerate;
    *outLooptype = g_defaultLooptype;
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    char path[768];
    snprintf(path, sizeof(path), "%s\\Textures\\%s.txt", dir, baseName);
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") != 0 || !f) return;
    char line[512];
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

static int CountFrames(const char* baseName) {
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    char searchPath[768];
    snprintf(searchPath, sizeof(searchPath), "%s\\Textures\\%s*.png", dir, baseName);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;
    int validCount = 0;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        char dummyBase[256];
        if (ParseFrameName(fd.cFileName, dummyBase, 256) > 0) {
            if (_stricmp(dummyBase, baseName) == 0) validCount++;
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
    return validCount;
}

static DWORD LoadTextureViaGame(DWORD graphicsAddr, const char* filename) {
    DWORD base = (DWORD)(DWORD_PTR)GetModuleHandleA(NULL);
    typedef DWORD(__thiscall *LoadTexFn)(DWORD, const char*, char);
    LoadTexFn fn = (LoadTexFn)(base + 0x55C50);
    return fn(graphicsAddr, filename, 1);
}

static void TrySetupAnimations() {
    if (!g_api) return;
    App* app = g_api->GetApp();
    if (!app) return;
    DWORD appAddr = (DWORD)(DWORD_PTR)app;
    if (IsBadReadPtr((void*)(size_t)appAddr, 4)) return;

    if (IsBadReadPtr((void*)(size_t)(appAddr + 0x174), 4)) return;
    DWORD graphics = *(DWORD*)(appAddr + 0x174);
    if (!graphics || IsBadReadPtr((void*)(size_t)graphics, 4)) return;

    if (IsBadReadPtr((void*)(size_t)(graphics + 0x2E8), 4)) return;
    int cacheCount = *(int*)(graphics + 0x2E8);
    if (cacheCount < 1) return;

    if (IsBadReadPtr((void*)(size_t)(graphics + 0x6F0), 4)) return;
    DWORD arrPtr = *(DWORD*)(graphics + 0x6F0);
    if (!arrPtr || IsBadReadPtr((void*)(size_t)arrPtr, cacheCount * 4)) return;

    Log("[AnimTex] Scanning %d cache entries...\n", cacheCount);

    for (int i = 0; i < cacheCount && g_animCount < 16; i++) {
        DWORD entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)(size_t)entry, 0x20)) continue;

        DWORD namePtr = *(DWORD*)(entry + 0x08);
        if (!namePtr || IsBadReadPtr((void*)(size_t)namePtr, 1)) continue;
        const char* texName = (const char*)(size_t)namePtr;

        char baseName[256];
        int frameNum = ParseFrameName(texName, baseName, 256);
        if (frameNum != 1) continue;

        // Check if we already set up this animation
        bool alreadySetup = false;
        for (int j = 0; j < g_animCount; j++) {
            if (_stricmp(g_animTextures[j].baseName, baseName) == 0) {
                alreadySetup = true;
                break;
            }
        }
        if (alreadySetup) continue;

        int totalFrames = CountFrames(baseName);
        Log("[AnimTex] Found %s in cache (frame 1). %d frame files exist.\n", texName, totalFrames);
        if (totalFrames < 2) continue;

        if (IsBadReadPtr((void*)(size_t)(entry + 0x04), 4)) continue;
        DWORD originalTex = *(DWORD*)(entry + 0x04);
        if (!originalTex) continue;

        AnimTexture* anim = &g_animTextures[g_animCount];
        anim->d3dTexObjAddr = entry;
        anim->originalD3DTex = originalTex;
        anim->frameTextures[0] = originalTex;
        anim->frameCount = 1;
        anim->currentFrame = 0;
        anim->direction = 1;
        anim->lastSwapTime = GetTime();
        strcpy_s(anim->baseName, 256, baseName);
        LoadConfigForBase(baseName, &anim->framerate, &anim->looptype);

        for (int f = 2; f <= totalFrames && f <= 32; f++) {
            char frameName[256];
            snprintf(frameName, sizeof(frameName), "%s%02d.png", baseName, f);
            DWORD texObj = LoadTextureViaGame(graphics, frameName);
            if (texObj && !IsBadReadPtr((void*)(size_t)(texObj + 0x04), 4)) {
                DWORD d3dTex = *(DWORD*)(texObj + 0x04);
                if (d3dTex) {
                    anim->frameTextures[anim->frameCount] = d3dTex;
                    anim->frameCount++;
                    Log("[AnimTex] Loaded frame %d: %s -> tex=0x%08X\n", f, frameName, d3dTex);
                }
            } else {
                Log("[AnimTex] Failed to load frame %d: %s\n", f, frameName);
            }
        }

        if (anim->frameCount < 2) {
            Log("[AnimTex] Not enough frames for %s (only %d)\n", baseName, anim->frameCount);
            continue;
        }

        Log("[AnimTex] Setup complete: %s, %d frames, rate=%.2f, loop=%d\n",
            anim->baseName, anim->frameCount, anim->framerate, anim->looptype);
        g_animCount++;
    }
}

static void RestoreTextures() {
    for (int i = 0; i < g_animCount; i++) {
        AnimTexture* anim = &g_animTextures[i];
        if (anim->d3dTexObjAddr && !IsBadWritePtr((void*)(size_t)(anim->d3dTexObjAddr + 0x04), 4)) {
            *(DWORD*)(anim->d3dTexObjAddr + 0x04) = anim->originalD3DTex;
        }
    }
    g_animCount = 0;
}

static DWORD WINAPI AnimThread(LPVOID param) {
    g_api = (IModAPI*)param;
    Sleep(2000);
    Log("[AnimTex] Thread started\n");

    int scanTimer = 0;
    bool wasInLevel = false;

    while (g_running) {
        Sleep(16);

        // Check if we're in a level
        Scene* scene = g_api ? g_api->GetScene() : nullptr;
        bool inLevel = (scene != nullptr);

        if (!inLevel && wasInLevel) {
            Log("[AnimTex] Level ended, restoring textures\n");
            RestoreTextures();
            wasInLevel = false;
            scanTimer = 0;
        }

        if (inLevel && !wasInLevel) {
            Log("[AnimTex] Level started, waiting for textures to load...\n");
            wasInLevel = true;
            scanTimer = 0;
        }

        // Periodically try to set up animations (textures load after level start)
        if (inLevel && g_animCount == 0) {
            scanTimer++;
            // Try every ~1 second (60 frames * 16ms ≈ 1s), first try after 60 frames
            if (scanTimer >= 60) {
                scanTimer = 0;
                Log("[AnimTex] Attempting to set up animations...\n");
                TrySetupAnimations();
            }
        }

        // Animation swapping
        if (g_animCount == 0) continue;

        double now = GetTime();
        for (int i = 0; i < g_animCount; i++) {
            AnimTexture* anim = &g_animTextures[i];
            if (anim->frameCount < 2) continue;
            if (anim->d3dTexObjAddr == 0) continue;
            if (IsBadReadPtr((void*)(size_t)(anim->d3dTexObjAddr + 0x04), 4)) {
                Log("[AnimTex] Texture object became invalid, clearing\n");
                RestoreTextures();
                break;
            }

            double elapsed = now - anim->lastSwapTime;
            if (elapsed < (double)anim->framerate) continue;

            anim->lastSwapTime = now;
            int nextFrame = anim->currentFrame + anim->direction;

            if (anim->looptype == 0) {
                if (nextFrame >= anim->frameCount) nextFrame = anim->frameCount - 1;
                else if (nextFrame < 0) nextFrame = 0;
            } else if (anim->looptype == 1) {
                if (nextFrame >= anim->frameCount) nextFrame = 0;
                else if (nextFrame < 0) nextFrame = anim->frameCount - 1;
            } else if (anim->looptype == 2) {
                if (nextFrame >= anim->frameCount) {
                    anim->direction = -1;
                    nextFrame = anim->frameCount - 2;
                    if (nextFrame < 0) nextFrame = 0;
                } else if (nextFrame < 0) {
                    anim->direction = 1;
                    nextFrame = 1;
                    if (nextFrame >= anim->frameCount) nextFrame = anim->frameCount - 1;
                }
            }

            anim->currentFrame = nextFrame;
            DWORD newTex = anim->frameTextures[nextFrame];
            if (newTex && !IsBadWritePtr((void*)(size_t)(anim->d3dTexObjAddr + 0x04), 4)) {
                *(DWORD*)(anim->d3dTexObjAddr + 0x04) = newTex;
            }
        }
    }
    return 0;
}

class AnimatedTexturesMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
public:
    const char* GetModName() override    { return "Animated Textures"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        g_running = true;
        g_thread = CreateThread(NULL, 0, AnimThread, api, 0, NULL);
    }

    void onLevelStart() override {
        // Texture setup is handled by the background thread,
        // which periodically scans the cache after textures are loaded.
    }

    void onSceneEnd() override {
        RestoreTextures();
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new AnimatedTexturesMod();
}
