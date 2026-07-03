#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct AnimTexture {
    DWORD d3dTexObjAddr;          // address of the D3DTexture object in game memory
    DWORD originalD3DTex;         // original IDirect3DTexture8* (frame 0)
    DWORD frameTextures[32];      // IDirect3DTexture8* for each frame
    int frameCount;
    float framerate;              // seconds between frame swaps
    int looptype;                 // 0=once, 1=loop, 2=ping-pong
    int currentFrame;
    int direction;                // 1=forward, -1=backward (ping-pong)
    double lastSwapTime;
    char baseName[256];           // e.g. "arrowanim"
};

struct AnimConfig {
    char baseName[256];
    float framerate;
    int looptype;
    int frameCount;
    char frameNames[32][256];     // full filenames e.g. "arrowanim_01.png"
};

static AnimConfig g_configs[16];
static int g_configCount = 0;
static AnimTexture g_animTextures[16];
static int g_animCount = 0;
static bool g_running = true;
static HANDLE g_thread = NULL;

static double GetTime() {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
}

static void ParseConfigFile(const char* txtPath, const char* baseName) {
    FILE* f = NULL;
    if (fopen_s(&f, txtPath, "r") != 0 || !f) return;

    char line[512];
    float framerate = 0.5f;
    int looptype = 1;

    while (fgets(line, sizeof(line), f)) {
        char key[128], val[128];
        if (sscanf(line, "%127[^=]=%127s", key, val) == 2) {
            char* k = key;
            while (*k == ' ' || *k == '\t') k++;
            char* end = k + strlen(k) - 1;
            while (end > k && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) *end-- = 0;
            if (_stricmp(k, "framerate") == 0) framerate = (float)atof(val);
            else if (_stricmp(k, "looptype") == 0) looptype = atoi(val);
        }
    }
    fclose(f);

    // Scan for frame files: baseName_XX.png
    char searchDir[512];
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    snprintf(searchDir, sizeof(searchDir), "%s\\Textures\\%s_*.png", dir, baseName);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchDir, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    int frameCount = 0;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (frameCount < 32) {
                snprintf(g_configs[g_configCount].frameNames[frameCount], 256, "%s", fd.cFileName);
                frameCount++;
            }
        }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);

    if (frameCount == 0) return;

    // Sort frames by filename so _01 comes before _02 etc
    for (int i = 0; i < frameCount - 1; i++) {
        for (int j = i + 1; j < frameCount; j++) {
            if (_stricmp(g_configs[g_configCount].frameNames[i], g_configs[g_configCount].frameNames[j]) > 0) {
                char tmp[256];
                strcpy_s(tmp, 256, g_configs[g_configCount].frameNames[i]);
                strcpy_s(g_configs[g_configCount].frameNames[i], 256, g_configs[g_configCount].frameNames[j]);
                strcpy_s(g_configs[g_configCount].frameNames[j], 256, tmp);
            }
        }
    }

    strcpy_s(g_configs[g_configCount].baseName, 256, baseName);
    g_configs[g_configCount].framerate = framerate;
    g_configs[g_configCount].looptype = looptype;
    g_configs[g_configCount].frameCount = frameCount;
    g_configCount++;

    char dbg[256];
    snprintf(dbg, sizeof(dbg), "[AnimTex] Config: %s, frames=%d, rate=%.2f, loop=%d\n", baseName, frameCount, framerate, looptype);
    OutputDebugStringA(dbg);
}

static void LoadConfigs() {
    g_configCount = 0;
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    char searchPath[512];
    snprintf(searchPath, sizeof(searchPath), "%s\\Textures\\*.txt", dir);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        const char* fn = fd.cFileName;
        size_t len = strlen(fn);
        if (len < 5 || _stricmp(fn + len - 4, ".txt") != 0) continue;

        // Check if it contains "framerate" 
        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath), "%s\\Textures\\%s", dir, fn);

        FILE* f = NULL;
        if (fopen_s(&f, fullPath, "r") != 0 || !f) continue;
        bool hasFramerate = false;
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, "framerate")) { hasFramerate = true; break; }
        }
        fclose(f);
        if (!hasFramerate) continue;

        // Extract base name (filename without .txt)
        char baseName[256];
        strncpy_s(baseName, 256, fn, len - 4);
        baseName[len - 4] = 0;

        ParseConfigFile(fullPath, baseName);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
}

// Find the D3DTexture object for a given texture name in the Graphics cache
static DWORD FindTextureInCache(DWORD graphicsAddr, const char* name) {
    if (!graphicsAddr || IsBadReadPtr((void*)graphicsAddr, 4)) return 0;

    // Graphics+0x2E8 = texture cache count, Graphics+0x6F0 = array ptr
    if (IsBadReadPtr((void*)(graphicsAddr + 0x2E8), 4)) return 0;
    int count = *(int*)(graphicsAddr + 0x2E8);
    if (count < 1) return 0;

    if (IsBadReadPtr((void*)(graphicsAddr + 0x6F0), 4)) return 0;
    DWORD arrPtr = *(DWORD*)(graphicsAddr + 0x6F0);
    if (!arrPtr || IsBadReadPtr((void*)arrPtr, count * 4)) return 0;

    for (int i = 0; i < count; i++) {
        DWORD entry = *(DWORD*)(arrPtr + i * 4);
        if (!entry || IsBadReadPtr((void*)entry, 0x20)) continue;

        // D3DTexture+0x08 = name (char*)
        DWORD namePtr = *(DWORD*)(entry + 0x08);
        if (!namePtr || IsBadReadPtr((void*)namePtr, 1)) continue;

        const char* texName = (const char*)namePtr;
        if (_stricmp(texName, name) == 0) return entry;
    }
    return 0;
}

// Load a texture using the game's texture loader
// Returns the D3DTexture object address, or 0 on failure
static DWORD LoadTextureViaGame(DWORD graphicsAddr, const char* filename) {
    // Call FUN_00455C50: __thiscall(Graphics*, char* filename, char useCache)
    // This allocates a D3DTexture (0x74 bytes), loads the file, caches it, returns ptr
    DWORD base = (DWORD)(DWORD_PTR)GetModuleHandleA(NULL);
    typedef DWORD(__thiscall *LoadTexFn)(DWORD, const char*, char);
    LoadTexFn fn = (LoadTexFn)(base + 0x55C50);
    return fn(graphicsAddr, filename, 1);
}

static void SetupAnimations(DWORD appAddr) {
    g_animCount = 0;
    if (!appAddr || IsBadReadPtr((void*)appAddr, 4)) return;

    // Graphics at App+0x174
    if (IsBadReadPtr((void*)(appAddr + 0x174), 4)) return;
    DWORD graphics = *(DWORD*)(appAddr + 0x174);
    if (!graphics || IsBadReadPtr((void*)graphics, 4)) return;

    for (int c = 0; c < g_configCount && g_animCount < 16; c++) {
        AnimConfig* cfg = &g_configs[c];

        // Find the first frame texture in the cache (already loaded by the game)
        DWORD firstTexObj = FindTextureInCache(graphics, cfg->frameNames[0]);
        if (!firstTexObj) {
            // Try without extension
            char nameNoExt[256];
            strncpy_s(nameNoExt, 256, cfg->frameNames[0], 255);
            char* dot = strrchr(nameNoExt, '.');
            if (dot) *dot = 0;
            firstTexObj = FindTextureInCache(graphics, nameNoExt);
        }
        if (!firstTexObj) continue;

        // D3DTexture+0x04 = IDirect3DTexture8*
        if (IsBadReadPtr((void*)(firstTexObj + 0x04), 4)) continue;
        DWORD originalTex = *(DWORD*)(firstTexObj + 0x04);
        if (!originalTex) continue;

        AnimTexture* anim = &g_animTextures[g_animCount];
        anim->d3dTexObjAddr = firstTexObj;
        anim->originalD3DTex = originalTex;
        anim->frameTextures[0] = originalTex;
        anim->frameCount = 1;
        anim->framerate = cfg->framerate;
        anim->looptype = cfg->looptype;
        anim->currentFrame = 0;
        anim->direction = 1;
        anim->lastSwapTime = GetTime();
        strcpy_s(anim->baseName, 256, cfg->baseName);

        // Load remaining frames
        for (int i = 1; i < cfg->frameCount && i < 32; i++) {
            DWORD texObj = LoadTextureViaGame(graphics, cfg->frameNames[i]);
            if (texObj && !IsBadReadPtr((void*)(texObj + 0x04), 4)) {
                DWORD d3dTex = *(DWORD*)(texObj + 0x04);
                if (d3dTex) {
                    anim->frameTextures[anim->frameCount] = d3dTex;
                    anim->frameCount++;
                }
            }
        }

        if (anim->frameCount < 2) continue;

        char dbg[256];
        snprintf(dbg, sizeof(dbg), "[AnimTex] Setup: %s, %d frames loaded\n", anim->baseName, anim->frameCount);
        OutputDebugStringA(dbg);

        g_animCount++;
    }
}

static void RestoreTextures() {
    for (int i = 0; i < g_animCount; i++) {
        AnimTexture* anim = &g_animTextures[i];
        if (anim->d3dTexObjAddr && !IsBadWritePtr((void*)(anim->d3dTexObjAddr + 0x04), 4)) {
            *(DWORD*)(anim->d3dTexObjAddr + 0x04) = anim->originalD3DTex;
        }
    }
    g_animCount = 0;
}

static DWORD WINAPI AnimThread(LPVOID param) {
    IModAPI* api = (IModAPI*)param;
    Sleep(3000);

    while (g_running) {
        Sleep(16);

        if (g_animCount == 0) continue;

        double now = GetTime();
        for (int i = 0; i < g_animCount; i++) {
            AnimTexture* anim = &g_animTextures[i];
            if (anim->frameCount < 2) continue;
            if (anim->d3dTexObjAddr == 0) continue;
            if (IsBadReadPtr((void*)(anim->d3dTexObjAddr + 0x04), 4)) continue;

            double elapsed = now - anim->lastSwapTime;
            if (elapsed < (double)anim->framerate) continue;

            anim->lastSwapTime = now;

            int nextFrame = anim->currentFrame + anim->direction;

            if (anim->looptype == 0) {
                // Once: stop on last frame
                if (nextFrame >= anim->frameCount) nextFrame = anim->frameCount - 1;
                else if (nextFrame < 0) nextFrame = 0;
            } else if (anim->looptype == 1) {
                // Loop: wrap around
                if (nextFrame >= anim->frameCount) nextFrame = 0;
                else if (nextFrame < 0) nextFrame = anim->frameCount - 1;
            } else if (anim->looptype == 2) {
                // Ping-pong: reverse direction at ends
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
                if (!IsBadWritePtr((void*)(anim->d3dTexObjAddr + 0x04), 4)) {
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
        LoadConfigs();
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
