/*
 * Hamsterball Reimplementation — WinMain Entry Point
 * 
 * Mirrors original flow:
 *   WinMain (0x4278E0) → App_Initialize (0x46BB40) → App_Run (0x46BD80)
 * 
 * Uses same APIs as original: D3D8, DirectInput8, DirectSound8, BASS
 * Cross-compiled with i686-w64-mingw32-gcc
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>  /* Needed for WAVEFORMATEX before dsound.h */
#include <d3d8.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Reusable parsers from the project */
#include "level/meshworld_parser.h"
#include "level/mesh_parser.h"
#include "graphics/texture.h"

/* ===== Constants (matching original) ===== */
#define WINDOW_CLASS    "AthenaWindow"       /* Original class name */
#define WINDOW_TITLE    "Hamsterball"
#define DEFAULT_WIDTH   1024
#define DEFAULT_HEIGHT  768
#define TARGET_FPS      30
#define FRAME_TIME_MS   (1000 / TARGET_FPS)

/* Ball physics constants from RE */
#define BALL_RADIUS     26.0f               /* Ball+0x284 */
#define BALL_MAX_SPEED  5000.0f             /* Ball+0x188 (5.0 * 1000) */
#define BALL_DAMPING    0.95f               /* _DAT_004CF3F0 */
#define BALL_GRAVITY    0.5f                /* Ball+0xC94 default */
#define BALL_SPEED_SCALE 1.05f              /* Ball+0xC */

/* ===== Global State (mirrors App struct) ===== */
static HINSTANCE g_hinst = NULL;
static HWND g_hwnd = NULL;
static BOOL g_running = TRUE;
static BOOL g_minimized = FALSE;
static BOOL g_active = TRUE;

/* D3D8 (mirrors GfxEngine struct at App+0x174) */
static IDirect3D8 *g_d3d = NULL;
static IDirect3DDevice8 *g_device = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp;
static int g_width = DEFAULT_WIDTH;
static int g_height = DEFAULT_HEIGHT;

/* DInput8 (mirrors InputDevice at App+0x1E4/0x1E8) */
static IDirectInput8 *g_dinput = NULL;
static IDirectInputDevice8 *g_keyboard = NULL;
static IDirectInputDevice8 *g_mouse = NULL;
static BYTE g_keys[256];            /* DIK key state buffer */
static DIMOUSESTATE g_mouse_state;  /* Mouse relative state */

/* DirectSound8 */
static IDirectSound8 *g_dsound = NULL;
static IDirectSoundBuffer *g_primary_buffer = NULL;

/* Game state (mirrors App+0x708) */
typedef enum {
    STATE_LOADING = 0,
    STATE_MENU = 1,
    STATE_RACING = 3,
    STATE_PAUSED = 4
} GameState;
static GameState g_state = STATE_LOADING;

/* Level */
static mw_level_t *g_level = NULL;
static char g_game_dir[MAX_PATH] = "";
static char g_level_name[256] = "";

/* Ball (mirrors Ball struct key fields) */
static struct {
    float x, y, z;           /* Position (+0x164) */
    float vx, vy, vz;       /* Velocity (+0x170) */
    float radius;            /* +0x284 = 26.0 */
    float max_speed;         /* +0x188 = 5000.0 */
    float speed_scale;       /* +0x18C */
    float damping;           /* Ball_Damping = 0.95 */
    float gravity;           /* +0x1A8 = 0.5 */
    float input_fx, input_fy; /* Accumulated input force */
} g_ball;

/* Camera (mirrors Scene camera system 0x419FA0) */
static struct {
    float x, y, z;       /* Camera position (computed from orbit) */
    float tx, ty, tz;    /* Look-at target = ball position */
    float orbit_angle;   /* Scene+0x29BC: orbital angle (radians) */
    float orbit_dist;    /* Scene+0x29C0: distance from ball (800.0 arena) */
    float orbit_tilt;    /* Y component of orbit direction (0.9 = slightly above) */
} g_camera;

/* Timing (mirrors App_Run) */
static DWORD g_last_tick = 0;
static DWORD g_frame_counter = 0;
static DWORD g_fps_timer = 0;
static int g_current_fps = 0;

/* ===== Forward Declarations ===== */
static BOOL InitWindow(HINSTANCE hInst);
static BOOL InitD3D8(void);
static BOOL InitDInput8(void);
static BOOL InitDSound8(void);
static BOOL LoadAssets(void);
static void Cleanup(void);
static void GameLoop(void);
static void HandleInput(void);
static void UpdatePhysics(float dt);
static void Render(void);
static void RenderBall(void);
static void RenderLevelGeometry(void);
static void RenderLevelObjects(void);
static void RenderHUD(void);
static BOOL SaveScreenshot(const char *filename);

/* ===== Screenshot Function (Windows GDI approach for D3D8 compatibility) ===== */
static BOOL SaveScreenshot(const char *filename) {
    if (!g_device) return FALSE;
    
    /* On Wine SW vertex processing, backbuffer locks and CopyRects both fail.
     * Use a renderable offscreen surface: render to it, then lock it directly. 
     * Since we're SW vertex processing, we can create a D3DPOOL_SCRATCH surface 
     * which is always lockable. */
    
    /* Create a lockable offscreen surface in system memory */
    IDirect3DSurface8 *pSurf = NULL;
    HRESULT hr = g_device->lpVtbl->CreateImageSurface(g_device, g_width, g_height,
                                                         D3DFMT_A8R8G8B8, &pSurf);
    if (FAILED(hr)) {
        printf("[Screenshot] CreateImageSurface failed: 0x%08lx\n", hr);
        return FALSE;
    }
    
    /* Get backbuffer */
    IDirect3DSurface8 *pBack = NULL;
    hr = g_device->lpVtbl->GetBackBuffer(g_device, 0, D3DBACKBUFFER_TYPE_MONO, &pBack);
    if (FAILED(hr)) {
        printf("[Screenshot] GetBackBuffer failed: 0x%08lx\n", hr);
        pSurf->lpVtbl->Release(pSurf);
        return FALSE;
    }
    
    /* Try CopyRects from backbuffer to system memory surface */
    hr = g_device->lpVtbl->CopyRects(g_device, pBack, NULL, 0, pSurf, NULL);
    pBack->lpVtbl->Release(pBack);
    
    if (FAILED(hr)) {
        /* CopyRects failed — try direct lock of backbuffer as last resort */
        D3DLOCKED_RECT locked;
        IDirect3DSurface8 *pBack2 = NULL;
        HRESULT hr2 = g_device->lpVtbl->GetBackBuffer(g_device, 0, D3DBACKBUFFER_TYPE_MONO, &pBack2);
        if (SUCCEEDED(hr2)) {
            hr2 = pBack2->lpVtbl->LockRect(pBack2, &locked, NULL, D3DLOCK_READONLY);
            if (SUCCEEDED(hr2)) {
                /* Direct lock worked — save from it directly */
                int row_size = ((g_width * 3 + 3) & ~3);
                BITMAPFILEHEADER bf = {0};
                BITMAPINFOHEADER bi = {0};
                bi.biSize = sizeof(BITMAPINFOHEADER);
                bi.biWidth = g_width; bi.biHeight = g_height;
                bi.biPlanes = 1; bi.biBitCount = 24; bi.biCompression = BI_RGB;
                DWORD dwBmpSize = row_size * g_height;
                bf.bfType = 0x4D42;
                bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
                bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                
                FILE *fp = fopen(filename, "wb");
                if (fp) {
                    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, fp);
                    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, fp);
                    BYTE *row_buf = malloc(row_size);
                    for (int y = g_height - 1; y >= 0; y--) {
                        DWORD *src_row = (DWORD *)((BYTE *)locked.pBits + y * locked.Pitch);
                        for (int x = 0; x < g_width; x++) {
                            DWORD px = src_row[x];
                            row_buf[x*3+0]=(px>>16)&0xFF; row_buf[x*3+1]=(px>>8)&0xFF; row_buf[x*3+2]=px&0xFF;
                        }
                        fwrite(row_buf, 1, row_size, fp);
                    }
                    free(row_buf);
                    fclose(fp);
                    printf("[Screenshot] Saved: %s (%dx%d) via direct backbuffer lock\n", filename, g_width, g_height);
                }
                pBack2->lpVtbl->UnlockRect(pBack2);
                pBack2->lpVtbl->Release(pBack2);
                pSurf->lpVtbl->Release(pSurf);
                return TRUE;
            }
            pBack2->lpVtbl->Release(pBack2);
        }
        
        printf("[Screenshot] All methods failed (CopyRects=0x%08lx)\n", hr);
        pSurf->lpVtbl->Release(pSurf);
        return FALSE;
    }
    
    /* CopyRects succeeded — lock system surface and write BMP */
    D3DLOCKED_RECT locked;
    hr = pSurf->lpVtbl->LockRect(pSurf, &locked, NULL, D3DLOCK_READONLY);
    if (FAILED(hr)) {
        printf("[Screenshot] LockRect failed: 0x%08lx\n", hr);
        pSurf->lpVtbl->Release(pSurf);
        return FALSE;
    }
    
    int row_size = ((g_width * 3 + 3) & ~3);
    BITMAPFILEHEADER bf = {0};
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = g_width; bi.biHeight = g_height;
    bi.biPlanes = 1; bi.biBitCount = 24; bi.biCompression = BI_RGB;
    DWORD dwBmpSize = row_size * g_height;
    bf.bfType = 0x4D42;
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    FILE *fp = fopen(filename, "wb");
    if (fp) {
        fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, fp);
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, fp);
        BYTE *row_buf = malloc(row_size);
        for (int y = g_height - 1; y >= 0; y--) {
            DWORD *src_row = (DWORD *)((BYTE *)locked.pBits + y * locked.Pitch);
            for (int x = 0; x < g_width; x++) {
                DWORD px = src_row[x];
                row_buf[x*3+0]=(px>>16)&0xFF; row_buf[x*3+1]=(px>>8)&0xFF; row_buf[x*3+2]=px&0xFF;
            }
            fwrite(row_buf, 1, row_size, fp);
        }
        free(row_buf);
        fclose(fp);
        printf("[Screenshot] Saved: %s (%dx%d)\n", filename, g_width, g_height);
    }
    
    pSurf->lpVtbl->UnlockRect(pSurf);
    pSurf->lpVtbl->Release(pSurf);
    return fp != NULL;
}

/* ===== Window Procedure (matches original WndProc) ===== */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ACTIVATE:
        g_active = (LOWORD(wp) != WA_INACTIVE);
        if (g_keyboard) g_keyboard->lpVtbl->Acquire(g_keyboard);
        if (g_mouse) g_mouse->lpVtbl->Acquire(g_mouse);
        break;
    case WM_SIZE:
        if (wp == SIZE_MINIMIZED) g_minimized = TRUE;
        else g_minimized = FALSE;
        break;
    case WM_CLOSE:
        g_running = FALSE;
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_F12) {  /* F12 for screenshot */
            char filename[MAX_PATH];
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            snprintf(filename, sizeof(filename),
                "screenshot_%04d%02d%02d_%02d%02d%02d.bmp",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
            SaveScreenshot(filename);
        }
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

/* ===== Init: Window (Step 3 of App_Initialize) ===== */
static BOOL InitWindow(HINSTANCE hInst) {
    g_hinst = hInst;
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = WINDOW_CLASS;
    if (!RegisterClassExA(&wc)) return FALSE;

    /* Original: CreateWindow "AthenaWindow" with 1024x768 */
    g_hwnd = CreateWindowExA(
        0, WINDOW_CLASS, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_width, g_height,
        NULL, NULL, hInst, NULL
    );
    return g_hwnd != NULL;
}

/* ===== Init: D3D8 (Step 11 of App_Initialize, mirrors Graphics_CreateDevice 0x453C90) ===== */
static BOOL InitD3D8(void) {
    g_d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if (!g_d3d) { fprintf(stderr, "Direct3DCreate8 failed\n"); return FALSE; }

    /* Find display mode matching original resolution */
    D3DDISPLAYMODE mode;
    g_d3d->lpVtbl->GetAdapterDisplayMode(g_d3d, D3DADAPTER_DEFAULT, &mode);

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferWidth = g_width;
    g_d3dpp.BackBufferHeight = g_height;
    g_d3dpp.BackBufferCount = 1;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    /* CreateDevice — mirrors Graphics_CreateDevice */
    HRESULT hr = g_d3d->lpVtbl->CreateDevice(
        g_d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_device
    );
    if (FAILED(hr)) {
        /* Fallback to software vertex processing */
        hr = g_d3d->lpVtbl->CreateDevice(
            g_d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dpp, &g_device
        );
    }
    if (FAILED(hr)) { 
        fprintf(stderr, "CreateDevice failed: 0x%08lX\n", hr); 
        return FALSE; 
    }

    /* Graphics_Defaults (0x455A60) — set initial render states */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_AMBIENT, 0x00202020);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_CCW);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_SPECULARENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_NORMALIZENORMALS, TRUE);

    /* Setup light (mirrors Graphics_SetupLights 0x454630) */
    D3DLIGHT8 light;
    ZeroMemory(&light, sizeof(light));
    light.Type = D3DLIGHT_DIRECTIONAL;
    light.Diffuse.r = 1.0f; light.Diffuse.g = 1.0f; light.Diffuse.b = 1.0f;
    light.Ambient.r = 0.3f; light.Ambient.g = 0.3f; light.Ambient.b = 0.3f;
    light.Direction.x = 0.5f; light.Direction.y = -1.0f; light.Direction.z = 0.3f;
    g_device->lpVtbl->SetLight(g_device, 0, &light);
    g_device->lpVtbl->LightEnable(g_device, 0, TRUE);

    printf("[D3D8] Device created, %dx%d\n", g_width, g_height);
    return TRUE;
}

/* ===== Init: DirectInput8 (Steps 15-22 of App_Initialize) ===== */
static BOOL InitDInput8(void) {
    HRESULT hr = DirectInput8Create(g_hinst, DIRECTINPUT_VERSION,
                                     &IID_IDirectInput8, (void**)&g_dinput, NULL);
    if (FAILED(hr)) { fprintf(stderr, "DirectInput8Create failed\n"); return FALSE; }

    /* Keyboard (type 1) */
    g_dinput->lpVtbl->CreateDevice(g_dinput, &GUID_SysKeyboard, &g_keyboard, NULL);
    if (g_keyboard) {
        g_keyboard->lpVtbl->SetDataFormat(g_keyboard, &c_dfDIKeyboard);
        g_keyboard->lpVtbl->SetCooperativeLevel(g_keyboard, g_hwnd,
            DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        g_keyboard->lpVtbl->Acquire(g_keyboard);
    }

    /* Mouse (type 2) */
    g_dinput->lpVtbl->CreateDevice(g_dinput, &GUID_SysMouse, &g_mouse, NULL);
    if (g_mouse) {
        g_mouse->lpVtbl->SetDataFormat(g_mouse, &c_dfDIMouse);
        g_mouse->lpVtbl->SetCooperativeLevel(g_mouse, g_hwnd,
            DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
        g_mouse->lpVtbl->Acquire(g_mouse);
    }

    printf("[DInput8] Keyboard + Mouse initialized\n");
    return TRUE;
}

/* ===== Init: DirectSound8 ===== */
static BOOL InitDSound8(void) {
    HRESULT hr = DirectSoundCreate8(NULL, &g_dsound, NULL);
    if (FAILED(hr)) { 
        fprintf(stderr, "DirectSoundCreate8 failed (non-fatal)\n"); 
        return TRUE; /* Non-fatal: sound not required for first build */
    }
    g_dsound->lpVtbl->SetCooperativeLevel(g_dsound, g_hwnd, DSSCL_NORMAL);
    printf("[DSound8] Initialized\n");
    return TRUE;
}

/* ===== Asset Discovery ===== */
static BOOL FindGameDir(void) {
    /* Try common locations for original game files */
    const char *search[] = {
        ".", "./extracted", "../extracted",
        "./originals/installed/extracted",
        "../originals/installed/extracted",
        "../../originals/installed/extracted",
        NULL
    };
    for (int i = 0; search[i]; i++) {
        char test[MAX_PATH];
        snprintf(test, sizeof(test), "%s/Levels", search[i]);
        DWORD attr = GetFileAttributesA(test);
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
            strncpy(g_game_dir, search[i], sizeof(g_game_dir) - 1);
            printf("[Assets] Game dir: %s\n", g_game_dir);
            return TRUE;
        }
    }
    /* Dev path fallback */
    const char *dev = "/home/evan/hamsterball-re/originals/installed/extracted";
    strncpy(g_game_dir, dev, sizeof(g_game_dir) - 1);
    printf("[Assets] Using dev path: %s\n", g_game_dir);
    return TRUE;
}

/* ===== Load Level (mirrors LoadRaceData 0x40A120) ===== */
static BOOL LoadLevel(const char *name) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/Levels/%s.MESHWORLD", g_game_dir, name);
    if (g_level) meshworld_free(g_level);
    g_level = meshworld_parse_file(path);
    if (!g_level) {
        fprintf(stderr, "[Load] Failed: %s\n", path);
        return FALSE;
    }
    strncpy(g_level_name, name, sizeof(g_level_name) - 1);
    printf("[Load] %s: %d objects, %d vertices, %d materials\n", name, g_level->object_count, g_level->vertex_count, 0 /* TODO: material_count */);
    return TRUE;
}

/* ===== Load Assets ===== */
static BOOL LoadAssets(void) {
    if (!FindGameDir()) return FALSE;

    /* Load level — try WarmUp first (has proper geometry), then SpawnPlatform */
    if (!LoadLevel("Arena-WarmUp")) {
        if (!LoadLevel("Arena-SpawnPlatform")) {
            if (!LoadLevel("Level1")) return FALSE;
        }
    }

    /* Reset ball at start position — on the level surface */
    if (g_level && g_level->vertex_count > 0) {
        /* Use the computed bbox to position ball at center of the level surface */
        g_ball.x = (g_level->bounds_min.x + g_level->bounds_max.x) / 2.0f;
        g_ball.y = g_level->bounds_max.y + g_ball.radius + 1.0f; /* Just above the highest surface */
        g_ball.z = (g_level->bounds_min.z + g_level->bounds_max.z) / 2.0f;
    } else {
        g_ball.x = 0; g_ball.y = 13.33f + 26.0f; g_ball.z = 0;
    }
    g_ball.vx = g_ball.vy = g_ball.vz = 0;
    g_ball.radius = BALL_RADIUS;
    g_ball.max_speed = BALL_MAX_SPEED;
    g_ball.speed_scale = BALL_SPEED_SCALE;
    g_ball.damping = BALL_DAMPING;
    g_ball.gravity = BALL_GRAVITY;

    /* Find ball start in level objects */
    if (g_level) {
        for (int i = 0; i < g_level->object_count; i++) {
            if (g_level->objects[i].type == MW_OBJ_START) {
                g_ball.x = g_level->objects[i].position.x;
                g_ball.y = g_level->objects[i].position.y + g_ball.radius + 1.0f;
                g_ball.z = g_level->objects[i].position.z;
                break;
            }
        }
    }

    /* Camera defaults — orbital follow (mirrors CameraLookAt 0x413280).
     * Camera_SetView(orbit_dir, distance):
     *   orbit_dir = (cos(angle), tilt, sin(angle)) normalized 
     *   eye = ball_pos + normalize(orbit_dir) * distance
     * Arena default: distance=800, angle=45°, tilt=0.9
     * Race default: distance=250, angle=0°, tilt=1.2 (closer, more overhead) */
    g_camera.tx = g_ball.x; g_camera.ty = g_ball.y; g_camera.tz = g_ball.z;
    g_camera.orbit_angle = -1.5708f;  /* -PI/2: camera behind ball in -Z direction */
    g_camera.orbit_dist = 200.0f;     /* Medium follow distance */
    g_camera.orbit_tilt = 0.15f;      /* Low tilt = nearly horizontal view */

    /* Initialize texture system after D3D device is ready */
    {
        char tex_dir[MAX_PATH];
        snprintf(tex_dir, sizeof(tex_dir), "%s/Textures", g_game_dir);
        texture_system_init(g_device, tex_dir);
        
        /* Pre-load all textures referenced by level geoms */
        if (g_level) {
            for (int g = 0; g < g_level->geom_count; g++) {
                if (g_level->geoms[g].has_texture && g_level->geoms[g].texture[0]) {
                    texture_load(g_level->geoms[g].texture);
                }
            }
        }
    }

    printf("[Load] Ball at (%.1f, %.1f, %.1f)\n", g_ball.x, g_ball.y, g_ball.z);
    return TRUE;
}

/* ===== Input: Poll DInput8 (mirrors InputDevice_PollAndRelease 0x46EBD0) ===== */
static void PollInput(void) {
    if (g_keyboard) {
        HRESULT hr = g_keyboard->lpVtbl->GetDeviceState(g_keyboard, sizeof(g_keys), g_keys);
        if (FAILED(hr)) g_keyboard->lpVtbl->Acquire(g_keyboard);
    }
    if (g_mouse) {
        HRESULT hr = g_mouse->lpVtbl->GetDeviceState(g_mouse, sizeof(g_mouse_state), &g_mouse_state);
        if (FAILED(hr)) g_mouse->lpVtbl->Acquire(g_mouse);
    }
}

/* ===== Input: Ball_GetInputForce (0x46EC30) ===== */
static void HandleInput(void) {
    PollInput();
    g_ball.input_fx = 0;
    g_ball.input_fy = 0;

    if (g_state != STATE_RACING) {
        /* Menu input */
        if (g_keys[DIK_ESCAPE] & 0x80) {
            if (g_state == STATE_RACING) g_state = STATE_MENU;
            else g_running = FALSE;
        }
        return;
    }

    /* Keyboard mode (matches Ball_GetInputForce mode 1) */
    if (g_keys[DIK_UP] & 0x80)    g_ball.input_fy -= 0.5f;
    if (g_keys[DIK_DOWN] & 0x80)  g_ball.input_fy += 1.0f;
    if (g_keys[DIK_LEFT] & 0x80)  g_ball.input_fx -= 1.0f;
    if (g_keys[DIK_RIGHT] & 0x80) g_ball.input_fx += 1.0f;

    /* WASD alternative */
    if (g_keys[DIK_W] & 0x80) g_ball.input_fy -= 0.5f;
    if (g_keys[DIK_S] & 0x80) g_ball.input_fy += 1.0f;
    if (g_keys[DIK_A] & 0x80) g_ball.input_fx -= 1.0f;
    if (g_keys[DIK_D] & 0x80) g_ball.input_fx += 1.0f;

    /* Mouse mode (matches Ball_GetInputForce mode 2) */
    if ((g_mouse_state.rgbButtons[0] & 0x80) && g_active) {
        POINT pt; GetCursorPos(&pt);
        LONG cx = g_width / 2, cy = g_height / 2;
        g_ball.input_fx += (float)(pt.x - cx) / (float)cx;
        g_ball.input_fy += (float)(pt.y - cy) / (float)cy;
    }

    /* Escape → pause/menu */
    if (g_keys[DIK_ESCAPE] & 0x80) {
        g_state = STATE_MENU;
    }
}

/* ===== Physics: Ball_AdvancePositionOrCollision (0x4564C0) simplified ===== */
static void UpdatePhysics(float dt) {
    if (g_state != STATE_RACING) return;

    /* Phase 2: Input velocity */
    float force_scale = g_ball.speed_scale * 1000.0f;
    g_ball.vx += g_ball.input_fx * force_scale * dt;
    g_ball.vz += g_ball.input_fy * force_scale * dt;

    /* Phase 3: Damping */
    float damp = (1.0f - dt) + (1.0f - g_ball.damping) * dt;
    g_ball.vx *= damp;
    g_ball.vz *= damp;

    /* Clamp max speed */
    float speed = sqrtf(g_ball.vx * g_ball.vx + g_ball.vz * g_ball.vz);
    if (speed > g_ball.max_speed) {
        float scale = g_ball.max_speed / speed;
        g_ball.vx *= scale;
        g_ball.vz *= scale;
    }

    /* Phase 5: Gravity */
    g_ball.vy -= g_ball.gravity * 100.0f * dt;

    /* Apply velocity */
    g_ball.x += g_ball.vx * dt;
    g_ball.y += g_ball.vy * dt;
    g_ball.z += g_ball.vz * dt;

    /* Simple ground collision (y=0 plane) */
    if (g_ball.y < g_ball.radius) {
        g_ball.y = g_ball.radius;
        g_ball.vy = 0;
    }
}

/* ===== Rendering: Graphics_RenderScene (0x454BC0) ===== */
static void SetMatrices(void) {
    /* Projection (Graphics_SetProjection 0x454AB0) */
    D3DMATRIX proj;
    float aspect = (float)g_width / (float)g_height;
    float fov = 3.14159265f / 3.0f; /* 60 degree FOV */
    float zn = 1.0f, zf = 10000.0f;  /* Extended far plane for large arena levels */
    float yscale = cosf(fov / 2.0f) / sinf(fov / 2.0f);
    float xscale = yscale / aspect;
    ZeroMemory(&proj, sizeof(proj));
    proj._11 = xscale; proj._22 = yscale;
    proj._33 = zf / (zf - zn); proj._34 = 1.0f;  /* Left-handed D3D convention */
    proj._43 = -zn * zf / (zf - zn);
    g_device->lpVtbl->SetTransform(g_device, D3DTS_PROJECTION, &proj);

    /* View — orbital follow camera (mirrors Scene_SetCamera 0x419FA0 Mode 5)
     * orbit_dir = (cos(angle), tilt, sin(angle)) normalized, then:
     * eye = ball_pos + normalize(orbit_dir) * distance
     * at  = ball_pos
     * This gives the classic Hamsterball "floating overhead behind" view. */
    float cos_a = cosf(g_camera.orbit_angle);
    float sin_a = sinf(g_camera.orbit_angle);

    /* Build orbit direction vector (from focus toward camera) */
    float dir_x = cos_a;
    float dir_y = g_camera.orbit_tilt;
    float dir_z = sin_a;
    /* Normalize it */
    float dlen = sqrtf(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);
    if (dlen > 0) { dir_x /= dlen; dir_y /= dlen; dir_z /= dlen; }

    float eyex = g_ball.x + dir_x * g_camera.orbit_dist;
    float eyey = g_ball.y + dir_y * g_camera.orbit_dist;
    float eyez = g_ball.z + dir_z * g_camera.orbit_dist;
    float atx = g_ball.x, aty = g_ball.y, atz = g_ball.z;
    float upx = 0, upy = 1, upz = 0;
    
    D3DMATRIX view;
    /* Compute axes for D3D left-handed look-at matrix (D3DXMatrixLookAtLH convention) */
    float zaxis_x = atx - eyex, zaxis_y = aty - eyey, zaxis_z = atz - eyez;
    float zlen = sqrtf(zaxis_x*zaxis_x + zaxis_y*zaxis_y + zaxis_z*zaxis_z);
    if (zlen > 0) { zaxis_x /= zlen; zaxis_y /= zlen; zaxis_z /= zlen; }

    /* Right = normalize(cross(up, zaxis)) — D3D left-handed convention */
    float xaxis_x = upy*zaxis_z - upz*zaxis_y;
    float xaxis_y = upz*zaxis_x - upx*zaxis_z;
    float xaxis_z = upx*zaxis_y - upy*zaxis_x;
    float xlen = sqrtf(xaxis_x*xaxis_x + xaxis_y*xaxis_y + xaxis_z*xaxis_z);
    if (xlen > 0) { xaxis_x /= xlen; xaxis_y /= xlen; xaxis_z /= xlen; }

    /* Up = cross(zaxis, xaxis) — D3D left-handed convention */
    float yaxis_x = zaxis_y*xaxis_z - zaxis_z*xaxis_y;
    float yaxis_y = zaxis_z*xaxis_x - zaxis_x*xaxis_z;
    float yaxis_z = zaxis_x*xaxis_y - zaxis_y*xaxis_x;

    ZeroMemory(&view, sizeof(view));
    /* D3DXMatrixLookAtLH layout (from MS docs) — axes as COLUMNS, translation in ROW 3 */
    view._11 = xaxis_x;  view._12 = yaxis_x;  view._13 = zaxis_x;  view._14 = 0.0f;
    view._21 = xaxis_y;  view._22 = yaxis_y;  view._23 = zaxis_y;  view._24 = 0.0f;
    view._31 = xaxis_z;  view._32 = yaxis_z;  view._33 = zaxis_z;  view._34 = 0.0f;
    view._41 = -(xaxis_x*eyex + xaxis_y*eyey + xaxis_z*eyez);
    view._42 = -(yaxis_x*eyex + yaxis_y*eyey + yaxis_z*eyez);
    view._43 = -(zaxis_x*eyex + zaxis_y*eyey + zaxis_z*eyez);
    view._44 = 1.0f;

    g_device->lpVtbl->SetTransform(g_device, D3DTS_VIEW, &view);
    
    /* Debug: print first frame camera info */
    static int frame_count = 0;
    if (frame_count < 3) {
        printf("[Camera] Orbit angle=%.2f dist=%.0f tilt=%.1f\n", 
               g_camera.orbit_angle, g_camera.orbit_dist, g_camera.orbit_tilt);
        printf("[Camera] Eye=(%.1f,%.1f,%.1f) At=(%.1f,%.1f,%.1f)\n", 
               eyex, eyey, eyez, atx, aty, atz);
        frame_count++;
    }
}

static void RenderBall(void) {
    /* Generate sphere geometry for the ball */
    const int slices = 16, stacks = 12;
    const float radius = g_ball.radius;
    
    /* Material (matches original ball material) */
    D3DMATERIAL8 mat;
    ZeroMemory(&mat, sizeof(mat));
    mat.Diffuse.r = 0.9f; mat.Diffuse.g = 0.7f; mat.Diffuse.b = 0.3f; mat.Diffuse.a = 1.0f;
    mat.Ambient.r = 0.3f; mat.Ambient.g = 0.25f; mat.Ambient.b = 0.1f;
    mat.Specular.r = 0.7f; mat.Specular.g = 0.7f; mat.Specular.b = 0.7f;
    mat.Power = 64.0f;
    g_device->lpVtbl->SetMaterial(g_device, &mat);

    D3DMATRIX world;
    ZeroMemory(&world, sizeof(world));
    world._11 = 1.0f; world._22 = 1.0f; world._33 = 1.0f; world._44 = 1.0f;
    world._41 = g_ball.x; world._42 = g_ball.y; world._43 = g_ball.z;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &world);

    for (int i = 0; i < stacks; i++) {
        float phi1 = 3.14159265f * i / stacks - 3.14159265f / 2.0f;
        float phi2 = 3.14159265f * (i + 1) / stacks - 3.14159265f / 2.0f;
        
        for (int j = 0; j < slices; j++) {
            float theta1 = 2.0f * 3.14159265f * j / slices;
            float theta2 = 2.0f * 3.14159265f * (j + 1) / slices;

            struct { float x, y, z; float nx, ny, nz; } verts[4];
            
            verts[0].x = radius * cosf(phi1) * cosf(theta1);
            verts[0].y = radius * sinf(phi1);
            verts[0].z = radius * cosf(phi1) * sinf(theta1);
            verts[0].nx = cosf(phi1) * cosf(theta1);
            verts[0].ny = sinf(phi1);
            verts[0].nz = cosf(phi1) * sinf(theta1);

            verts[1].x = radius * cosf(phi1) * cosf(theta2);
            verts[1].y = radius * sinf(phi1);
            verts[1].z = radius * cosf(phi1) * sinf(theta2);
            verts[1].nx = cosf(phi1) * cosf(theta2);
            verts[1].ny = sinf(phi1);
            verts[1].nz = cosf(phi1) * sinf(theta2);

            verts[2].x = radius * cosf(phi2) * cosf(theta1);
            verts[2].y = radius * sinf(phi2);
            verts[2].z = radius * cosf(phi2) * sinf(theta1);
            verts[2].nx = cosf(phi2) * cosf(theta1);
            verts[2].ny = sinf(phi2);
            verts[2].nz = cosf(phi2) * sinf(theta1);

            verts[3].x = radius * cosf(phi2) * cosf(theta2);
            verts[3].y = radius * sinf(phi2);
            verts[3].z = radius * cosf(phi2) * sinf(theta2);
            verts[3].nx = cosf(phi2) * cosf(theta2);
            verts[3].ny = sinf(phi2);
            verts[3].nz = cosf(phi2) * sinf(theta2);

            g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_NORMAL);
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2,
                verts, sizeof(verts[0]));
        }
    }
}

/* ===== Render Level Mesh — Per-geom with D3D lighting + materials + textures =====
 * Each geom has its own material (ambient/diffuse/specular/emissive/power)
 * and optional texture. We render each geom separately with:
 *   - D3D lighting + material for all geoms (realistic shading from normals)
 *   - Texture modulation for textured geoms (PinkChecker, etc.)
 * 
 * Strip format: each strip has (tri_count, vertex_offset) into the global vertex
 * buffer. We expand to triangle list for DrawPrimitiveUP compatibility.
 * 
 * FVF = XYZ|NORMAL|TEX1 (24 bytes) — normals enable proper D3D lighting. */

/* Full vertex for rendering with normals + UV */
typedef struct { float x, y, z; float nx, ny, nz; float u, v; } LevelVertex;

static void RenderLevelGeometry(void) {
    if (!g_level || g_level->vertex_count == 0 || !g_level->geoms) return;
    
    /* Identity world matrix */
    D3DMATRIX world;
    ZeroMemory(&world, sizeof(world));
    world._11 = 1.0f; world._22 = 1.0f; world._33 = 1.0f; world._44 = 1.0f;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &world);
    
    /* Enable Z-buffer, disable culling (track surfaces face both ways) */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_NONE);
    
    /* ENABLE D3D lighting — materials + normals give realistic shading */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
    
    /* Directional light (matches original game's lighting setup) */
    {
        D3DLIGHT8 light;
        ZeroMemory(&light, sizeof(light));
        light.Type = D3DLIGHT_DIRECTIONAL;
        light.Ambient.r = 0.35f; light.Ambient.g = 0.35f; light.Ambient.b = 0.35f;
        light.Diffuse.r = 0.8f; light.Diffuse.g = 0.8f; light.Diffuse.b = 0.8f;
        light.Specular.r = 0.5f; light.Specular.g = 0.5f; light.Specular.b = 0.5f;
        light.Direction.x = -0.3f; light.Direction.y = -1.0f; light.Direction.z = 0.3f;
        g_device->lpVtbl->SetLight(g_device, 0, &light);
        g_device->lpVtbl->LightEnable(g_device, 0, TRUE);
        DWORD ambient = D3DCOLOR_RGBA(90, 90, 90, 255);
        g_device->lpVtbl->SetRenderState(g_device, D3DRS_AMBIENT, ambient);
    }
    
    /* Vertex format with normals: XYZ|NORMAL|TEX1 = 24 bytes per vertex */
    typedef struct { float x, y, z; float nx, ny, nz; float u, v; } LitVertex;
    DWORD fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;
    g_device->lpVtbl->SetVertexShader(g_device, fvf);
    
    /* Allocate expansion buffer */
    static LitVertex *tri_buf = NULL;
    static int tri_buf_count = 0;
    int max_verts = g_level->vertex_count * 3;
    if (!tri_buf || tri_buf_count < max_verts) {
        free(tri_buf);
        tri_buf = malloc(max_verts * sizeof(LitVertex));
        tri_buf_count = max_verts;
    }
    
    /* Render each geom with its own D3D material + optional texture */
    static int first_frame = 0;
    int total_tris = 0;
    
    for (int gi = 0; gi < g_level->geom_count; gi++) {
        mw_geom_t *geom = &g_level->geoms[gi];
        
        /* Set D3D material from geom's material properties */
        D3DMATERIAL8 mat;
        ZeroMemory(&mat, sizeof(mat));
        mat.Ambient.r  = geom->ambient[0];  mat.Ambient.g  = geom->ambient[1];
        mat.Ambient.b  = geom->ambient[2];  mat.Ambient.a  = geom->ambient[3];
        mat.Diffuse.r  = geom->diffuse[0];  mat.Diffuse.g  = geom->diffuse[1];
        mat.Diffuse.b  = geom->diffuse[2];  mat.Diffuse.a  = geom->diffuse[3];
        mat.Specular.r = geom->specular[0]; mat.Specular.g = geom->specular[1];
        mat.Specular.b = geom->specular[2]; mat.Specular.a = geom->specular[3];
        mat.Emissive.r = geom->emissive[0]; mat.Emissive.g = geom->emissive[1];
        mat.Emissive.b = geom->emissive[2]; mat.Emissive.a = geom->emissive[3];
        mat.Power = geom->power;
        g_device->lpVtbl->SetMaterial(g_device, &mat);
        
        /* Bind texture or clear it */
        texture_t *tex = NULL;
        if (geom->has_texture && geom->texture[0]) {
            tex = texture_load(geom->texture);
        }
        texture_bind(tex, g_device);
        
        /* Set texture stage: modulate texture with D3D lighting result */
        if (tex) {
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        } else {
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        }
        
        /* Expand this geom's strips into triangle list */
        int tri_out = 0;
        for (int s = 0; s < geom->strip_count; s++) {
            mw_strip_t *strip = &geom->strips[s];
            int ntri = strip->tri_count;
            int base = strip->vertex_offset;
            
            for (int t = 0; t < ntri; t++) {
                int v0 = base + t;
                int v1 = (t % 2 == 0) ? (base + t + 1) : (base + t + 2);
                int v2 = (t % 2 == 0) ? (base + t + 2) : (base + t + 1);
                
                if (v0 < g_level->vertex_count && v1 < g_level->vertex_count && 
                    v2 < g_level->vertex_count && tri_out + 3 <= tri_buf_count) {
                    mw_vertex_t *sv;
                    sv = &g_level->vertices[v0];
                    tri_buf[tri_out++] = (LitVertex){ sv->x, sv->y, sv->z, sv->nx, sv->ny, sv->nz, sv->u, sv->v };
                    sv = &g_level->vertices[v1];
                    tri_buf[tri_out++] = (LitVertex){ sv->x, sv->y, sv->z, sv->nx, sv->ny, sv->nz, sv->u, sv->v };
                    sv = &g_level->vertices[v2];
                    tri_buf[tri_out++] = (LitVertex){ sv->x, sv->y, sv->z, sv->nx, sv->ny, sv->nz, sv->u, sv->v };
                }
            }
        }
        
        int prim_count = tri_out / 3;
        if (prim_count > 0) {
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLELIST, prim_count,
                tri_buf, sizeof(LitVertex));
            total_tris += prim_count;
        }
    }
    
    if (first_frame < 2) {
        printf("[Render] Per-geom D3D-lit render: %d geoms, %d total triangles, %d vertices\n",
               g_level->geom_count, total_tris, g_level->vertex_count);
        /* Dump first 5 geoms' materials */
        int dump_count = g_level->geom_count < 5 ? g_level->geom_count : 5;
        for (int di = 0; di < dump_count; di++) {
            mw_geom_t *dg = &g_level->geoms[di];
            printf("[Geom %d] amb=(%.2f,%.2f,%.2f,%.2f) dif=(%.2f,%.2f,%.2f,%.2f) tex=%d'%s' strips=%d\n",
                   di, dg->ambient[0],dg->ambient[1],dg->ambient[2],dg->ambient[3],
                   dg->diffuse[0],dg->diffuse[1],dg->diffuse[2],dg->diffuse[3],
                   dg->has_texture, dg->texture, dg->strip_count);
        }
        first_frame++;
    }
    
    /* Restore defaults */
    g_device->lpVtbl->SetTexture(g_device, 0, NULL);
    g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_CCW);
}

static void RenderLevelObjects(void) {
    if (!g_level) return;
    D3DMATRIX world;
    
    for (int i = 0; i < g_level->object_count; i++) {
        mw_object_t *obj = &g_level->objects[i];
        D3DMATERIAL8 mat;
        ZeroMemory(&mat, sizeof(mat));
        mat.Diffuse.a = 1.0f; mat.Ambient.a = 1.0f;

        switch (obj->type) {
        case MW_OBJ_START:
            mat.Diffuse.r = 0; mat.Diffuse.g = 1; mat.Diffuse.b = 0; break;
        case MW_OBJ_FLAG:
            mat.Diffuse.r = 1; mat.Diffuse.g = 1; mat.Diffuse.b = 0; break;
        case MW_OBJ_CAMERALOOKAT:
            mat.Diffuse.r = 1; mat.Diffuse.g = 0; mat.Diffuse.b = 1; break;
        case MW_OBJ_SAFESPOT:
            mat.Diffuse.r = 0; mat.Diffuse.g = 0.7f; mat.Diffuse.b = 0.7f; break;
        default:
            mat.Diffuse.r = 0.5f; mat.Diffuse.g = 0.5f; mat.Diffuse.b = 0.5f; break;
        }
        mat.Ambient = mat.Diffuse;
        g_device->lpVtbl->SetMaterial(g_device, &mat);

        ZeroMemory(&world, sizeof(world));
        world._11 = 1.0f; world._22 = 1.0f; world._33 = 1.0f; world._44 = 1.0f;
        world._41 = obj->position.x; world._42 = obj->position.y; world._43 = obj->position.z;
        g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &world);

        /* Render as small cube for each object */
        float size = 8.0f;
        struct { float x, y, z; } verts[] = {
            /* Front face (two triangles as strip) */
            {-size, 0, -size}, {-size, size, -size}, {size, 0, -size}, {size, size, -size},
            /* Right face */
            {size, 0, -size}, {size, size, -size}, {size, 0, size}, {size, size, size},
            /* Back face */
            {size, 0, size}, {size, size, size}, {-size, 0, size}, {-size, size, size},
            /* Left face */
            {-size, 0, size}, {-size, size, size}, {-size, 0, -size}, {-size, size, -size},
            /* Top face */
            {-size, size, -size}, {-size, size, size}, {size, size, -size}, {size, size, size},
            /* Bottom face */
            {-size, 0, -size}, {size, 0, -size}, {-size, 0, size}, {size, 0, size},
        };
        g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ);
        /* Draw as triangle strips */
        for (int f = 0; f < 6; f++) {
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, 
                &verts[f*4], sizeof(verts[0]));
        }
    }
}

static void RenderHUD(void) {
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, FALSE);

    /* Just display FPS */
    char fpsText[64];
    snprintf(fpsText, sizeof(fpsText), "FPS: %d | Pos(%.1f,%.1f,%.1f)", 
             g_current_fps, g_ball.x, g_ball.y, g_ball.z);
    SetWindowTextA(g_hwnd, fpsText);

    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);
}

static void Render(void) {
    if (!g_device) return;

    /* Clear with level background color (Section 4) or default */
    DWORD clear_color = D3DCOLOR_RGBA(40, 60, 100, 255);  /* Default blue-gray */
    if (g_level) {
        /* bg_color is stored as float [0..1] per channel */
        int r = (int)(g_level->bg_color.x * 255.0f);
        int g = (int)(g_level->bg_color.y * 255.0f);
        int b = (int)(g_level->bg_color.z * 255.0f);
        if (r < 0) r = 0; if (r > 255) r = 255;
        if (g < 0) g = 0; if (g > 255) g = 255;
        if (b < 0) b = 0; if (b > 255) b = 255;
        clear_color = D3DCOLOR_RGBA(r, g, b, 255);
        static int bg_printed = 0;
        if (bg_printed < 2) {
            printf("[Render] BG color: (%d, %d, %d) = 0x%08lx\n", r, g, b, clear_color);
            bg_printed++;
        }
    }
    g_device->lpVtbl->Clear(g_device, 0, NULL, 
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        clear_color, 1.0f, 0);
    g_device->lpVtbl->BeginScene(g_device);

    SetMatrices();
    RenderLevelGeometry();
    RenderLevelObjects();
    RenderBall();
    RenderHUD();

    g_device->lpVtbl->EndScene(g_device);
    g_device->lpVtbl->Present(g_device, NULL, NULL, NULL, NULL);
    
    /* Auto-screenshot after a few frames for testing */
    static int render_count = 0;
    render_count++;
    if (render_count == 5) {
        SaveScreenshot("tests/screenshots/reimpl_test31_textured.bmp");
        printf("[Render] Auto-screenshot saved\n");
    }
}

/* ===== Game Loop (mirrors App_Run 0x46BD80) ===== */
static void GameLoop(void) {
    g_last_tick = GetTickCount();
    g_fps_timer = g_last_tick;

    while (g_running) {
        MSG msg;
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { g_running = FALSE; break; }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!g_running) break;

        Sleep(0);

        DWORD now = GetTickCount();
        DWORD dt_ms = now - g_last_tick;

        if (dt_ms >= FRAME_TIME_MS || dt_ms > 1000) {
            float dt = (float)FRAME_TIME_MS / 1000.0f;
            if (dt_ms > 1000) dt = 1.0f / TARGET_FPS;

            HandleInput();
            UpdatePhysics(dt);

            if (!g_minimized) {
                Render();
            }

            g_last_tick = now;

            if ((GetTickCount() - g_last_tick) > 1000) {
                g_last_tick = GetTickCount();
            }
        }

        g_frame_counter++;
        now = GetTickCount();
        if (now - g_fps_timer >= 1000) {
            g_current_fps = g_frame_counter;
            g_frame_counter = 0;
            g_fps_timer = now;
        }
    }
}

/* ===== Cleanup ===== */
static void Cleanup(void) {
    texture_system_shutdown();
    if (g_level) meshworld_free(g_level);
    if (g_mouse) { g_mouse->lpVtbl->Unacquire(g_mouse); g_mouse->lpVtbl->Release(g_mouse); }
    if (g_keyboard) { g_keyboard->lpVtbl->Unacquire(g_keyboard); g_keyboard->lpVtbl->Release(g_keyboard); }
    if (g_dinput) g_dinput->lpVtbl->Release(g_dinput);
    if (g_primary_buffer) g_primary_buffer->lpVtbl->Release(g_primary_buffer);
    if (g_dsound) g_dsound->lpVtbl->Release(g_dsound);
    if (g_device) g_device->lpVtbl->Release(g_device);
    if (g_d3d) g_d3d->lpVtbl->Release(g_d3d);
    if (g_hwnd) DestroyWindow(g_hwnd);
    UnregisterClassA(WINDOW_CLASS, g_hinst);
}

/* ===== WinMain ===== */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int show) {
    printf("Hamsterball Reimplementation (D3D8/DInput8/DSound8)\n");

    if (!InitWindow(hInst)) { Cleanup(); return 1; }
    if (!InitD3D8()) { Cleanup(); return 1; }
    if (!InitDInput8()) { Cleanup(); return 1; }
    InitDSound8();
    if (!LoadAssets()) { Cleanup(); return 1; }

    g_state = STATE_RACING;
    GameLoop();
    Cleanup();
    return 0;
}