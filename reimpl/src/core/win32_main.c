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
#include "audio/sfx_dsound.h"

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
#define BALL_DAMPING    0.5f               /* _DAT_004CF3F0: per-frame velocity damping */
#define BALL_GRAVITY    0.15f              /* Ball+0xC94: default gravity (NOT 0.5) */
#define BALL_SPEED_SCALE 1.05f             /* Ball+0x18C */
#define BALL_SPEED_FRICTION 0.85f         /* _DAT_004CF4C0: high-speed friction */

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

/* Sky texture */
static texture_t *g_sky_tex = NULL;

/* Countdown snap flag: file-scope so ResetBallAndCamera can reset it */
static int s_countdown_snap_done = 0;

/* Race flow state */
typedef enum {
    RACE_COUNTDOWN = 0,
    RACE_RUNNING,
    RACE_FINISHED,
    RACE_RESPAWNING
} RaceState;
static RaceState g_race_state = RACE_COUNTDOWN;
static float g_countdown_timer = 3.0f;   /* 3.0→0.0 then GO */
static float g_race_time = 0.0f;         /* Elapsed since GO */
static float g_best_time = 99999.0f;     /* Best finish time */
static int g_current_checkpoint = -1;    /* Last passed SAFESPOT object index */
static BOOL g_checkpoint_passed[128];    /* Passed flags per object */

/* Level list — press [ or ] to cycle at runtime, or pass -level NAME on command line */
static const char *g_level_list[] = {
    "Level1", "Level2", "Level3", "Level4", "Level5",
    "Level6", "Level7", "Level8", "Level9", "Level10",
    "Arena-WarmUp", "Arena-Intermediate", "Arena-Expert", "Arena-Toob", "Arena-SpawnPlatform",
    NULL
};
static int g_level_index = 0; /* default: Level1 (WarmUp, fastest for Wine sw render) */
static int g_level_count = 0; /* computed at startup */
static int g_prev_keys_bracket = 0; /* debounce for [ ] keys */

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
    float tx, ty, tz;    /* Look-at target (lerps toward desired target) */
    float orbit_angle;   /* Scene+0x29BC: orbital angle (radians) */
    float orbit_dist;    /* Scene+0x29C0: distance from ball (800.0 arena) */
    float orbit_tilt;    /* Y component of orbit direction (0.9 = slightly above) */
} g_camera;

/* CAMERALOOKAT target from MESHWORLD (arena center point) */
static float g_camlookat_x = 0, g_camlookat_y = 0, g_camlookat_z = 0;
static BOOL g_has_camlookat = FALSE;

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
static void RenderBallShadow(void);
static void DrawSpherePrimitive(void);
static void RenderLevelGeometry(void);
static void RenderLevelObjects(void);
static void RenderHUD(void);
/* static void RenderCheckpointMarkers(void); */
static void UpdateCountdown(float dt);
static void UpdateCheckpoints(void);
static void RespawnBall(void);

/* Forward declaration for collision system */
typedef struct { float nx, ny, nz; float depth; float cx, cy, cz; int tri_index; } CollisionResult;
static int TestSphereVsLevel(float sx, float sy, float sz, float radius, CollisionResult *results, int max_results);
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
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_AMBIENT, D3DCOLOR_RGBA(180, 185, 200, 255));
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
    sfx_init(g_dsound);
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

static void ResetRaceState(void);

/* Reset ball position and camera after level load */
static void ResetBallAndCamera(void) {
    g_ball.vx = g_ball.vy = g_ball.vz = 0;
    g_ball.radius = BALL_RADIUS;
    g_ball.max_speed = BALL_MAX_SPEED;
    g_ball.speed_scale = BALL_SPEED_SCALE;
    g_ball.damping = BALL_DAMPING;
    g_ball.gravity = BALL_GRAVITY;
    g_has_camlookat = FALSE;

    /* Default ball position */
    g_ball.x = 0;
    g_ball.y = 52.0f;
    g_ball.z = 0;

    if (g_level && g_level->vertex_count > 0) {
        /* Find topmost surface from upward-facing triangles as fallback for race tracks */
        float top_floor_y = g_level->bounds_min.y;
        float top_floor_x = 0, top_floor_z = 0;
        int found_geometry_floor = 0;
        if (g_level->indices && g_level->index_count >= 3) {
            for (int i = 0; i + 2 < g_level->index_count; i += 3) {
                int i0 = g_level->indices[i], i1 = g_level->indices[i+1], i2 = g_level->indices[i+2];
                if (i0 >= g_level->vertex_count || i1 >= g_level->vertex_count || i2 >= g_level->vertex_count) continue;
                float ny = (g_level->vertices[i0].ny + g_level->vertices[i1].ny + g_level->vertices[i2].ny) / 3.0f;
                float ty = (g_level->vertices[i0].y + g_level->vertices[i1].y + g_level->vertices[i2].y) / 3.0f;
                if (ny > 0.5f && ty > top_floor_y + 20.0f) {
                    top_floor_y = ty;
                    top_floor_x = (g_level->vertices[i0].x + g_level->vertices[i1].x + g_level->vertices[i2].x) / 3.0f;
                    top_floor_z = (g_level->vertices[i0].z + g_level->vertices[i1].z + g_level->vertices[i2].z) / 3.0f;
                    found_geometry_floor = 1;
                }
            }
        }
        if (!found_geometry_floor) {
            top_floor_y = g_level->bounds_min.y;
            top_floor_x = 0; top_floor_z = 0;
        }

        /* Find START and CAMERALOOKAT objects */
        float start_x = 0, start_y_obj = 0, start_z = 0;
        int has_start = 0;
        for (int i = 0; i < g_level->object_count; i++) {
            mw_object_t *obj = &g_level->objects[i];
            if (obj->type == MW_OBJ_START) {
                /* Pick START1-* for single-player; only overwrite if no START1 yet */
                if (!has_start || strncmp(obj->type_string, "START1-", 7) == 0) {
                    start_x = obj->position.x;
                    start_y_obj = obj->position.y;
                    start_z = obj->position.z;
                    has_start = 1;
                }
            }
            if (obj->type == MW_OBJ_CAMERALOOKAT) {
                g_camlookat_x = obj->position.x;
                g_camlookat_y = obj->position.y;
                g_camlookat_z = obj->position.z;
                g_has_camlookat = TRUE;
            }
        }

        /* Determine spawn strategy:
         * Race tracks (no CAMERALOOKAT): START marks the race start. Place at START XZ
         *   with Y+radius above the surface. Arena (no CAMERALOOKAT): do same.
         *   In both cases, countdown-snap will push ball to actual surface if geometry exists. */
        if (has_start) {
            g_ball.x = start_x;
            g_ball.y = start_y_obj + g_ball.radius + 1.0f;
            g_ball.z = start_z;
        } else if (found_geometry_floor) {
            /* Fallback: no START object, use highest upward-facing triangle */
            g_ball.x = top_floor_x;
            g_ball.y = top_floor_y + g_ball.radius + 1.0f;
            g_ball.z = top_floor_z;
        }

        /* Probe: if no mesh collision at spawn, scan downward to find actual surface.
         * This catches levels where START.y places ball above the track. */
        if (g_level && g_level->index_count >= 3 && has_start) {
            CollisionResult probe[8];
            int nhits = TestSphereVsLevel(g_ball.x, g_ball.y, g_ball.z, g_ball.radius, probe, 8);
            if (nhits == 0) {
                printf("[Spawn] No nearby geometry at (%.1f,%.1f,%.1f), scanning downward...\n",
                       g_ball.x, g_ball.y, g_ball.z);
                int found = 0;
                for (float scan_y = g_ball.y - 50.0f; scan_y >= g_ball.y - 2000.0f; scan_y -= 50.0f) {
                    nhits = TestSphereVsLevel(g_ball.x, scan_y, g_ball.z, g_ball.radius, probe, 8);
                    if (nhits > 0) {
                        g_ball.y = probe[0].cy + g_ball.radius + 1.0f;
                        printf("[Spawn] Snapped to floor below at Y=%.1f\n", g_ball.y);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("[Spawn] WARNING: No geometry found at START.\n");
                }
            }
        }
        ResetRaceState();
    }

    /* Reset camera — isometric-style view matching original game.
     * Original uses CameraLookAt (0x413280) with orbit_tilt ~0.9 (42° from horizontal)
     * and orbit_dist adapted to level size. */
    g_camera.tx = g_ball.x; g_camera.ty = g_ball.y; g_camera.tz = g_ball.z;
    g_camera.orbit_angle = -0.7854f;  /* -PI/4: classic isometric angle */
    if (g_level && g_level->vertex_count > 0) {
        /* Race tracks: 400 dist, 0.9 tilt.
         * Arena levels: 500 dist, 0.9 tilt to see the bowl. */
        if (g_has_camlookat) {
            g_camera.orbit_dist = 500.0f;
            g_camera.orbit_tilt = 0.9f;
        } else {
            g_camera.orbit_dist = 400.0f;
            g_camera.orbit_tilt = 0.9f;  /* 42° from horizontal, matching original */
        }
    } else {
        g_camera.orbit_dist = 400.0f;
        g_camera.orbit_tilt = 0.9f;
    }

    printf("[Spawn] Ball at (%.1f, %.1f, %.1f) camlookat=%s\n",
           g_ball.x, g_ball.y, g_ball.z, g_has_camlookat ? "YES" : "NO");
}

/* Reset race state at level start — called from ResetBallAndCamera */
static void ResetRaceState(void) {
    g_race_state = RACE_COUNTDOWN;
    g_countdown_timer = 3.0f;
    g_race_time = 0.0f;
    g_current_checkpoint = -1;
    memset(g_checkpoint_passed, 0, sizeof(g_checkpoint_passed));
    s_countdown_snap_done = 0;  /* Reset so next countdown will snap again */
    printf("[Race] Countdown: 3.0...\n");
}

/* Switch to a level by index, reset ball to spawn. Returns TRUE on success. */
static BOOL SwitchLevel(int index) {
    if (index < 0 || index >= g_level_count) return FALSE;
    g_level_index = index;
    if (!LoadLevel(g_level_list[g_level_index])) return FALSE;
    ResetBallAndCamera();
    printf("[Level] Now: %s ([ ] to cycle)\n", g_level_list[g_level_index]);
    return TRUE;
}

/* ===== Load Assets ===== */
static BOOL LoadAssets(void) {
    if (!FindGameDir()) return FALSE;

    /* Count levels in list */
    g_level_count = 0;
    while (g_level_list[g_level_count]) g_level_count++;

    /* Check for -level NAME on command line */
    const char *cmd_level = NULL;
    for (int i = 1; i < __argc; i++) {
        if (strcmp(__argv[i], "-level") == 0 && i + 1 < __argc) {
            cmd_level = __argv[i + 1];
            break;
        }
    }

    if (cmd_level) {
        /* Find matching index */
        for (int i = 0; i < g_level_count; i++) {
            if (strcmp(g_level_list[i], cmd_level) == 0) { g_level_index = i; break; }
        }
    }

    /* Try to load the selected level, fall back through the list */
    if (!LoadLevel(g_level_list[g_level_index])) {
        /* Try each level until one works */
        for (g_level_index = 0; g_level_index < g_level_count; g_level_index++) {
            if (LoadLevel(g_level_list[g_level_index])) break;
        }
        if (g_level_index >= g_level_count) return FALSE;
    }

    printf("[Level] Loaded: %s (use [ ] keys to cycle levels)\n", g_level_list[g_level_index]);

    ResetBallAndCamera();

    /* Initialize texture system after D3D device is ready */
    {
        char tex_dir[MAX_PATH];
        snprintf(tex_dir, sizeof(tex_dir), "%s/Textures", g_game_dir);
        texture_set_device((void*)g_device);
        texture_system_init(tex_dir);
        
        /* Determine sky texture from MESHWORLD file trailing strings.
         * Each level references its own sky: Arena-Sky=SkyChecker, Level9=Clouds. */
        const char *sky_name = NULL;
        {
            char mw_path[MAX_PATH];
            snprintf(mw_path, sizeof(mw_path), "%s/Levels/%s.MESHWORLD",
                     g_game_dir, g_level_list[g_level_index]);
            FILE *fp = fopen(mw_path, "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                long sz = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                char *buf = malloc(sz);
                if (buf && fread(buf, 1, sz, fp) == (size_t)sz) {
                    long i;
                    for (i = 0; i <= sz - 10; i++) {
                        if (memcmp(buf + i, "Clouds.png", 10) == 0) {
                            sky_name = "Clouds.png"; break;
                        }
                        if (memcmp(buf + i, "SkyChecker.png", 14) == 0) {
                            sky_name = "SkyChecker.png"; break;
                        }
                        if (memcmp(buf + i, "SkyCheckerFlag.png", 18) == 0) {
                            sky_name = "SkyCheckerFlag.png"; break;
                        }
                        if (memcmp(buf + i, "Tourney-Sky.png", 14) == 0) {
                            sky_name = "Tourney-Sky.png"; break;
                        }
                    }
                }
                free(buf);
                fclose(fp);
            }
        }
        if (sky_name) {
            g_sky_tex = texture_load(sky_name);
            if (g_sky_tex) printf("[Sky] Loaded %s for this level\n", sky_name);
        }
        
        /* Pre-load all textures referenced by level geoms */
        if (g_level) {
            for (int g = 0; g < g_level->geom_count; g++) {
                if (g_level->geoms[g].has_texture && g_level->geoms[g].texture[0]) {
                    texture_load(g_level->geoms[g].texture);
                }
            }
        }
    }

    /* Load sound effects */
    {
        char sfx_dir[MAX_PATH];
        snprintf(sfx_dir, sizeof(sfx_dir), "%s/Sounds", g_game_dir);
        const char *sfx_names[] = { "Collide", "DropIn", "GOAL", "Bell", "Bumper", NULL };
        for (int i = 0; sfx_names[i]; i++) {
            char path[MAX_PATH];
            snprintf(path, sizeof(path), "%s/%s.ogg", sfx_dir, sfx_names[i]);
            sfx_load_ogg(sfx_names[i], path);
        }
    }

    printf("[Load] Ball at (%.1f, %.1f, %.1f)\n", g_ball.x, g_ball.y, g_ball.z);
    return TRUE;
}

/* ===== Countdown: 3.0 → 0.0 then GO ===== */
static void UpdateCountdown(float dt) {
    if (g_race_state == RACE_COUNTDOWN) {
        g_countdown_timer -= dt;
        if (g_countdown_timer <= 0.0f) {
            g_race_state = RACE_RUNNING;
            g_countdown_timer = 0.0f;
            g_race_time = 0.0f;
            printf("[Race] GO!\n");
        }
    } else if (g_race_state == RACE_RUNNING) {
        g_race_time += dt;
    }
}

/* ===== Checkpoint Detection =====
 * Check if ball crosses near a SAFESPOT object. Respawn uses last passed SAFESPOT.
 * Also detect GOAL crossing for race finish.
 */
static void UpdateCheckpoints(void) {
    if (!g_level || g_race_state == RACE_FINISHED || g_race_state == RACE_COUNTDOWN) return;
    for (int i = 0; i < g_level->object_count; i++) {
        mw_object_t *obj = &g_level->objects[i];
        if (obj->type == MW_OBJ_SAFESPOT) {
            if (g_checkpoint_passed[i]) continue;
            float dx = g_ball.x - obj->position.x;
            float dz = g_ball.z - obj->position.z;
            float dy = g_ball.y - obj->position.y;
            float dist_sq = dx*dx + dy*dy + dz*dz;
            if (dist_sq < 10000.0f) {
                g_current_checkpoint = i;
                g_checkpoint_passed[i] = TRUE;
                printf("[Checkpoint] SAFESPOT %d at (%.1f,%.1f,%.1f)\n",
                       i, obj->position.x, obj->position.y, obj->position.z);
            }
        } else if (obj->type == MW_OBJ_GOAL || strstr(obj->type_string, "GOAL") || strstr(obj->type_string, "FINISH")) {
            /* GOAL detection — finish line */
            float dx = g_ball.x - obj->position.x;
            float dz = g_ball.z - obj->position.z;
            float dy = g_ball.y - obj->position.y;
            float dist_sq = dx*dx + dy*dy + dz*dz;
            if (dist_sq < 1600.0f && g_race_state == RACE_RUNNING) {
                g_race_state = RACE_FINISHED;
                if (g_race_time < g_best_time) g_best_time = g_race_time;
                printf("[RACE FINISHED] Time: %.3f seconds (Best: %.3f)\n", g_race_time, g_best_time);
                sfx_play("GOAL");
            }
        }
    }
}

/* ===== Respawn Ball =====
 * Respawn at current checkpoint (or START if none passed)
 */
static void RespawnBall(void) {
    if (g_current_checkpoint >= 0) {
        g_ball.x = g_level->objects[g_current_checkpoint].position.x;
        g_ball.y = g_level->objects[g_current_checkpoint].position.y + g_ball.radius + 2.0f;
        g_ball.z = g_level->objects[g_current_checkpoint].position.z;
    } else if (g_level) {
        /* Respawn at START */
        for (int i = 0; i < g_level->object_count; i++) {
            if (g_level->objects[i].type == MW_OBJ_START) {
                g_ball.x = g_level->objects[i].position.x;
                g_ball.y = g_level->objects[i].position.y + g_ball.radius + 2.0f;
                g_ball.z = g_level->objects[i].position.z;
                break;
            }
        }
        
        /* Fix: Check if START position has geometry below. If not (Level3 issue),
         * do collision scan in both directions to find the floor surface.
         * Level3 START is at (192, -85, 1359) but geometry is ~300 units below at Y~-2000. */
        if (g_level && g_level->index_count > 0) {
            CollisionResult hits[8];
            int nhits = TestSphereVsLevel(g_ball.x, g_ball.y, g_ball.z, g_ball.radius, hits, 8);
            if (nhits == 0) {
                printf("[Spawn] No geometry at START (%g,%g,%g), scanning...\n",
                       g_ball.x, g_ball.y, g_ball.z);
                /* Scan both upward and downward to find geometry */
                int found = 0;
                /* Scan downward first (gravity direction in D3D, but we scan both) */
                for (float scan_y = g_ball.y - 50.0f; scan_y >= g_ball.y - 2000.0f; scan_y -= 50.0f) {
                    nhits = TestSphereVsLevel(g_ball.x, scan_y, g_ball.z, g_ball.radius, hits, 8);
                    if (nhits > 0) {
                        g_ball.y = hits[0].cy + g_ball.radius + 1.0f;
                        printf("[Spawn] Snapped to floor below at Y=%.1f\n", g_ball.y);
                        found = 1;
                        break;
                    }
                }
                /* If not found below, scan upward */
                if (!found) {
                    for (float scan_y = g_ball.y + 50.0f; scan_y <= g_ball.y + 500.0f; scan_y += 50.0f) {
                        nhits = TestSphereVsLevel(g_ball.x, scan_y, g_ball.z, g_ball.radius, hits, 8);
                        if (nhits > 0) {
                            g_ball.y = hits[0].cy + g_ball.radius + 1.0f;
                            printf("[Spawn] Snapped to floor above at Y=%.1f\n", g_ball.y);
                            found = 1;
                            break;
                        }
                    }
                }
                if (!found) {
                    printf("[Spawn] WARNING: No geometry found, using fallback Y\n");
                    /* Fallback: use a reasonable default for Level3 */
                    g_ball.y = -1975.0f + g_ball.radius + 1.0f;
                }
            }
        }
    }
    g_ball.vx = g_ball.vy = g_ball.vz = 0;
    g_race_state = RACE_COUNTDOWN;
    g_countdown_timer = 3.0f;
    printf("[Race] Respawned. Countdown: 3.0...\n");
}

/* ===== End of Checkpoint/Race System ===== */

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

    /* R = manual respawn at last checkpoint (or START if none passed) */
    {
        static int prev_r = 0;
        int r_now = (g_keys[DIK_R] & 0x80) ? 1 : 0;
        if (r_now && !prev_r) {
            RespawnBall();
            printf("[Input] Respawn pressed (R)\n");
        }
        prev_r = r_now;
    }

    /* [ and ] keys cycle through levels at runtime */
    {
        int bracket_now = 0;
        if (g_keys[DIK_LBRACKET] & 0x80) bracket_now = 1;  /* [ = prev level */
        if (g_keys[DIK_RBRACKET] & 0x80) bracket_now = 2;  /* ] = next level */
        if (bracket_now && !g_prev_keys_bracket) {
            int next = g_level_index;
            if (bracket_now == 1) next = (g_level_index - 1 + g_level_count) % g_level_count;
            if (bracket_now == 2) next = (g_level_index + 1) % g_level_count;
            SwitchLevel(next);
        }
        g_prev_keys_bracket = bracket_now;
    }
}

/* ===== Triangle Collision System =====
 * Original uses Ball_AdvancePositionOrCollision (0x4564C0) with a 6-phase physics
 * pipeline. Phase 4 is collision detection against MESHWORLD geometry.
 * 
 * We implement sphere-vs-triangle collision:
 * 1. For each triangle in the level, check if sphere overlaps it
 * 2. Compute penetration depth and push sphere out along triangle normal
 * 3. Response: reflect velocity component into the surface, apply friction
 *
 * Optimization: only test triangles within a bounding box around the ball.
 * For WarmUp (2320 verts, ~770 tris) brute-force is fast enough.
 */

/* Compute cross product */
static void Vec3Cross(float ax, float ay, float az,
                     float bx, float by, float bz,
                     float *ox, float *oy, float *oz) {
    *ox = ay * bz - az * by;
    *oy = az * bx - ax * bz;
    *oz = ax * by - ay * bx;
}

/* Compute dot product */
static float Vec3Dot(float ax, float ay, float az,
                     float bx, float by, float bz) {
    return ax * bx + ay * by + az * bz;
}

/* Closest point on triangle to point P (Möller–Trumbore barycentric approach) */
static void ClosestPointOnTriangle(float px, float py, float pz,
                                    float ax, float ay, float az,
                                    float bx, float by, float bz,
                                    float cx, float cy, float cz,
                                    float *outx, float *outy, float *outz) {
    /* Edges */
    float e0x = bx - ax, e0y = by - ay, e0z = bz - az;
    float e1x = cx - ax, e1y = cy - ay, e1z = cz - az;
    float dx = px - ax, dy = py - ay, dz = pz - az;
    
    float d00 = Vec3Dot(e0x, e0y, e0z, e0x, e0y, e0z);
    float d01 = Vec3Dot(e0x, e0y, e0z, e1x, e1y, e1z);
    float d11 = Vec3Dot(e1x, e1y, e1z, e1x, e1y, e1z);
    float d20 = Vec3Dot(dx, dy, dz, e0x, e0y, e0z);
    float d21 = Vec3Dot(dx, dy, dz, e1x, e1y, e1z);
    
    float denom = d00 * d11 - d01 * d01;
    float v = 0.0f, w = 0.0f;
    if (fabsf(denom) > 1e-8f) {
        v = (d11 * d20 - d01 * d21) / denom;
        w = (d00 * d21 - d01 * d20) / denom;
    }
    
    /* Clamp to triangle (Voronoi region classification) */
    if (v < 0.0f) { v = 0.0f; w = (d21 > 0) ? 0.0f : (d21 < d11 ? 1.0f : -d21 / d11); }
    else if (w < 0.0f) { w = 0.0f; v = (d20 > 0) ? 0.0f : (d20 < d00 ? 1.0f : -d20 / d00); }
    
    /* Clamp v+w to 1 (edge BC) */
    if (v + w > 1.0f) {
        float f = 1.0f / (v + w);
        v *= f;
        w *= f;
        /* Re-clamp after scaling */
        if (v < 0.0f) v = 0.0f;
        if (w < 0.0f) w = 0.0f;
        if (v + w > 1.0f) { v = 1.0f - w; }
    }
    
    float u = 1.0f - v - w;
    *outx = u * ax + v * bx + w * cx;
    *outy = u * ay + v * by + w * cy;
    *outz = u * az + v * bz + w * cz;
}

/* Collision system constants */
#define MAX_COLLISIONS 16
#define MAX_SHADOW_HITS 4

/* Test sphere (center + radius) against all level triangles
 * NOTE: CollisionResult typedef and TestSphereVsLevel prototype are at top of file */
static int TestSphereVsLevel(float sx, float sy, float sz, float radius,
                              CollisionResult *results, int max_results) {
    if (!g_level || g_level->index_count < 3) return 0;
    
    int count = 0;
    int num_tris = g_level->index_count / 3;
    
    for (int t = 0; t < num_tris && count < max_results; t++) {
        int i0 = g_level->indices[t * 3 + 0];
        int i1 = g_level->indices[t * 3 + 1];
        int i2 = g_level->indices[t * 3 + 2];
        
        if (i0 >= g_level->vertex_count || i1 >= g_level->vertex_count ||
            i2 >= g_level->vertex_count) continue;
        
        mw_vertex_t *v0 = &g_level->vertices[i0];
        mw_vertex_t *v1 = &g_level->vertices[i1];
        mw_vertex_t *v2 = &g_level->vertices[i2];
        
        /* Quick AABB pre-filter: skip triangles far from sphere */
        float tri_minx = v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x);
        float tri_miny = v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y);
        float tri_minz = v0->z < v1->z ? (v0->z < v2->z ? v0->z : v2->z) : (v1->z < v2->z ? v1->z : v2->z);
        float tri_maxx = v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x);
        float tri_maxy = v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y);
        float tri_maxz = v0->z > v1->z ? (v0->z > v2->z ? v0->z : v2->z) : (v1->z > v2->z ? v1->z : v2->z);
        
        /* Expand AABB by sphere radius for pre-filter.
         * Use a generous margin to avoid missing thin geometry */
        float margin = radius * 2.0f;
        if (tri_maxx < sx - margin || tri_minx > sx + margin ||
            tri_maxy < sy - margin || tri_miny > sy + margin ||
            tri_maxz < sz - margin || tri_minz > sz + margin) continue;
        
        /* Find closest point on triangle to sphere center */
        float cpx, cpy, cpz;
        ClosestPointOnTriangle(sx, sy, sz,
                                v0->x, v0->y, v0->z,
                                v1->x, v1->y, v1->z,
                                v2->x, v2->y, v2->z,
                                &cpx, &cpy, &cpz);
        
        /* Distance from sphere center to closest point */
        float dx = sx - cpx, dy = sy - cpy, dz = sz - cpz;
        float dist_sq = dx * dx + dy * dy + dz * dz;
        
        if (dist_sq < radius * radius) {
            float dist = sqrtf(dist_sq);
            float nx, ny, nz;
            
            if (dist > 1e-6f) {
                /* Normal from closest point toward sphere center */
                nx = dx / dist;
                ny = dy / dist;
                nz = dz / dist;
            } else {
                /* Sphere center is ON the triangle — use triangle face normal */
                float e1x = v1->x - v0->x, e1y = v1->y - v0->y, e1z = v1->z - v0->z;
                float e2x = v2->x - v0->x, e2y = v2->y - v0->y, e2z = v2->z - v0->z;
                Vec3Cross(e1x, e1y, e1z, e2x, e2y, e2z, &nx, &ny, &nz);
                float nlen = sqrtf(nx * nx + ny * ny + nz * nz);
                if (nlen > 1e-6f) { nx /= nlen; ny /= nlen; nz /= nlen; }
                else { nx = 0; ny = 1; nz = 0; }
                
                /* Make sure normal points away from triangle toward sphere */
                if (Vec3Dot(nx, ny, nz, dx, dy, dz) < 0) {
                    nx = -nx; ny = -ny; nz = -nz;
                }
            }
            
            results[count].nx = nx;
            results[count].ny = ny;
            results[count].nz = nz;
            results[count].depth = radius - dist;
            results[count].cx = cpx;
            results[count].cy = cpy;
            results[count].cz = cpz;
            results[count].tri_index = t;
            count++;
        }
    }
    
    return count;
}

/* ===== Physics: Ball_AdvancePositionOrCollision (0x4564C0) =====
 * Simplified from original's 6-phase pipeline:
 *   Phase 1: Gravity
 *   Phase 2: Input force
 *   Phase 3: Velocity damping
 *   Phase 4: Collision detection + response (NEW — was y=0 plane before)
 *   Phase 5: Position integration
 *   Phase 6: Speed clamping
 */
static void UpdatePhysics(float dt) {
    /* During countdown: no gravity, no input, no movement — freeze ball on spawn surface.
     * Still run collision to keep ball from falling through if spawn is slightly above ground. */
    if (g_race_state == RACE_COUNTDOWN) {
        /* Zero velocity, zero input */
        g_ball.vx = g_ball.vy = g_ball.vz = 0;
        g_ball.input_fx = g_ball.input_fy = 0;
        /* One collision pass to snap to surface on FIRST frame of countdown */
        if (!s_countdown_snap_done) {
            s_countdown_snap_done = 1;
            if (g_level && g_level->index_count >= 3) {
                CollisionResult hits[MAX_COLLISIONS];
                int nhits = TestSphereVsLevel(g_ball.x, g_ball.y, g_ball.z, g_ball.radius, hits, MAX_COLLISIONS);
                /* If no hits (START is inside/below geometry on some levels), sweep upward from below
                 * to find the floor surface. Try up to 1000 units below. */
                if (nhits == 0) {
                    for (float scan_y = g_ball.y - 50.0f; scan_y > g_ball.y - 1000.0f; scan_y -= 25.0f) {
                        nhits = TestSphereVsLevel(g_ball.x, scan_y, g_ball.z, g_ball.radius, hits, MAX_COLLISIONS);
                        if (nhits > 0) break;
                    }
                    if (nhits > 0) {
                        int deepest = 0;
                        for (int i = 1; i < nhits; i++) if (hits[i].depth > hits[deepest].depth) deepest = i;
                        /* Use scan position for displacement, not original g_ball.y.
                         * Place ball at closest_point + normal * radius to sit on surface. */
                        g_ball.x = hits[deepest].cx + hits[deepest].nx * g_ball.radius * 1.01f;
                        g_ball.y = hits[deepest].cy + hits[deepest].ny * g_ball.radius * 1.01f;
                        g_ball.z = hits[deepest].cz + hits[deepest].nz * g_ball.radius * 1.01f;
                    }
                }
            }
        }
        return;
    }

    /* Reset countdown snap flag when race restarts */
    if (g_race_state == RACE_RUNNING || g_race_state == RACE_RESPAWNING) {
        s_countdown_snap_done = 0;
    }

    /* Respawning: freeze, countdown, no physics */
    if (g_race_state == RACE_RESPAWNING) return;

    /* Finished: don't update physics, just sit there */
    if (g_race_state == RACE_FINISHED) return;

    /* Phase 1: Gravity (Ball+0x1A8 / Ball+0xC94: default = 0.15)
     * Original applies in world units where 1 unit ≈ 1 game cm.
     * Effective gravity ≈ 490 world units/s² (tuned to feel like the original) */
    g_ball.vy -= g_ball.gravity * 500.0f * dt;

    /* Phase 2: Input force (Ball_GetInputForce 0x46EC30) */
    /* Camera-relative input: rotate by orbit_angle so UP = forward on screen */
    float cos_a = cosf(g_camera.orbit_angle);
    float sin_a = sinf(g_camera.orbit_angle);
    float input_wx = g_ball.input_fx * cos_a - g_ball.input_fy * sin_a;
    float input_wz = g_ball.input_fx * sin_a + g_ball.input_fy * cos_a;
    
    float force_scale = g_ball.speed_scale * 1000.0f;
    g_ball.vx += input_wx * force_scale * dt;
    g_ball.vz += input_wz * force_scale * dt;

    /* Phase 3: Velocity damping
     * Low speed: heavy damping (0.5 = lose 50% per step — fine control)
     * High speed: lighter damping (0.85 = lose 15% per step — momentum) */
    float speed_h = sqrtf(g_ball.vx * g_ball.vx + g_ball.vz * g_ball.vz);
    float damp_factor = (speed_h > 100.0f) ? 0.85f : 0.5f;
    g_ball.vx *= damp_factor;
    g_ball.vz *= damp_factor;

    /* Phase 5: Integrate position (with substeps for fast-moving ball) */
    float speed_before = sqrtf(g_ball.vx * g_ball.vx + g_ball.vy * g_ball.vy + g_ball.vz * g_ball.vz);
    int substeps = 1;
    /* If ball moves more than radius per frame, use substeps to prevent tunneling */
    float move_dist = speed_before * dt;
    if (move_dist > g_ball.radius * 0.5f) {
        substeps = (int)(move_dist / (g_ball.radius * 0.5f)) + 1;
        if (substeps > 8) substeps = 8;  /* Cap to prevent spiral of death */
    }
    
    float sub_dt = dt / (float)substeps;
    for (int ss = 0; ss < substeps; ss++) {
        g_ball.x += g_ball.vx * sub_dt;
        g_ball.y += g_ball.vy * sub_dt;
        g_ball.z += g_ball.vz * sub_dt;
        
        /* Phase 4: Collision detection + response against level geometry */
        CollisionResult hits[MAX_COLLISIONS];
        int nhits = TestSphereVsLevel(g_ball.x, g_ball.y, g_ball.z, g_ball.radius,
                                       hits, MAX_COLLISIONS);
        
        /* Resolve collisions iteratively (up to 4 passes for corners) */
        int had_ground_contact = 0;
        float ground_nx = 0, ground_ny = 0, ground_nz = 0;
        for (int pass = 0; pass < 4 && nhits > 0; pass++) {
            /* Find the deepest penetration */
            int deepest = 0;
            for (int i = 1; i < nhits; i++) {
                if (hits[i].depth > hits[deepest].depth) deepest = i;
            }
            
            CollisionResult *hit = &hits[deepest];
            
            /* Track ground contact normal for slope gravity */
            if (hit->ny > 0.5f) {
                had_ground_contact = 1;
                ground_nx = hit->nx;
                ground_ny = hit->ny;
                ground_nz = hit->nz;
            }
            
            /* Push ball out of surface along collision normal */
            g_ball.x += hit->nx * hit->depth * 1.01f;  /* Slightly over-resolve to prevent sticking */
            g_ball.y += hit->ny * hit->depth * 1.01f;
            g_ball.z += hit->nz * hit->depth * 1.01f;
            
            /* Velocity response: remove component going INTO the surface */
            float vel_dot_n = Vec3Dot(g_ball.vx, g_ball.vy, g_ball.vz,
                                       hit->nx, hit->ny, hit->nz);
            
            if (vel_dot_n < 0.0f) {
                /* Decompose velocity into normal and tangential */
                float vn_x = hit->nx * vel_dot_n;
                float vn_y = hit->ny * vel_dot_n;
                float vn_z = hit->nz * vel_dot_n;
                float vt_x = g_ball.vx - vn_x;
                float vt_y = g_ball.vy - vn_y;
                float vt_z = g_ball.vz - vn_z;
                
                /* Bounce: reflect normal component with restitution */
                float bounce = 0.3f;  /* Coefficient of restitution */
                
                /* Apply bounce to normal component and friction to tangential */
                float friction = 0.98f;
                float new_vx = vt_x * friction - vn_x * bounce;
                float new_vy = vt_y * friction - vn_y * bounce;
                float new_vz = vt_z * friction - vn_z * bounce;
                g_ball.vx = new_vx;
                g_ball.vy = new_vy;
                g_ball.vz = new_vz;
                
                /* Play collision sound on significant impact */
                if (vel_dot_n < -50.0f) {
                    static int sfx_debounce = 0;
                    if (sfx_debounce++ % 4 == 0) sfx_play("Collide");
                }
                
                /* Ground detection: if normal is mostly pointing up, ball is on ground */
                if (hit->ny > 0.5f) {
                    /* Kill residual vertical velocity when on near-flat ground */
                    if (fabsf(g_ball.vy) < 5.0f) g_ball.vy = 0.0f;
                }
            }
            
            /* Re-test after resolution (for corner cases) */
            if (pass < 3) {
                nhits = TestSphereVsLevel(g_ball.x, g_ball.y, g_ball.z, g_ball.radius,
                                           hits, MAX_COLLISIONS);
            }
        }
        
        /* Slope gravity: accelerate ball along surface tangent when on ground */
        if (had_ground_contact) {
            float slope_accel = g_ball.gravity * 500.0f;
            g_ball.vx += slope_accel * ground_nx * ground_ny * sub_dt;
            g_ball.vz += slope_accel * ground_nz * ground_ny * sub_dt;
        }
        
        /* Log first 3 frames only for debugging */
        if (ss == 0) {
            static int collision_log_count = 0;
            if (collision_log_count < 3) {
                printf("[Collision] nhits=%d pos=(%.1f,%.1f,%.1f)\n",
                       nhits, g_ball.x, g_ball.y, g_ball.z);
                collision_log_count++;
            }
        }
        
        /* Fallback removed: ground plane at Y=radius breaks freefall levels
         * (Level2, Level3) where the ball must fall from START to track.
         * Ball falling through the world is caught by respawn trigger at
         * bounds_min.y - 500 instead. */
#if 0
        if (nhits == 0 && g_ball.y < g_ball.radius) {
            g_ball.y = g_ball.radius;
            if (g_ball.vy < 0) g_ball.vy = -g_ball.vy * 0.3f;
            if (fabsf(g_ball.vy) < 5.0f) g_ball.vy = 0.0f;
        }
#endif
    }
    
    /* Phase 6: Clamp max speed (after substeps) */
    float speed = sqrtf(g_ball.vx * g_ball.vx + g_ball.vy * g_ball.vy + g_ball.vz * g_ball.vz);
    if (speed > g_ball.max_speed) {
        float scale = g_ball.max_speed / speed;
        g_ball.vx *= scale; g_ball.vy *= scale; g_ball.vz *= scale;
    }
    
    /* Failsafe: if ball falls way below the level, respawn at checkpoint */
    if (g_level && g_ball.y < g_level->bounds_min.y - 500.0f) {
        RespawnBall();
        printf("[Physics] Ball fell below level — respawned\n");
    }
    
    /* Debug: log ball position periodically */
    static int physics_log_count = 0;
    physics_log_count++;
    if (physics_log_count <= 3 || (physics_log_count % 300 == 0)) {
        printf("[Physics] frame=%d pos=(%.1f,%.1f,%.1f) vel=(%.1f,%.1f,%.1f) substeps=%d\n",
               physics_log_count, g_ball.x, g_ball.y, g_ball.z,
               g_ball.vx, g_ball.vy, g_ball.vz, substeps);
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
     * Arena mode: camera targets blend between CAMERALOOKAT center + ball
     * Race mode: camera targets ball directly
     * orbit_dir = (cos(angle), tilt, sin(angle)) normalized, then:
     * eye = target + normalize(orbit_dir) * distance
     * Smooth lerp: target moves toward desired position each frame */
    float cos_a = cosf(g_camera.orbit_angle);
    float sin_a = sinf(g_camera.orbit_angle);

    /* Camera target for arena: blend between arena center (to see the whole bowl)
     * and ball position (to keep ball visible).
     * For levels without CAMERALOOKAT, just track the ball. */
    float desired_x, desired_y, desired_z;
    if (g_has_camlookat) {
        /* Arena: look toward the bowl center but weighted toward ball
         * so the ball stays on-screen. This shows the arena geometry. */
        desired_x = g_camlookat_x * 0.4f + g_ball.x * 0.6f;
        desired_y = g_camlookat_y * 0.3f + g_ball.y * 0.7f;
        desired_z = g_camlookat_z * 0.4f + g_ball.z * 0.6f;
    } else {
        desired_x = g_ball.x;
        desired_y = g_ball.y;
        desired_z = g_ball.z;
    }

    /* Smooth lerp toward desired target (matches Scene_SetCamera spring) */
    float lerp_speed = 0.08f;
    g_camera.tx += (desired_x - g_camera.tx) * lerp_speed;
    g_camera.ty += (desired_y - g_camera.ty) * lerp_speed;
    g_camera.tz += (desired_z - g_camera.tz) * lerp_speed;

    /* Build orbit direction vector (from focus toward camera).
     * Original: eye = target + dir * dist (always + sign).
     * Camera looks toward -dir. */
    float dir_x = cos_a;
    float dir_y = g_camera.orbit_tilt;
    float dir_z = sin_a;
    /* Normalize it */
    float dlen = sqrtf(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z);
    if (dlen > 0) { dir_x /= dlen; dir_y /= dlen; dir_z /= dlen; }

    /* Race track: fixed distance 400 for consistent isometric feel.
     * Arena: 500-600 with camlookat for bowl visibility. */
    if (g_has_camlookat) {
        g_camera.orbit_dist = 500.0f;
    } else {
        g_camera.orbit_dist = 400.0f;
    }

    /* Always use + sign: eye = target + dir * dist, matching original Scene_SetCamera */
    float eyex = g_camera.tx + dir_x * g_camera.orbit_dist;
    float eyey = g_camera.ty + dir_y * g_camera.orbit_dist;
    float eyez = g_camera.tz + dir_z * g_camera.orbit_dist;
    float atx = g_camera.tx, aty = g_camera.ty, atz = g_camera.tz;
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
    /* Ball rendering: translucent sphere matching the original's look.
     * Original Ball_Render (0x402de0) uses Sphere.MESH with translucent material.
     * The ball appears as a clear/translucent shell with a hamster inside.
     * 
     * We render it with:
     * - Alpha blending enabled (translucent)
     * - Grey-blue tinted material (matching the ball texture's mean color)
     * - Z-write disabled for transparent objects
     * - Two-pass: back faces (darker interior) then front faces (lighter shell)
     */
    
    D3DMATRIX world;
    ZeroMemory(&world, sizeof(world));
    world._11 = g_ball.radius; world._22 = g_ball.radius; world._33 = g_ball.radius; world._44 = 1.0f;
    world._41 = g_ball.x; world._42 = g_ball.y; world._43 = g_ball.z;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &world);
    
    /* Enable alpha blending for translucent ball */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, FALSE);
    
    /* Ball material: translucent grey-blue (matches original ball texture).
     * Original ball texture mean RGBA: ~(210, 210, 210, 103) */
    D3DMATERIAL8 mat;
    ZeroMemory(&mat, sizeof(mat));
    
    /* --- Pass 1: Back faces (inside of ball, slightly darker) --- */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_CW); /* Render back faces */
    mat.Diffuse.r = 0.55f; mat.Diffuse.g = 0.58f; mat.Diffuse.b = 0.65f; mat.Diffuse.a = 0.35f;
    mat.Ambient.r = 0.30f; mat.Ambient.g = 0.30f; mat.Ambient.b = 0.35f; mat.Ambient.a = 0.35f;
    mat.Specular.r = 0.4f; mat.Specular.g = 0.4f; mat.Specular.b = 0.4f; mat.Specular.a = 1.0f;
    mat.Power = 40.0f;
    g_device->lpVtbl->SetMaterial(g_device, &mat);
    g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_NORMAL);
    DrawSpherePrimitive();
    
    /* --- Pass 2: Front faces (outside shell, lighter, see-through) --- */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_CCW); /* Render front faces */
    mat.Diffuse.r = 0.75f; mat.Diffuse.g = 0.78f; mat.Diffuse.b = 0.85f; mat.Diffuse.a = 0.45f;
    mat.Ambient.r = 0.45f; mat.Ambient.g = 0.47f; mat.Ambient.b = 0.52f; mat.Ambient.a = 0.45f;
    mat.Specular.r = 0.9f; mat.Specular.g = 0.9f; mat.Specular.b = 0.95f; mat.Specular.a = 1.0f;
    mat.Power = 64.0f;
    g_device->lpVtbl->SetMaterial(g_device, &mat);
    DrawSpherePrimitive();
    
    /* Restore render state */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_NONE);
}

/* Draw a circular shadow on the ground below the ball.
 * Original Ball_Render (0x402de0) calls Sprite_RenderQuad for shadow.
 * We find ground Y by checking ball position against level geometry. */
static void RenderBallShadow(void) {
    /* Find ground Y below ball — use collision result from physics if available,
     * otherwise fall back to simple downward probe. */ 
    float ground_y = g_ball.y - g_ball.radius - 1.0f;  /* default: just below ball */
    
    /* Quick downward probe: test sphere at 3 heights below the ball */
    if (g_level && g_level->index_count > 0) {
        CollisionResult shadow_hits[MAX_SHADOW_HITS];
        /* Check at ball radius (touching ground), then further down */
        int nhits = TestSphereVsLevel(g_ball.x, g_ball.y - g_ball.radius, g_ball.z,
                                      g_ball.radius * 0.3f, shadow_hits, MAX_SHADOW_HITS);
        if (nhits > 0) {
            ground_y = shadow_hits[0].cy;  /* Closest point on triangle */
        } else {
            /* Probe further down */
            for (float dy = 50.0f; dy <= 300.0f; dy += 50.0f) {
                nhits = TestSphereVsLevel(g_ball.x, g_ball.y - dy, g_ball.z,
                                          dy * 0.5f, shadow_hits, MAX_SHADOW_HITS);
                if (nhits > 0) {
                    ground_y = shadow_hits[0].cy;
                    break;
                }
            }
        }
    }
    
    float dist_to_ground = g_ball.y - ground_y;
    float shadow_alpha = 0.3f;
    /* Shadow fades with distance from ground */
    if (dist_to_ground > 300.0f) return;  /* Too far, skip shadow entirely */
    else if (dist_to_ground > 150.0f) shadow_alpha *= (300.0f - dist_to_ground) / 150.0f;
    if (shadow_alpha <= 0.01f) return;
    
    float shadow_size = g_ball.radius * 1.3f;
    
    /* Draw shadow as a flat disc at ground_y */
    D3DMATRIX world;
    ZeroMemory(&world, sizeof(world));
    world._11 = shadow_size;  world._22 = 1.0f;  world._33 = shadow_size;  world._44 = 1.0f;
    world._41 = g_ball.x;  world._42 = ground_y + 0.5f;  world._43 = g_ball.z;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &world);
    
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_NONE);
    
    D3DMATERIAL8 mat;
    ZeroMemory(&mat, sizeof(mat));
    mat.Diffuse.r = 0; mat.Diffuse.g = 0; mat.Diffuse.b = 0; mat.Diffuse.a = shadow_alpha;
    mat.Ambient.r = 0; mat.Ambient.g = 0; mat.Ambient.b = 0; mat.Ambient.a = shadow_alpha;
    g_device->lpVtbl->SetMaterial(g_device, &mat);
    g_device->lpVtbl->SetTexture(g_device, 0, NULL);
    
    /* Flat circle (8-sided polygon via triangle fan) */
    struct { float x, y, z; DWORD diff; } verts[10];
    DWORD color = D3DCOLOR_RGBA(0, 0, 0, (BYTE)(shadow_alpha * 255));
    verts[0].x = 0; verts[0].y = 0; verts[0].z = 0; verts[0].diff = color;
    for (int i = 0; i <= 8; i++) {
        float a = 2.0f * 3.14159265f * i / 8;
        verts[i + 1].x = cosf(a);
        verts[i + 1].y = 0;
        verts[i + 1].z = sinf(a);
        verts[i + 1].diff = color;
    }
    g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE);
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLEFAN, 8, verts, sizeof(verts[0]));
    
    /* Restore */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZWRITEENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
}

/* Draw the ball sphere using vertex buffer (cached) */
static void DrawSpherePrimitive(void) {
    static BOOL generated = FALSE;
    static struct { float x, y, z; float nx, ny, nz; } sphere_verts[16*12*6]; /* ~1152 verts */
    static int vert_count = 0;
    
    if (!generated) {
        const int slices = 16, stacks = 12;
        vert_count = 0;
        
        for (int i = 0; i < stacks; i++) {
            float phi1 = 3.14159265f * i / stacks - 3.14159265f / 2.0f;
            float phi2 = 3.14159265f * (i + 1) / stacks - 3.14159265f / 2.0f;
            
            for (int j = 0; j < slices; j++) {
                float theta1 = 2.0f * 3.14159265f * j / slices;
                float theta2 = 2.0f * 3.14159265f * (j + 1) / slices;
                
                float cp1 = cosf(phi1), sp1 = sinf(phi1);
                float cp2 = cosf(phi2), sp2 = sinf(phi2);
                float ct1 = cosf(theta1), st1 = sinf(theta1);
                float ct2 = cosf(theta2), st2 = sinf(theta2);
                
                /* Tri 1 */
                sphere_verts[vert_count].x = cp1*ct1; sphere_verts[vert_count].y = sp1; sphere_verts[vert_count].z = cp1*st1;
                sphere_verts[vert_count].nx = cp1*ct1; sphere_verts[vert_count].ny = sp1; sphere_verts[vert_count].nz = cp1*st1;
                vert_count++;
                sphere_verts[vert_count].x = cp1*ct2; sphere_verts[vert_count].y = sp1; sphere_verts[vert_count].z = cp1*st2;
                sphere_verts[vert_count].nx = cp1*ct2; sphere_verts[vert_count].ny = sp1; sphere_verts[vert_count].nz = cp1*st2;
                vert_count++;
                sphere_verts[vert_count].x = cp2*ct1; sphere_verts[vert_count].y = sp2; sphere_verts[vert_count].z = cp2*st1;
                sphere_verts[vert_count].nx = cp2*ct1; sphere_verts[vert_count].ny = sp2; sphere_verts[vert_count].nz = cp2*st1;
                vert_count++;
                /* Tri 2 */
                sphere_verts[vert_count].x = cp1*ct2; sphere_verts[vert_count].y = sp1; sphere_verts[vert_count].z = cp1*st2;
                sphere_verts[vert_count].nx = cp1*ct2; sphere_verts[vert_count].ny = sp1; sphere_verts[vert_count].nz = cp1*st2;
                vert_count++;
                sphere_verts[vert_count].x = cp2*ct2; sphere_verts[vert_count].y = sp2; sphere_verts[vert_count].z = cp2*st2;
                sphere_verts[vert_count].nx = cp2*ct2; sphere_verts[vert_count].ny = sp2; sphere_verts[vert_count].nz = cp2*st2;
                vert_count++;
                sphere_verts[vert_count].x = cp2*ct1; sphere_verts[vert_count].y = sp2; sphere_verts[vert_count].z = cp2*st1;
                sphere_verts[vert_count].nx = cp2*ct1; sphere_verts[vert_count].ny = sp2; sphere_verts[vert_count].nz = cp2*st1;
                vert_count++;
            }
        }
        generated = TRUE;
    }
    
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLELIST, 
        vert_count / 3, sphere_verts, sizeof(sphere_verts[0]));
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
    
    /* Software lighting: D3D8 hardware lighting doesn't work on Wine/llvmpipe.
     * We compute per-vertex colors from normals + materials.
     * For checker textures: we create programmatic checker textures and use
     * D3DTOP_MODULATE to combine per-pixel texture with per-vertex lighting.
     * On llvmpipe, BMP-loaded textures don't render, but programmatic ones do. */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    
    /* Create checker textures on first call */
    static IDirect3DTexture8 *chk_pink = NULL;
    static IDirect3DTexture8 *chk_blue = NULL;
    if (!chk_pink) {
        /* PinkChecker: white + pale pink */
        int sz = 128;
        g_device->lpVtbl->CreateTexture(g_device, sz, sz, 1, 0, D3DFMT_A8R8G8B8,
                                         D3DPOOL_MANAGED, &chk_pink);
        D3DLOCKED_RECT lr;
        if (chk_pink && chk_pink->lpVtbl->LockRect(chk_pink, 0, &lr, NULL, 0) == D3D_OK) {
            for (int y = 0; y < sz; y++) {
                DWORD *row = (DWORD *)((BYTE *)lr.pBits + y * lr.Pitch);
                for (int x = 0; x < sz; x++) {
                    int cell = ((x / (sz/8)) + (y / (sz/8))) & 1;
                    if (cell) row[x] = 0xFFE0D8EA; /* Pale pink ARGB */
                    else      row[x] = 0xFFFFF8FF; /* Near-white ARGB */
                }
            }
            chk_pink->lpVtbl->UnlockRect(chk_pink, 0);
        }
        /* BlueChecker: white + light blue */
        g_device->lpVtbl->CreateTexture(g_device, sz, sz, 1, 0, D3DFMT_A8R8G8B8,
                                         D3DPOOL_MANAGED, &chk_blue);
        if (chk_blue && chk_blue->lpVtbl->LockRect(chk_blue, 0, &lr, NULL, 0) == D3D_OK) {
            for (int y = 0; y < sz; y++) {
                DWORD *row = (DWORD *)((BYTE *)lr.pBits + y * lr.Pitch);
                for (int x = 0; x < sz; x++) {
                    int cell = ((x / (sz/8)) + (y / (sz/8))) & 1;
                    if (cell) row[x] = 0xFFD0E0FF; /* Light blue ARGB */
                    else      row[x] = 0xFFFFF8FF; /* Near-white ARGB */
                }
            }
            chk_blue->lpVtbl->UnlockRect(chk_blue, 0);
        }
    }
    
    /* Vertex format: XYZ + DIFFUSE color + TEX1 = 28 bytes/vertex */
    typedef struct { float x, y, z; DWORD color; float u, v; } LitVertex;
    DWORD fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
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
    
    /* Per-geom material colors from MESHWORLD are correct for race tracks.
     * Each geom stores its own ambient/diffuse/specular/emissive from the file.
     * Previously tried using Section 1 PLATFORM material but that's only right
     * for arena levels — race tracks use pink/magenta per-geom colors. */
    static int first_frame = 0;
    
    /* Render each geom with its own D3D material + optional texture */
    int total_tris = 0;
    
    for (int gi = 0; gi < g_level->geom_count; gi++) {
        mw_geom_t *geom = &g_level->geoms[gi];
        
        /* Skip invisible geoms: N: (notification), E: (edge limit) per Level_LoadMeshes */
        if (geom->no_render) continue;
        
        /* Set D3D material from geom's material properties.
         * Material ambient scaled to 0.5x diffuse to allow directional light contrast. */
        D3DMATERIAL8 mat;
        ZeroMemory(&mat, sizeof(mat));
        mat.Ambient.r  = geom->diffuse[0] * 0.5f;  mat.Ambient.g  = geom->diffuse[1] * 0.5f;
        mat.Ambient.b  = geom->diffuse[2] * 0.5f;  mat.Ambient.a  = 1.0f;
        mat.Diffuse.r  = geom->diffuse[0];  mat.Diffuse.g  = geom->diffuse[1];
        mat.Diffuse.b  = geom->diffuse[2];  mat.Diffuse.a  = geom->diffuse[3];
        mat.Specular.r = geom->specular[0]; mat.Specular.g = geom->specular[1];
        mat.Specular.b = geom->specular[2]; mat.Specular.a = geom->specular[3];
        mat.Emissive.r = geom->emissive[0]; mat.Emissive.g = geom->emissive[1];
        mat.Emissive.b = geom->emissive[2]; mat.Emissive.a = geom->emissive[3];
        mat.Power = geom->power;
        g_device->lpVtbl->SetMaterial(g_device, &mat);
        
        /* Bind texture or clear it. For checker-textured geoms, use programmatic
         * checker textures + MODULATE (per-pixel texture × per-vertex lighting).
         * For non-checker geoms, use SELECTARG1 (vertex color only). */
        texture_t *tex = NULL;
        IDirect3DTexture8 *checker_tex = NULL;
        if (geom->has_texture && geom->texture[0]) {
            tex = texture_load(geom->texture);
            /* Select programmatic checker texture based on type */
            if (strstr(geom->texture, "Pink") || strstr(geom->texture, "pink"))
                checker_tex = chk_pink;
            else if (strstr(geom->texture, "Checker") || strstr(geom->texture, "checker"))
                checker_tex = chk_blue;
        }
        if (checker_tex) {
            /* Bind programmatic checker + MODULATE for per-pixel×per-vertex */
            g_device->lpVtbl->SetTexture(g_device, 0, (IDirect3DBaseTexture8 *)checker_tex);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
        } else {
            /* No checker: use vertex color directly */
            texture_bind(tex, 0);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        
        /* Software lighting — target-matched colors.
         * Strategy: Set lit color = bright white-blue (matching original's bright highlights).
         * Set shadow color = deep royal blue (matching original's shadow side).
         * The lit_factor blends between them based on normal × light direction.
         * This is the INVERSE of standard material×lighting: we define output colors directly.
         * Wall lit: ~(172,222,254)  Wall shadow: ~(62,94,138)
         * Floor lit: ~(245,245,255)  Floor shadow: ~(215,215,255) */
        float mat_r = geom->diffuse[0], mat_g = geom->diffuse[1], mat_b = geom->diffuse[2]; /* unused */
        float lit_r, lit_g, lit_b;     /* Fully lit target color */
        float shadow_r, shadow_g, shadow_b;  /* Fully shadowed target color */
        if (tex) {
            /* Floor (textured): near-white lit, pale lavender-blue shadow.
             * Original: bright=(245,245,255), shadow=(180,200,255). Very bright. */
            lit_r = 0.96f; lit_g = 0.96f; lit_b = 1.0f;
            shadow_r = 0.70f; shadow_g = 0.78f; shadow_b = 1.0f;
        } else {
            /* Walls: bright icy-blue lit, deep navy shadow.
             * Original: lit=(172,222,254), shadow=(62,88,124). Wide dynamic range!
             * With fill light=0, fully shadowed faces hit shadow color directly.
             * Shadow color needs to encode the "dark but not black" original look. */
            lit_r = 0.72f; lit_g = 0.90f; lit_b = 1.0f;
            shadow_r = 0.24f; shadow_g = 0.35f; shadow_b = 0.49f;
        }
        /* For textured floor geoms, checker pattern comes from programmatic textures. */
        
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
                    for (int vi_idx = 0; vi_idx < 3; vi_idx++) {
                        int vi = (vi_idx == 0) ? v0 : (vi_idx == 1) ? v1 : v2;
                        mw_vertex_t *sv = &g_level->vertices[vi];
                        /* Directional shading with target-matched colors.
                         * Original uses 2 lights + ambient=0.25 which creates
                         * sharp transitions: lit faces are bright, shadow faces are deep blue.
                         * We simulate this with a steep non-linear falloff. */
                        float NdotL1 = 0.35f*sv->nx + 0.85f*sv->ny + 0.35f*sv->nz;
                        if (NdotL1 < 0) NdotL1 = 0;
                        /* Fill light: set to zero — original ambient=0.25 but the
                         * deep shadow color we defined already accounts for ambient.
                         * Any fill light lifts shadows too much. */
                        float NdotL2 = -0.3f*sv->nx + -0.6f*sv->ny + -0.3f*sv->nz;
                        if (NdotL2 < 0) NdotL2 = 0;
                        float raw_factor = 1.0f * NdotL1 + 0.0f * NdotL2;
                        if (raw_factor > 1.0f) raw_factor = 1.0f;
                        /* Aggressive gamma for original's punchy contrast.
                         * Original D3D has 2 directional lights + ambient=0.25.
                         * With ambient=0.25, even the darkest face gets lit*0.25.
                         * But shadow faces in original are VERY dark (RGB ~62,88,124).
                         * This means the ambient contribution is small and blue-tinted.
                         * Use pow(x, 0.35) for very steep falloff. */
                        float lit_factor = powf(raw_factor, 0.35f);
                        /* Select lit/shadow colors based on checker tile for floors */
                        float cur_lit_r = lit_r, cur_lit_g = lit_g, cur_lit_b = lit_b;
                        float cur_shad_r = shadow_r, cur_shad_g = shadow_g, cur_shad_b = shadow_b;
                        /* No per-vertex checker — checker pattern comes from texture */
                        /* Blend between shadow and lit based on NdotL.
                         * lit_factor=0 → shadow color, lit_factor=1 → fully lit color */
                        float fr = 255.0f * (cur_shad_r + (cur_lit_r - cur_shad_r) * lit_factor);
                        float fg = 255.0f * (cur_shad_g + (cur_lit_g - cur_shad_g) * lit_factor);
                        float fb = 255.0f * (cur_shad_b + (cur_lit_b - cur_shad_b) * lit_factor);
                        if (fr > 255) fr = 255; if (fg > 255) fg = 255; if (fb > 255) fb = 255;
                        if (fr < 0) fr = 0; if (fg < 0) fg = 0; if (fb < 0) fb = 0;
                        DWORD col = D3DCOLOR_RGBA((BYTE)fr, (BYTE)fg, (BYTE)fb, 255);
                        tri_buf[tri_out++] = (LitVertex){ sv->x, sv->y, sv->z, col, sv->u, sv->v };
                    }
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
    
    if (first_frame < 1) {
        printf("[Render] %d geoms, %d triangles, %d vertices (software lighting)\n",
               g_level->geom_count, total_tris, g_level->vertex_count);
        float vmin_y=1e30f, vmax_y=-1e30f;
        for (int vi = 0; vi < g_level->vertex_count; vi++) {
            mw_vertex_t *v = &g_level->vertices[vi];
            if (v->y < vmin_y) vmin_y = v->y;
            if (v->y > vmax_y) vmax_y = v->y;
        }
        printf("[Bounds] Y range: %.1f to %.1f (ball at %.1f)\n", vmin_y, vmax_y, g_ball.y);
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
    
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);
    g_device->lpVtbl->SetTexture(g_device, 0, NULL);

    for (int i = 0; i < g_level->object_count; i++) {
        mw_object_t *obj = &g_level->objects[i];

        if (obj->type == MW_OBJ_START) {
            /* Green start pad — matching original's bright green circle pad */
            float pad_y = obj->position.y + 0.5f;
            float pad_r = 40.0f;
            DWORD pad_color = D3DCOLOR_RGBA(80, 200, 60, 255);
            DWORD border_color = D3DCOLOR_RGBA(0, 0, 0, 255);
            struct { float x, y, z; DWORD diff; } pad_verts[34];
            pad_verts[0].x = 0; pad_verts[0].y = 0; pad_verts[0].z = 0; pad_verts[0].diff = pad_color;
            for (int j = 0; j <= 32; j++) {
                float a = 2.0f * 3.14159265f * j / 32;
                float px = cosf(a) * pad_r;
                float pz = sinf(a) * pad_r;
                pad_verts[j + 1].x = px; pad_verts[j + 1].y = 0; pad_verts[j + 1].z = pz;
                pad_verts[j + 1].diff = (j == 0 || j == 32) ? border_color : pad_color;
            }
            D3DMATRIX pad_world;
            ZeroMemory(&pad_world, sizeof(pad_world));
            pad_world._11 = 1.0f; pad_world._22 = 1.0f; pad_world._33 = 1.0f; pad_world._44 = 1.0f;
            pad_world._41 = obj->position.x; pad_world._42 = pad_y; pad_world._43 = obj->position.z;
            g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &pad_world);
            g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE);
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLEFAN, 32, pad_verts, sizeof(pad_verts[0]));

        } else if (obj->type == MW_OBJ_FLAG) {
            /* Yellow checkpoint pole */
            D3DMATRIX flag_world;
            ZeroMemory(&flag_world, sizeof(flag_world));
            flag_world._11 = 1.0f; flag_world._22 = 1.0f; flag_world._33 = 1.0f; flag_world._44 = 1.0f;
            flag_world._41 = obj->position.x; flag_world._42 = obj->position.y; flag_world._43 = obj->position.z;
            g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &flag_world);
            struct { float x, y, z; DWORD diff; } pole[2] = {
                {0, 0, 0, D3DCOLOR_RGBA(255, 255, 0, 255)},
                {0, 80, 0, D3DCOLOR_RGBA(255, 255, 0, 255)}
            };
            g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE);
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_LINESTRIP, 1, pole, sizeof(pole[0]));

        } else if (obj->type == MW_OBJ_SAFESPOT) {
            /* Checkpoint marker — translucent yellow diamond */
            float cy = obj->position.y + 40.0f;
            float size = 25.0f;
            DWORD cp_col = D3DCOLOR_RGBA(255, 220, 0, 200);   /* Yellow */
            DWORD cp_border = D3DCOLOR_RGBA(180, 150, 0, 255); /* Darker yellow */
            if (g_checkpoint_passed[i]) {
                cp_col = D3DCOLOR_RGBA(80, 220, 80, 200);       /* Green = passed */
                cp_border = D3DCOLOR_RGBA(40, 150, 40, 255);
            }
            struct { float x, y, z; DWORD diff; } cp_verts[6];
            cp_verts[0].x = 0; cp_verts[0].y = size; cp_verts[0].z = 0; cp_verts[0].diff = cp_col;
            cp_verts[1].x = size; cp_verts[1].y = 0; cp_verts[1].z = 0; cp_verts[1].diff = cp_border;
            cp_verts[2].x = 0; cp_verts[2].y = -size; cp_verts[2].z = 0; cp_verts[2].diff = cp_col;
            cp_verts[3].x = -size; cp_verts[3].y = 0; cp_verts[3].z = 0; cp_verts[3].diff = cp_border;
            cp_verts[4].x = 0; cp_verts[4].y = 0; cp_verts[4].z = size; cp_verts[4].diff = cp_col;
            cp_verts[5].x = 0; cp_verts[5].y = 0; cp_verts[5].z = -size; cp_verts[5].diff = cp_border;
            D3DMATRIX cp_world;
            ZeroMemory(&cp_world, sizeof(cp_world));
            cp_world._11 = 1.0f; cp_world._22 = 1.0f; cp_world._33 = 1.0f; cp_world._44 = 1.0f;
            cp_world._41 = obj->position.x; cp_world._42 = cy; cp_world._43 = obj->position.z;
            g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &cp_world);
            g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE);
            /* Simple diamond: 4 tris from center to edges */
            g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, TRUE);
            g_device->lpVtbl->SetRenderState(g_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            g_device->lpVtbl->SetRenderState(g_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLEFAN, 4, cp_verts, sizeof(cp_verts[0]));
            g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, FALSE);
        }
    }

    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
}

static void RenderHUD(void) {
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, FALSE);
    g_device->lpVtbl->SetTexture(g_device, 0, NULL);

    /* Switch to screen-space coordinates (orthographic) */
    D3DMATRIX ident, ortho;
    ZeroMemory(&ident, sizeof(ident)); ident._11 = 1; ident._22 = 1; ident._33 = 1; ident._44 = 1;
    float w = (float)g_width, h = (float)g_height;
    ZeroMemory(&ortho, sizeof(ortho));
    ortho._11 = 2.0f / w;  ortho._22 = 2.0f / h;
    ortho._33 = 1.0f / (0.0f - 1.0f);
    ortho._41 = -1.0f;     ortho._42 = 1.0f;  ortho._43 = 0.0f;  ortho._44 = 1.0f;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &ident);
    g_device->lpVtbl->SetTransform(g_device, D3DTS_VIEW, &ident);
    g_device->lpVtbl->SetTransform(g_device, D3DTS_PROJECTION, &ortho);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    struct { float x, y, z; DWORD diff; } v[20];

    /* === Timer / Countdown (top center) === */
    char time_str[32];
    if (g_race_state == RACE_COUNTDOWN) {
        if (g_countdown_timer > 2.0f)     snprintf(time_str, sizeof(time_str), "3");
        else if (g_countdown_timer > 1.0f) snprintf(time_str, sizeof(time_str), "2");
        else if (g_countdown_timer > 0.0f) snprintf(time_str, sizeof(time_str), "1");
        else snprintf(time_str, sizeof(time_str), "GO!");
    } else if (g_race_state == RACE_FINISHED) {
        snprintf(time_str, sizeof(time_str), "FINISH! %.2f", g_race_time);
    } else {
        snprintf(time_str, sizeof(time_str), "%.2f", g_race_time);
    }

    /* Timer background pill */
    float pill_cx = w * 0.5f;
    float pill_cy = 28.0f;
    float pill_hw = (g_race_state == RACE_COUNTDOWN && g_countdown_timer > 0.0f) ? 50.0f : 90.0f;
    float pill_hh = 18.0f;
    DWORD pill_bg  = D3DCOLOR_RGBA(15, 25, 60, 200);
    DWORD pill_bdr = D3DCOLOR_RGBA(60, 90, 180, 220);
    /* Background quad (2 tris) */
    v[0].x=(pill_cx-pill_hw)*2/w-1; v[0].y=1-(pill_cy-pill_hh)*2/h; v[0].z=0.99f; v[0].diff=pill_bg;
    v[1].x=(pill_cx+pill_hw)*2/w-1; v[1].y=1-(pill_cy-pill_hh)*2/h; v[1].z=0.99f; v[1].diff=pill_bg;
    v[2].x=(pill_cx-pill_hw)*2/w-1; v[2].y=1-(pill_cy+pill_hh)*2/h; v[2].z=0.99f; v[2].diff=pill_bg;
    v[3].x=(pill_cx+pill_hw)*2/w-1; v[3].y=1-(pill_cy+pill_hh)*2/h; v[3].z=0.99f; v[3].diff=pill_bg;
    g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE);
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));
    /* Border line */
    struct { float x, y, z; DWORD diff; } bdr[5];
    for (int i = 0; i < 4; i++) { bdr[i].x = v[i].x; bdr[i].y = v[i].y; bdr[i].z = 0.99f; bdr[i].diff = pill_bdr; }
    bdr[4] = bdr[0];
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_LINESTRIP, 4, bdr, sizeof(bdr[0]));

    /* === Speed Bar (bottom center) === */
    float spd_h = w * 0.5f;
    float spd_v = h - 28.0f;
    float spd_hw = 140.0f;
    float spd_hh = 8.0f;
    /* Background */
    DWORD spd_bg = D3DCOLOR_RGBA(20, 20, 30, 180);
    v[0].x=(spd_h-spd_hw)*2/w-1; v[0].y=1-(spd_v-spd_hh)*2/h; v[0].z=0.99f; v[0].diff=spd_bg;
    v[1].x=(spd_h+spd_hw)*2/w-1; v[1].y=1-(spd_v-spd_hh)*2/h; v[1].z=0.99f; v[1].diff=spd_bg;
    v[2].x=(spd_h-spd_hw)*2/w-1; v[2].y=1-(spd_v+spd_hh)*2/h; v[2].z=0.99f; v[2].diff=spd_bg;
    v[3].x=(spd_h+spd_hw)*2/w-1; v[3].y=1-(spd_v+spd_hh)*2/h; v[3].z=0.99f; v[3].diff=spd_bg;
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));
    /* Fill */
    float speed = sqrtf(g_ball.vx*g_ball.vx + g_ball.vy*g_ball.vy + g_ball.vz*g_ball.vz);
    float spd_ratio = speed / g_ball.max_speed;
    if (spd_ratio > 1.0f) spd_ratio = 1.0f;
    float fill_w = spd_hw * 2.0f * spd_ratio;
    DWORD spd_col = D3DCOLOR_RGBA(80, 220, 80, 200);
    if (spd_ratio > 0.5f) spd_col = D3DCOLOR_RGBA(220, 220, 60, 200);
    if (spd_ratio > 0.8f) spd_col = D3DCOLOR_RGBA(220, 60, 60, 200);
    if (fill_w > 1.0f) {
        v[0].x=(spd_h-spd_hw)*2/w-1; v[0].y=1-(spd_v-spd_hh+2)*2/h; v[0].z=0.99f; v[0].diff=spd_col;
        v[1].x=(spd_h-spd_hw+fill_w)*2/w-1; v[1].y=1-(spd_v-spd_hh+2)*2/h; v[1].z=0.99f; v[1].diff=spd_col;
        v[2].x=(spd_h-spd_hw)*2/w-1; v[2].y=1-(spd_v+spd_hh-2)*2/h; v[2].z=0.99f; v[2].diff=spd_col;
        v[3].x=(spd_h-spd_hw+fill_w)*2/w-1; v[3].y=1-(spd_v+spd_hh-2)*2/h; v[3].z=0.99f; v[3].diff=spd_col;
        g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));
    }
    /* Speed bar border */
    struct { float x, y, z; DWORD diff; } sbd[5];
    sbd[0].x=(spd_h-spd_hw)*2/w-1; sbd[0].y=1-(spd_v-spd_hh)*2/h; sbd[0].z=0.99f; sbd[0].diff=D3DCOLOR_RGBA(100,100,140,200);
    sbd[1].x=(spd_h+spd_hw)*2/w-1; sbd[1].y=1-(spd_v-spd_hh)*2/h; sbd[1].z=0.99f; sbd[1].diff=D3DCOLOR_RGBA(100,100,140,200);
    sbd[2].x=(spd_h+spd_hw)*2/w-1; sbd[2].y=1-(spd_v+spd_hh)*2/h; sbd[2].z=0.99f; sbd[2].diff=D3DCOLOR_RGBA(100,100,140,200);
    sbd[3].x=(spd_h-spd_hw)*2/w-1; sbd[3].y=1-(spd_v+spd_hh)*2/h; sbd[3].z=0.99f; sbd[3].diff=D3DCOLOR_RGBA(100,100,140,200);
    sbd[4] = sbd[0];
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_LINESTRIP, 4, sbd, sizeof(sbd[0]));

    /* === Checkpoint Counter (top right) === */
    int total_cp = 0, passed_cp = 0;
    if (g_level) {
        for (int i = 0; i < g_level->object_count; i++) {
            if (g_level->objects[i].type == MW_OBJ_SAFESPOT) {
                total_cp++;
                if (g_checkpoint_passed[i]) passed_cp++;
            }
        }
    }
    if (total_cp > 0) {
        float cp_cx = w - 80.0f;
        float cp_cy = 28.0f;
        float cp_hw = 60.0f, cp_hh = 14.0f;
        DWORD cp_bg = D3DCOLOR_RGBA(15, 25, 60, 200);
        v[0].x=(cp_cx-cp_hw)*2/w-1; v[0].y=1-(cp_cy-cp_hh)*2/h; v[0].z=0.99f; v[0].diff=cp_bg;
        v[1].x=(cp_cx+cp_hw)*2/w-1; v[1].y=1-(cp_cy-cp_hh)*2/h; v[1].z=0.99f; v[1].diff=cp_bg;
        v[2].x=(cp_cx-cp_hw)*2/w-1; v[2].y=1-(cp_cy+cp_hh)*2/h; v[2].z=0.99f; v[2].diff=cp_bg;
        v[3].x=(cp_cx+cp_hw)*2/w-1; v[3].y=1-(cp_cy+cp_hh)*2/h; v[3].z=0.99f; v[3].diff=cp_bg;
        g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));
        struct { float x, y, z; DWORD diff; } cpb[5];
        for (int i = 0; i < 4; i++) { cpb[i].x = v[i].x; cpb[i].y = v[i].y; cpb[i].z = 0.99f; cpb[i].diff = pill_bdr; }
        cpb[4] = cpb[0];
        g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_LINESTRIP, 4, cpb, sizeof(cpb[0]));
    }

    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ALPHABLENDENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);

    /* Window title with all info */
    char fpsText[256];
    int total_cp2 = 0, passed_cp2 = 0;
    if (g_level) {
        for (int i = 0; i < g_level->object_count; i++) {
            if (g_level->objects[i].type == MW_OBJ_SAFESPOT) {
                total_cp2++;
                if (g_checkpoint_passed[i]) passed_cp2++;
            }
        }
    }
    snprintf(fpsText, sizeof(fpsText), "FPS:%d | %s | %s%s | CP:%d/%d | Spd:%.0f | []=cycle R=respawn",
             g_current_fps, g_level_name, time_str,
             (g_race_state == RACE_FINISHED && g_race_time == g_best_time) ? " (BEST!)" : "",
             passed_cp2, total_cp2, speed);
    SetWindowTextA(g_hwnd, fpsText);
}

/* ===== Sky rendering ===== */
static void RenderSky(void) {
    if (!g_sky_tex) return;

    /* Render a large sky dome centered on camera with clouds texture.
     * The camera is inside the sphere, so render inside-facing triangles. */
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_LIGHTING, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, FALSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_NONE);
    g_device->lpVtbl->SetTexture(g_device, 0, (IDirect3DBaseTexture8 *)g_sky_tex->d3d_tex);
    g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    struct _skyvtx { float x, y, z; DWORD diff; float u, v; };
    struct _skyvtx v[4];
    DWORD col = D3DCOLOR_RGBA(255,255,255,255);
    float s = 5000.0f; /* Large sky plane */

    /* Sky plane filling the far view (no depth testing means it covers the whole screen) */
    v[0].x = -s; v[0].y =  s; v[0].z =  s; v[0].diff = col; v[0].u = 0.0f; v[0].v = 0.0f;
    v[1].x =  s; v[1].y =  s; v[1].z =  s; v[1].diff = col; v[1].u = 1.0f; v[1].v = 0.0f;
    v[2].x = -s; v[2].y = -s; v[2].z =  s; v[2].diff = col; v[2].u = 0.0f; v[2].v = 1.0f;
    v[3].x =  s; v[3].y = -s; v[3].z =  s; v[3].diff = col; v[3].u = 1.0f; v[3].v = 1.0f;

    D3DMATRIX sky_world;
    ZeroMemory(&sky_world, sizeof(sky_world));
    sky_world._11 = 1.0f; sky_world._22 = 1.0f; sky_world._33 = 1.0f; sky_world._44 = 1.0f;
    sky_world._41 = g_camera.x; sky_world._42 = g_camera.y; sky_world._43 = g_camera.z;
    g_device->lpVtbl->SetTransform(g_device, D3DTS_WORLD, &sky_world);
    g_device->lpVtbl->SetVertexShader(g_device, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    g_device->lpVtbl->DrawPrimitiveUP(g_device, D3DPT_TRIANGLESTRIP, 2, v, sizeof(v[0]));

    g_device->lpVtbl->SetTexture(g_device, 0, NULL);
    g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
    g_device->lpVtbl->SetTextureStageState(g_device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_ZENABLE, TRUE);
    g_device->lpVtbl->SetRenderState(g_device, D3DRS_CULLMODE, D3DCULL_CCW);
}

static void Render(void) {
    if (!g_device) return;

    /* Clear with level background color (Section 4) or default */
    /* Section 4 bg_color is often garbage in MESHWORLD — hardcode per-level or use dark blue default */
    DWORD clear_color = D3DCOLOR_RGBA(85, 120, 215, 255);  /* Deep royal blue sky matching original */
    g_device->lpVtbl->Clear(g_device, 0, NULL, 
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        clear_color, 1.0f, 0);
    g_device->lpVtbl->BeginScene(g_device);

    SetMatrices();
    RenderSky();
    RenderLevelGeometry();
    RenderLevelObjects();
    RenderBallShadow();  /* Shadow first (behind ball, closer to ground) */
    RenderBall();
    RenderHUD();

    g_device->lpVtbl->EndScene(g_device);
    g_device->lpVtbl->Present(g_device, NULL, NULL, NULL, NULL);
    
    /* Auto-screenshot after a few frames for testing — disabled on Wine/llvmpipe
     * (CopyRects returns D3DERR_NOTAVAILABLE with SW vertex processing) */
    #if 0
    static int render_count = 0;
    render_count++;
    if (render_count == 5) {
        SaveScreenshot("tests/screenshots/reimpl_test31_textured.bmp");
        printf("[Render] Auto-screenshot saved\n");
    }
    #endif
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
            UpdateCountdown(dt);
            UpdateCheckpoints();
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
    sfx_shutdown();
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