#define _CRT_SECURE_NO_WARNINGS
#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

// ── Game Offsets ──────────────────────────────────────────────────
static constexpr DWORD GLOBAL_APP_PTR   = 0x5341E0;
static constexpr DWORD APP_GRAPHICS_OFF = 0x174;
static constexpr DWORD GFX_DEVICE_OFF   = 0x154;
static constexpr DWORD BALL_POS_X       = 0x164;
static constexpr DWORD BALL_POS_Y       = 0x168;
static constexpr DWORD BALL_POS_Z       = 0x16C;

// ── D3D8 Render States ───────────────────────────────────────────
#define D3DRS_FOGENABLE       28
#define D3DRS_FOGCOLOR        34
#define D3DRS_FOGTABLEMODE    35
#define D3DRS_FOGSTART        36
#define D3DRS_FOGEND          37
#define D3DRS_LIGHTING       137
#define D3DRS_AMBIENT        139
#define D3DRS_FOGVERTEXMODE  140

// ── D3D8 Vtable Indices (0-indexed from IUnknown) ────────────────
#define VT_SetMaterial     42
#define VT_SetLight        44
#define VT_LightEnable     46
#define VT_GetRenderState  51
#define VT_SetRenderState  50

// ── D3D8 Constants ────────────────────────────────────────────────
#define D3DLIGHT_POINT    1
#define D3DFOG_LINEAR    3

// ── D3D8 Structs ──────────────────────────────────────────────────
struct D3DLIGHT8 {
    DWORD Type;
    float Diffuse[4];
    float Specular[4];
    float Ambient[4];
    float Position[3];
    float Direction[3];
    float Range;
    float Falloff;
    float Attenuation0;
    float Attenuation1;
    float Attenuation2;
    float Theta;
    float Phi;
};

struct D3DMATERIAL8 {
    float Diffuse[4];
    float Ambient[4];
    float Specular[4];
    float Emissive[4];
    float Power;
};

// ── D3D Function Pointer Types (__stdcall for COM) ────────────────
typedef long (__stdcall *fn_SetRenderState)(void*, DWORD, DWORD);
typedef long (__stdcall *fn_GetRenderState)(void*, DWORD, DWORD*);
typedef long (__stdcall *fn_SetLight)(void*, DWORD, const void*);
typedef long (__stdcall *fn_LightEnable)(void*, DWORD, BOOL);
typedef long (__stdcall *fn_SetMaterial)(void*, const void*);

// ── Global State ──────────────────────────────────────────────────
static IModAPI* g_api = NULL;
static volatile bool g_enabled = false;
static volatile bool g_gameReady = false;
static DWORD g_frameCount = 0;

static bool g_savedStates = false;
static DWORD g_origFogEnable, g_origFogColor, g_origFogTableMode;
static DWORD g_origFogStart, g_origFogEnd, g_origFogVertexMode;
static DWORD g_origLighting, g_origAmbient;
static BOOL g_origLight0Enable = FALSE;

static D3DLIGHT8 g_light;
static D3DMATERIAL8 g_material;
static bool g_materialInit = false;

// ── D3D Helpers ───────────────────────────────────────────────────

static void* getD3DDevice() {
    DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
    if (!appPtr || appPtr < 0x10000) return NULL;
    if (IsBadReadPtr((void*)(appPtr + APP_GRAPHICS_OFF), 4)) return NULL;
    DWORD gfx = *(DWORD*)(appPtr + APP_GRAPHICS_OFF);
    if (!gfx || gfx < 0x10000) return NULL;
    if (IsBadReadPtr((void*)(gfx + GFX_DEVICE_OFF), 4)) return NULL;
    DWORD device = *(DWORD*)(gfx + GFX_DEVICE_OFF);
    if (!device || device < 0x10000) return NULL;
    if (IsBadReadPtr((void*)device, 4)) return NULL;
    return (void*)device;
}

static void d3d_SetRenderState(void* device, DWORD state, DWORD value) {
    void** vt = *(void***)device;
    ((fn_SetRenderState)vt[VT_SetRenderState])(device, state, value);
}

static DWORD d3d_GetRenderState(void* device, DWORD state) {
    void** vt = *(void***)device;
    DWORD val = 0;
    ((fn_GetRenderState)vt[VT_GetRenderState])(device, state, &val);
    return val;
}

static void d3d_SetLight(void* device, DWORD index, const D3DLIGHT8* light) {
    void** vt = *(void***)device;
    ((fn_SetLight)vt[VT_SetLight])(device, index, light);
}

static void d3d_LightEnable(void* device, DWORD index, BOOL enable) {
    void** vt = *(void***)device;
    ((fn_LightEnable)vt[VT_LightEnable])(device, index, enable);
}

static void d3d_SetMaterial(void* device, const D3DMATERIAL8* mat) {
    void** vt = *(void***)device;
    ((fn_SetMaterial)vt[VT_SetMaterial])(device, mat);
}

// ── Game Helpers ──────────────────────────────────────────────────

static bool getBallPos(void* ball, float* x, float* y, float* z) {
    if (!ball || IsBadReadPtr(ball, 0x200)) return false;
    *x = *(float*)((char*)ball + BALL_POS_X);
    *y = *(float*)((char*)ball + BALL_POS_Y);
    *z = *(float*)((char*)ball + BALL_POS_Z);
    return true;
}

static void initMaterial() {
    if (g_materialInit) return;
    memset(&g_material, 0, sizeof(g_material));
    g_material.Diffuse[0] = 1.0f;
    g_material.Diffuse[1] = 1.0f;
    g_material.Diffuse[2] = 1.0f;
    g_material.Diffuse[3] = 1.0f;
    g_material.Ambient[0] = 0.3f;
    g_material.Ambient[1] = 0.3f;
    g_material.Ambient[2] = 0.3f;
    g_material.Ambient[3] = 1.0f;
    g_material.Power = 0.0f;
    g_materialInit = true;
}

// ── Core: Apply Neon Lighting ────────────────────────────────────

static void saveOriginalStates(void* device) {
    if (g_savedStates) return;
    g_origFogEnable     = d3d_GetRenderState(device, D3DRS_FOGENABLE);
    g_origFogColor      = d3d_GetRenderState(device, D3DRS_FOGCOLOR);
    g_origFogTableMode  = d3d_GetRenderState(device, D3DRS_FOGTABLEMODE);
    g_origFogStart      = d3d_GetRenderState(device, D3DRS_FOGSTART);
    g_origFogEnd        = d3d_GetRenderState(device, D3DRS_FOGEND);
    g_origFogVertexMode = d3d_GetRenderState(device, D3DRS_FOGVERTEXMODE);
    g_origLighting      = d3d_GetRenderState(device, D3DRS_LIGHTING);
    g_origAmbient       = d3d_GetRenderState(device, D3DRS_AMBIENT);
    g_origLight0Enable  = FALSE;
    g_savedStates = true;
}

static void restoreOriginalStates(void* device) {
    if (!g_savedStates) return;
    d3d_LightEnable(device, 0, FALSE);
    d3d_LightEnable(device, 1, FALSE);
    d3d_SetRenderState(device, D3DRS_FOGENABLE, g_origFogEnable);
    d3d_SetRenderState(device, D3DRS_FOGCOLOR, g_origFogColor);
    d3d_SetRenderState(device, D3DRS_FOGTABLEMODE, g_origFogTableMode);
    d3d_SetRenderState(device, D3DRS_FOGSTART, g_origFogStart);
    d3d_SetRenderState(device, D3DRS_FOGEND, g_origFogEnd);
    d3d_SetRenderState(device, D3DRS_FOGVERTEXMODE, g_origFogVertexMode);
    d3d_SetRenderState(device, D3DRS_LIGHTING, g_origLighting);
    d3d_SetRenderState(device, D3DRS_AMBIENT, g_origAmbient);
    g_savedStates = false;
}

static void applyNeonLighting() {
    void* device = getD3DDevice();
    if (!device) return;

    saveOriginalStates(device);
    initMaterial();

    HBPlusAPI hb = HBAPI(g_api);
    float fogNear    = hb.GetSliderState("NEON_FOG_NEAR");
    float fogFar     = hb.GetSliderState("NEON_FOG_FAR");
    float lightRange = hb.GetSliderState("NEON_LIGHT_RANGE");
    float lightR     = hb.GetSliderState("NEON_LIGHT_R");
    float lightG     = hb.GetSliderState("NEON_LIGHT_G");
    float lightB     = hb.GetSliderState("NEON_LIGHT_B");

    if (fogFar <= fogNear) fogFar = fogNear + 100.0f;

    d3d_SetRenderState(device, D3DRS_FOGENABLE, 1);
    d3d_SetRenderState(device, D3DRS_FOGCOLOR, 0x00000000);
    d3d_SetRenderState(device, D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
    d3d_SetRenderState(device, D3DRS_FOGVERTEXMODE, 0);

    DWORD fsDW = *(DWORD*)&fogNear;
    DWORD feDW = *(DWORD*)&fogFar;
    d3d_SetRenderState(device, D3DRS_FOGSTART, fsDW);
    d3d_SetRenderState(device, D3DRS_FOGEND, feDW);

    d3d_SetRenderState(device, D3DRS_LIGHTING, 1);
    d3d_SetRenderState(device, D3DRS_AMBIENT, 0x00141414);
    d3d_SetMaterial(device, &g_material);

    void* p1 = hb.GetPlayer();
    float bx, by, bz;
    if (getBallPos(p1, &bx, &by, &bz)) {
        memset(&g_light, 0, sizeof(g_light));
        g_light.Type = D3DLIGHT_POINT;
        g_light.Diffuse[0] = lightR;
        g_light.Diffuse[1] = lightG;
        g_light.Diffuse[2] = lightB;
        g_light.Diffuse[3] = 1.0f;
        g_light.Position[0] = bx;
        g_light.Position[1] = by;
        g_light.Position[2] = bz;
        g_light.Range = lightRange;
        g_light.Falloff = 1.0f;
        g_light.Attenuation0 = 1.0f;
        d3d_SetLight(device, 0, &g_light);
        d3d_LightEnable(device, 0, TRUE);

        void* p2 = hb.GetPlayer2();
        float p2x, p2y, p2z;
        if (p2 && getBallPos(p2, &p2x, &p2y, &p2z)) {
            g_light.Position[0] = p2x;
            g_light.Position[1] = p2y;
            g_light.Position[2] = p2z;
            d3d_SetLight(device, 1, &g_light);
            d3d_LightEnable(device, 1, TRUE);
        } else {
            d3d_LightEnable(device, 1, FALSE);
        }
    }
}

// ── Vtable Callbacks ──────────────────────────────────────────────

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (g_enabled) {
        void* device = getD3DDevice();
        if (device) restoreOriginalStates(device);
    }
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Neon Lighting"; }
static const char* __thiscall get_author(void*) { return "Hamsterbot"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) {
    return "Recreates Neon Race darkness + glow in all levels";
}

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    HBPlusAPI hb = HBAPI(api);

    CustomButton btn("NEON_LIGHTS", "Neon Lighting Effect");
    btn.defaultState = false;
    btn.trueText = "ON";
    btn.falseText = "OFF";
    hb.CreateToggleButton(btn, thisptr);

    CustomSlider sFogNear("NEON_FOG_NEAR", "Neon Fog Start", 150.0f);
    sFogNear.lowerBound = 10.0f; sFogNear.upperBound = 3000.0f;
    sFogNear.stepSize = 10.0f; sFogNear.decimalPlaces = 0;
    hb.CreateSlider(sFogNear, thisptr);

    CustomSlider sFogFar("NEON_FOG_FAR", "Neon Fog End", 600.0f);
    sFogFar.lowerBound = 100.0f; sFogFar.upperBound = 10000.0f;
    sFogFar.stepSize = 50.0f; sFogFar.decimalPlaces = 0;
    hb.CreateSlider(sFogFar, thisptr);

    CustomSlider sLightRange("NEON_LIGHT_RANGE", "Neon Light Range", 400.0f);
    sLightRange.lowerBound = 50.0f; sLightRange.upperBound = 3000.0f;
    sLightRange.stepSize = 10.0f; sLightRange.decimalPlaces = 0;
    hb.CreateSlider(sLightRange, thisptr);

    CustomSlider sLR("NEON_LIGHT_R", "Neon Light Red", 1.0f);
    sLR.lowerBound = 0.0f; sLR.upperBound = 2.0f;
    sLR.stepSize = 0.05f; sLR.decimalPlaces = 2;
    hb.CreateSlider(sLR, thisptr);

    CustomSlider sLG("NEON_LIGHT_G", "Neon Light Green", 1.0f);
    sLG.lowerBound = 0.0f; sLG.upperBound = 2.0f;
    sLG.stepSize = 0.05f; sLG.decimalPlaces = 2;
    hb.CreateSlider(sLG, thisptr);

    CustomSlider sLB("NEON_LIGHT_B", "Neon Light Blue", 0.0f);
    sLB.lowerBound = 0.0f; sLB.upperBound = 2.0f;
    sLB.stepSize = 0.05f; sLB.decimalPlaces = 2;
    hb.CreateSlider(sLB, thisptr);
}

static void __thiscall ball_update(void*, Ball*) {}
static void __thiscall render_apply(void*, void*, float*) {}

static void __thiscall button_toggle(void* thisptr, const char* buttonId, bool newState) {
    if (strcmp(buttonId, "NEON_LIGHTS") == 0) {
        if (newState) {
            g_enabled = true;
            g_savedStates = false;
        } else {
            g_enabled = false;
            void* device = getD3DDevice();
            if (device) restoreOriginalStates(device);
        }
    }
}

static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall event_collide(void*, Ball*, char*) {}
static void __thiscall ball_bump(void*, Ball*, Ball*) {}
static void __thiscall text_render(void*) {}

static void __thiscall scene_end(void*) {
    if (g_enabled) g_savedStates = false;
}

static void __thiscall level_start(void*) {
    g_frameCount = 0;
    if (g_enabled) g_savedStates = false;
}

static void __thiscall game_update(void*) {
    g_frameCount++;
    if (!g_gameReady && g_frameCount > 120) g_gameReady = true;
    if (g_gameReady && g_enabled) applyNeonLighting();
}

// ── 16-Entry Manual Vtable (MSVC layout) ──────────────────────────

static void* g_vtable[16] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update,
    (void*)render_apply,
    (void*)button_toggle,
    (void*)slider_change,
    (void*)game_update,
    (void*)event_collide,
    (void*)text_render,
    (void*)ball_bump,
    (void*)scene_end,
    (void*)level_start,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
