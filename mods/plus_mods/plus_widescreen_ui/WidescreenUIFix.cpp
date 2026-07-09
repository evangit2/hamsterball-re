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

// ── State ──────────────────────────────────────────────────────────
static IModAPI* g_api = NULL;
static void* g_modInstance = NULL;
static bool g_enabled = true;
static bool g_inSceneRender = false;
static int g_viewportCallCount = 0;
static bool g_scaleModified = false;
static float g_origScaleX = 0.0f;
static DWORD g_origScaleXAddr = 0;

// ── Hook typedefs & originals ──────────────────────────────────────
typedef void(__fastcall *SetViewport_t)(void*, void*, int, int);
static SetViewport_t orig_SetViewport = NULL;

typedef void(__fastcall *SceneRender_t)(void*, void*, void*);
static SceneRender_t orig_SceneRender = NULL;

// ── UI Scale Fix ───────────────────────────────────────────────────
// UI uses D3DFVF_XYZRHW (transformed) vertices. Gfx_TransformY converts
// pixel X to NDC via: result = pixel * scaleX + offsetX
// where scaleX is at presentParams+0x1f8.
// On 16:9, X/Y ratio is wrong. Fix: set scaleX = 2/(bbHeight * 4/3)
// so X/Y ratio = 3/4 (4:3 proportions), centered on screen.
static void fixUIScale(void* gfx) {
	DWORD gfxAddr = (DWORD)gfx;
	if (IsBadReadPtr(gfx, 0x800)) return;

	DWORD presentParams = *(DWORD*)(gfxAddr + 0x5c);
	if (!presentParams || IsBadReadPtr((void*)presentParams, 0x200)) return;
	int bbWidth = *(int*)(presentParams + 0x15c);
	int bbHeight = *(int*)(presentParams + 0x160);
	if (bbWidth <= 0 || bbHeight <= 0) return;

	float currentAspect = (float)bbWidth / (float)bbHeight;
	if (currentAspect <= 1.34f) return;

	DWORD scaleXAddr = presentParams + 0x1f8;
	float origScaleX = *(float*)scaleXAddr;
	float newScaleX = 2.0f / ((float)bbHeight * (4.0f / 3.0f));

	g_origScaleX = origScaleX;
	g_origScaleXAddr = scaleXAddr;
	g_scaleModified = true;

	*(float*)scaleXAddr = newScaleX;
}

static void restoreUIScale() {
	if (g_scaleModified && g_origScaleXAddr) {
		if (!IsBadWritePtr((void*)g_origScaleXAddr, 4)) {
			*(float*)g_origScaleXAddr = g_origScaleX;
		}
		g_scaleModified = false;
		g_origScaleXAddr = 0;
	}
}

// ── Hooks ──────────────────────────────────────────────────────────
static void __fastcall hook_SetViewport(void* gfx, void* edx, int param1, int param2) {
	orig_SetViewport(gfx, edx, param1, param2);

	if (!g_enabled || !g_inSceneRender) return;
	if (param1 != 0 || param2 != 0) return;

	g_viewportCallCount++;

	// 1st (0,0) = 3D pass, 2nd (0,0) = UI pass
	if (g_viewportCallCount == 2) {
		fixUIScale(gfx);
	}
}

static void __fastcall hook_SceneRender(void* this_ptr, void* edx, void* param1) {
	restoreUIScale();

	g_inSceneRender = true;
	g_viewportCallCount = 0;
	orig_SceneRender(this_ptr, edx, param1);
	g_inSceneRender = false;

	restoreUIScale();
}

// ── Vtable implementations ─────────────────────────────────────────
static void* __thiscall sc_dtor(void* thisptr, int flags) {
	if (flags & 1) operator delete(thisptr);
	return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Widescreen UI Fix"; }
static const char* __thiscall get_author(void*) { return "BookwormKevin"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return ""; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
	g_api = api;
	g_modInstance = thisptr;

	CustomButton btn;
	btn.id = "ws_ui_fix";
	btn.displayText = "Widescreen UI Fix";
	btn.defaultState = true;
	HBAPI(api).CreateToggleButton(btn, thisptr);

	// RegisterCustomHook takes ABSOLUTE addresses (base 0x400000 + RVA)
	HBAPI(api).RegisterCustomHook(0x454f10, (void*)hook_SetViewport, (void**)&orig_SetViewport);
	HBAPI(api).RegisterCustomHook(0x41a2e0, (void*)hook_SceneRender, (void**)&orig_SceneRender);
}

static void __thiscall ball_update(void*, Ball*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char* id, bool state) {
	if (strcmp(id, "ws_ui_fix") == 0) g_enabled = state;
}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall game_update(void*) {}
static void __thiscall event_collide(void*, Ball*, const char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, Ball*, Ball*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

// ── Manual 16-entry vtable (matches MSVC layout) ───────────────────
static void* g_vtable[16] = {
	(void*)sc_dtor,          // [0]  scalar deleting destructor
	(void*)get_mod_name,     // [1]  GetModName
	(void*)get_author,       // [2]  GetAuthorName
	(void*)get_version,      // [3]  GetApiVersion
	(void*)get_contributors, // [4]  GetContributors
	(void*)init_impl,        // [5]  Initialize
	(void*)ball_update,      // [6]  onBallUpdate
	(void*)render_apply,     // [7]  onRenderApply
	(void*)button_toggle,    // [8]  onButtonToggle
	(void*)slider_change,    // [9]  onSliderChange
	(void*)game_update,      // [10] onGameUpdate
	(void*)event_collide,    // [11] onEventPlaneCollide
	(void*)text_render,      // [12] onTextRenderLoop
	(void*)ball_bump,         // [13] onBallBump
	(void*)scene_end,        // [14] onSceneEnd
	(void*)level_start,      // [15] onLevelStart
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
	void* obj = operator new(8);
	*(void**)obj = g_vtable;
	*(void**)((char*)obj + 4) = NULL;
	return (HamsterballAPI*)obj;
}
