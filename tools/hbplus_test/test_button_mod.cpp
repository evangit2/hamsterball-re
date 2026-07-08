#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

typedef void* (__thiscall *dtor_t)(void* thisptr, int flags);
typedef const char* (__thiscall *get_name_t)(void*);
typedef int (__thiscall *get_version_t)(void*);
typedef void (__thiscall *init_t)(void* thisptr, IModAPI* api);
typedef void (__thiscall *ball_update_t)(void*, Ball*);
typedef void (__thiscall *render_apply_t)(void*, void*, float*);
typedef void (__thiscall *button_toggle_t)(void*, const char*, bool);
typedef void (__thiscall *slider_change_t)(void*, const char*, float);
typedef void (__thiscall *game_update_t)(void*);
typedef void (__thiscall *event_collide_t)(void*, Ball*, char*);
typedef void (__thiscall *text_render_t)(void*);
typedef void (__thiscall *ball_bump_t)(void*, Ball*, Ball*);
typedef void (__thiscall *scene_end_t)(void*);
typedef void (__thiscall *level_start_t)(void*);

static IModAPI* g_api = NULL;

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}
static const char* __thiscall get_mod_name(void*) { return "TestButton"; }
static const char* __thiscall get_author(void*) { return "hbtest"; }
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }
static const char* __thiscall get_contributors(void*) { return ""; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    CustomButton btn("TEST_BUTTON", "Test Button");
    btn.defaultState = false;
    btn.trueText = "ON";
    btn.falseText = "OFF";
    if (api) {
        HBAPI(api).CreateToggleButton(btn, thisptr);
    }
}

static void __thiscall ball_update(void*, Ball*) {}
static void __thiscall render_apply(void*, void*, float*) {}
static void __thiscall button_toggle(void*, const char* id, bool state) {
    // Use Win32 API instead of CRT fopen/fprintf
    HANDLE h = CreateFileA("test_button_log.txt", FILE_APPEND_DATA, 0, NULL, OPEN_ALWAYS, 0, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        char buf[128];
        const char* s = state ? "ON" : "OFF";
        // Simple sprintf: "Button: ID = STATE\n"
        int len = 0;
        const char* prefix = "Button: ";
        while (prefix[len]) buf[len] = prefix[len], len++;
        int i = 0; while (id[i]) buf[len] = id[i], len++, i++;
        buf[len++] = ' '; buf[len++] = '='; buf[len++] = ' ';
        i = 0; while (s[i]) buf[len] = s[i], len++, i++;
        buf[len++] = '\n';
        DWORD written;
        WriteFile(h, buf, len, &written, NULL);
        CloseHandle(h);
    }
}
static void __thiscall slider_change(void*, const char*, float) {}
static void __thiscall game_update(void*) {}
static void __thiscall event_collide(void*, Ball*, char*) {}
static void __thiscall text_render(void*) {}
static void __thiscall ball_bump(void*, Ball*, Ball*) {}
static void __thiscall scene_end(void*) {}
static void __thiscall level_start(void*) {}

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
