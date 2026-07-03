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
static HANDLE g_thread = NULL;

static float g_defaultFramerate = 0.5f;
static int g_defaultLooptype = 1;

static double GetTime() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

// Extract base name and frame number from a texture filename like "arrowanim01.png"
// Returns the frame number (1+), or 0 if the filename doesn't match the pattern
static int ParseFrameName(const char* filename, char* outBase, int baseMaxLen) {
    // Find last dot (extension)
    const char* dot = strrchr(filename, '.');
    if (!dot || dot == filename) return 0;

    int nameLen = (int)(dot - filename);
    if (nameLen < 3) return 0; // need at least 1 char base + 2 digits

    // Find where digits start at the end of the name (before extension)
    int digitStart = nameLen - 1;
    while (digitStart >= 0 && isdigit((unsigned char)filename[digitStart])) {
        digitStart--;
    }
    digitStart++; // first digit

    int digitCount = nameLen - digitStart;
    if (digitCount < 1 || digitCount > 4) return 0; // 1-4 digit frame number

    int frameNum = atoi(filename + digitStart);
    if (frameNum < 1) return 0;

    int baseLen = digitStart;
    if (baseLen == 0 || baseLen >= baseMaxLen) return 0;

    strncpy_s(outBase, baseMaxLen, filename, baseLen);
    outBase[baseLen] = 0;
    return frameNum;
}

// Load framerate/looptype from <baseName>.txt in Textures/ folder, if it exists
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

// Count how many frame files exist for a base name (e.g. arrowanim01.png, arrowanim02.png, ...)
static int CountFrames(const char* baseName) {
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    char searchPath[768];
    snprintf(searchPath, sizeof(searchPath), "%s\\Textures\\%s*.png", dir, baseName);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    int count = 0;
    char frameFiles[32][256];
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (count < 32) {
            strncpy_s(frameFiles[count], 256, fd.cFileName, 255);
            count++;
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    // Verify each file matches the pattern baseNN.png (has digits after base)
    int validCount = 0;
    for (int i = 0; i < count; i++) {
        char dummyBase[256];
        if (ParseFrameName(frameFiles[i], dummyBase, 256) > 0) {
            if (_stricmp(dummyBase, baseName) == 0) validCount++;
        }
    }
    return validCount;
}

static DWORD FindTextureInCache(DWORD graphicsAddr, const char* name) {
    if (!graphicsAddr || IsBadReadPtr((void*)(size_t)graphicsAddr, 4)) return 0;

    if (IsBadReadPtr((void*)(size_t)(graphicsAddr + 0x2E8), 4)) return 0;
    int count = *(int*)(graphicsAddr + 0x2E8);
    if (count < 1) return 0;

    if (IsBadReadPtr((void*)(size_t)(graphicsAddr + 0x6F0), 4)) return 0;
    DWORD arrPtr = *(DWORD*)(graphicsAddr + 0x6F0);
    if (!arrPtr || IsBadReadPtr((void*)(size_t)arrPtr, count * 4)) return 0;

    for (int i = 0; i < count; i++) {
        DWORD entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)(size_t)entry, 0x20)) continue;

        DWORD namePtr = *(DWORD*)(entry + 0x08);
        if (!namePtr || IsBadReadPtr((void*)(size_t)namePtr, 1)) continue;

        const char* texName = (const char*)(size_t)namePtr;
        if (_stricmp(texName, name) == 0) return entry;
    }
    return 0;
}

static DWORD LoadTextureViaGame(DWORD graphicsAddr, const char* filename) {
    DWORD base = (DWORD)(DWORD_PTR)GetModuleHandleA(NULL);
    typedef DWORD(__thiscall *LoadTexFn)(DWORD, const char*, char);
    LoadTexFn fn = (LoadTexFn)(base + 0x55C50);
    return fn(graphicsAddr, filename, 1);
}

static void SetupAnimations(DWORD appAddr) {
    g_animCount = 0;
    if (!appAddr || IsBadReadPtr((void*)(size_t)appAddr, 4)) return;

    if (IsBadReadPtr((void*)(size_t)(appAddr + 0x174), 4)) return;
    DWORD graphics = *(DWORD*)(appAddr + 0x174);
    if (!graphics || IsBadReadPtr((void*)(size_t)graphics, 4)) return;

    if (IsBadReadPtr((void*)(size_t)(graphics + 0x2E8), 4)) return;
    int cacheCount = *(int*)(graphics + 0x2E8);
    if (cacheCount < 1) return;

    if (IsBadReadPtr((void*)(size_t)(graphics + 0x6F0), 4)) return;
    DWORD arrPtr = *(DWORD*)(graphics + 0x6F0);
    if (!arrPtr || IsBadReadPtr((void*)(size_t)arrPtr, cacheCount * 4)) return;

    for (int i = 0; i < cacheCount && g_animCount < 16; i++) {
        DWORD entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)(size_t)entry, 0x20)) continue;

        DWORD namePtr = *(DWORD*)(entry + 0x08);
        if (!namePtr || IsBadReadPtr((void*)(size_t)namePtr, 1)) continue;
        const char* texName = (const char*)(size_t)namePtr;

        char baseName[256];
        int frameNum = ParseFrameName(texName, baseName, 256);
        if (frameNum != 1) continue; // Only start from frame 1

        // Check if additional frames exist
        int totalFrames = CountFrames(baseName);
        if (totalFrames < 2) continue;

        // Get the D3D texture pointer from this entry
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

        // Load frames 2, 3, ... via the game's texture loader
        for (int f = 2; f <= totalFrames && f <= 32; f++) {
            char frameName[256];
            snprintf(frameName, sizeof(frameName), "%s%02d.png", baseName, f);
            DWORD texObj = LoadTextureViaGame(graphics, frameName);
            if (texObj && !IsBadReadPtr((void*)(size_t)(texObj + 0x04), 4)) {
                DWORD d3dTex = *(DWORD*)(texObj + 0x04);
                if (d3dTex) {
                    anim->frameTextures[anim->frameCount] = d3dTex;
                    anim->frameCount++;
                }
            }
        }

        if (anim->frameCount < 2) continue;

        char dbg[256];
        snprintf(dbg, sizeof(dbg), "[AnimTex] %s: %d frames, rate=%.2f, loop=%d\n",
                 anim->baseName, anim->frameCount, anim->framerate, anim->looptype);
        OutputDebugStringA(dbg);
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
    Sleep(3000);
    while (g_running) {
        Sleep(16);
        if (g_animCount == 0) continue;

        double now = GetTime();
        for (int i = 0; i < g_animCount; i++) {
            AnimTexture* anim = &g_animTextures[i];
            if (anim->frameCount < 2) continue;
            if (anim->d3dTexObjAddr == 0) continue;
            if (IsBadReadPtr((void*)(size_t)(anim->d3dTexObjAddr + 0x04), 4)) continue;

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
            if (newTex) {
                if (!IsBadWritePtr((void*)(size_t)(anim->d3dTexObjAddr + 0x04), 4)) {
                    *(DWORD*)(anim->d3dTexObjAddr + 0x04) = newTex;
                }
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
        if (!api) return;
        App* app = api->GetApp();
        if (!app) return;
        RestoreTextures();
        SetupAnimations((DWORD)(DWORD_PTR)app);
    }

    void onSceneEnd() override {
        RestoreTextures();
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new AnimatedTexturesMod();
}
