// Hamsterball App Struct Definition
// Global singleton at 0x4FD680 (g_App), created in WinMain
// Total size: ~0xA00+
//
// Key sources: App_Initialize_Full (0x429530), App_Initialize (0x46BB40), App_Run (0x46BD80)
// vtable at 0x004CE400 (referenced from WinMain)

#ifndef GAME_APP_H
#define GAME_APP_H

#include <stdint.h>

typedef struct {
    uint32_t vtable;              // +0x000 vtable ptr
    uint32_t hInstance;           // +0x004 WinMain param_1
    uint32_t cmdShow;             // +0x008 WinMain param_3 (nCmdShow)
    uint8_t pad_00c[0x54-0x00c];  // +0x00C padding
    uint32_t registryKey;         // +0x054 Registry/config key (FUN_00472ec0)
    uint8_t pad_058[0x05c-0x058]; // +0x058
    uint32_t targetFPS;           // +0x05C target FPS (used for frame timing)
    uint32_t frameTimeMs;         // +0x5A (1000/FPS) from App_Run
    uint32_t fpsDenominator;      // +0x5B frame time denominator
    uint32_t renderTarget;         // +0x5D render target surface ptr
    uint32_t frameCounter;        // +0x65 per-second frame counter
    uint8_t windowed;             // +0x158 windowed mode flag
    uint32_t width;               // +0x15C window width
    uint32_t height;              // +0x160 window height
    uint32_t graphics;            // +0x174 Graphics object pointer
    uint32_t audioSystem;         // +0x17C Audio/BASS system ptr
    uint32_t d3dDevice;           // +0x180 D3D device ptr (used for texture loading)
    uint32_t versionString;       // +0x1B4 version string ptr
    uint32_t loadedCount;         // +0x1CC loaded objects counter (incremented per load)
    uint8_t initialized;          // +0x200 init complete flag (set to 1 after init)
    uint32_t initStep;            // +0x208 current init step debug string
    uint8_t quitFlag;             // +0x159 set when WM_QUIT received
    uint8_t updateDisabled;       // +0x156 disable update flag
    uint32_t cursor;              // +0x240 loaded cursor handle (from LoadCursorA)
    uint32_t shadowTexture;       // +0x278 shadow.png texture ptr
    uint8_t pad_27c[0x534-0x27c]; // +0x27C padding
    uint32_t musicHandle;         // +0x534 BASS music handle ("music\\music.mo3")
    uint32_t musicChannel1;       // +0x538 BASS channel 1 handle
    uint32_t musicChannel2;       // +0x53C BASS channel 2 handle
    uint32_t gameMode1;           // +0x550 game mode 1 (1 player)
    uint32_t gameMode2;           // +0x554 game mode 2 (2 player)
    uint32_t gameMode3;           // +0x558 game mode 4 (4 player)
    uint32_t gameMode4;           // +0x55C game mode 5 (5 = tournament)
    uint8_t pad_560[0x914-0x560]; // +0x560 padding
    uint32_t playCount;           // +0x914 play count from registry
} App;

#endif // GAME_APP_H