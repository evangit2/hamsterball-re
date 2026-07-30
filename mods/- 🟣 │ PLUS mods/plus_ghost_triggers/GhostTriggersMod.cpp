#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Ghost Triggers for HB+ v2.1
 *
 * Port of ghost_triggers (bass.dll proxy) to HB+ API.
 *
 * Simplifications vs bass.dll:
 * - No BASS proxy shell
 * - No background thread / frame-epilogue hook — scan triggers once at level start
 *   and check distances in onGameUpdate
 * - Uses HB+ API GetScene() and direct S1 ref-point traversal
 *
 * Features preserved:
 * - Scans S1 ref points prefixed "GT:" for trigger zones
 * - Reads position from S1 entry and radius from rot_y field
 * - Per-frame distance check against the ghost ball (board+0x361C)
 * - 60-frame cooldown per trigger
 * - Writes events to ghost_triggers_log.txt
 */

#define DEFAULT_RADIUS 50.0f
#define TRIGGER_COOLDOWN_FRAMES 60
#define MAX_TRIGGERS 64

struct Trigger {
    float pos_x, pos_y, pos_z;
    float radius;
    float radius_sq;
    char name[64];
    bool inside;
    int cooldown;
};

class GhostTriggersMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    char g_logPath[MAX_PATH] = "";
    Trigger g_triggers[MAX_TRIGGERS];
    int g_triggerCount = 0;
    DWORD g_currentBoard = 0;
    int g_frameCount = 0;

    void log_msg(const char* msg) {
        if (!g_logPath[0]) return;
        HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD w;
            SetFilePointer(h, 0, NULL, FILE_END);
            WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
            WriteFile(h, "\r\n", 2, &w, NULL);
            CloseHandle(h);
        }
    }

    void log_fmt(const char* fmt, ...) {
        if (!g_logPath[0]) return;
        char buf[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        log_msg(buf);
    }

    void scan_s1_ref_points(DWORD board) {
        g_triggerCount = 0;
        if (!board || IsBadReadPtr((void*)board, 0x4000)) return;

        DWORD level = *(DWORD*)(board + 0x8AC);
        if (!level || IsBadReadPtr((void*)level, 0x1000)) return;

        DWORD mwData = *(DWORD*)(level + 0x480);
        if (!mwData || IsBadReadPtr((void*)mwData, 0x1000)) return;

        int s1Count = *(int*)(mwData + 0x898);
        DWORD s1Array = *(DWORD*)(mwData + 0xCA0);
        if (s1Count <= 0 || s1Count > 1000) return;
        if (!s1Array || IsBadReadPtr((void*)s1Array, s1Count * 4)) return;

        for (int i = 0; i < s1Count && g_triggerCount < MAX_TRIGGERS; i++) {
            DWORD entry = *(DWORD*)(s1Array + i * 4);
            if (!entry || IsBadReadPtr((void*)entry, 7 * 4)) continue;
            char* name = *(char**)(entry + 0);
            if (!name || IsBadReadPtr(name, 4)) continue;
            if (_strnicmp(name, "GT:", 3) == 0) {
                Trigger* t = &g_triggers[g_triggerCount];
                const char* eventName = name + 3;
                strncpy(t->name, eventName, sizeof(t->name) - 1);
                t->name[sizeof(t->name) - 1] = '\0';
                t->pos_x = *(float*)(entry + 0x04);
                t->pos_y = *(float*)(entry + 0x08);
                t->pos_z = *(float*)(entry + 0x0C);
                t->radius = *(float*)(entry + 0x14); // rot_y
                if (t->radius <= 0.0f || t->radius > 5000.0f) t->radius = DEFAULT_RADIUS;
                t->radius_sq = t->radius * t->radius;
                t->inside = false;
                t->cooldown = 0;
                g_triggerCount++;
            }
        }
        log_fmt("scan_s1: found %d GT: triggers", g_triggerCount);
    }

    void check_triggers(DWORD board) {
        g_frameCount++;
        if (g_triggerCount == 0) return;

        DWORD ghostBall = *(DWORD*)(board + 0x361C);
        if (!ghostBall || IsBadReadPtr((void*)ghostBall, 0x200)) {
            if (g_frameCount % 300 == 0)
                log_fmt("heartbeat: frame=%d, triggers=%d, no ghost ball (board=0x%X)",
                        g_frameCount, g_triggerCount, board);
            return;
        }

        float bx = *(float*)(ghostBall + 0x164);
        float by = *(float*)(ghostBall + 0x168);
        float bz = *(float*)(ghostBall + 0x16C);

        if (g_frameCount % 300 == 0)
            log_fmt("heartbeat: frame=%d, triggers=%d, ghost ball at (%.1f, %.1f, %.1f)",
                    g_frameCount, g_triggerCount, bx, by, bz);

        for (int i = 0; i < g_triggerCount; i++) {
            Trigger* t = &g_triggers[i];
            if (t->cooldown > 0) t->cooldown--;

            float dx = bx - t->pos_x;
            float dy = by - t->pos_y;
            float dz = bz - t->pos_z;
            float dist_sq = dx * dx + dy * dy + dz * dz;

            bool wasInside = t->inside;
            t->inside = (dist_sq < t->radius_sq);

            if (!wasInside && t->inside && t->cooldown == 0) {
                float dist = sqrtf(dist_sq);
                log_fmt("TRIGGER FIRED: '%s' - ghost ball entered zone at (%.1f, %.1f, %.1f), dist=%.1f, radius=%.1f, frame=%d",
                        t->name, bx, by, bz, dist, t->radius, g_frameCount);
                t->cooldown = TRIGGER_COOLDOWN_FRAMES;
            }
        }
    }

public:
    const char* GetModName() override      { return "Ghost Triggers"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("GHOST_TRIGGERS", "Log Ghost Triggers");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        char path[MAX_PATH];
        HMODULE hSelf = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                           | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)0x12345678, &hSelf);
        if (hSelf && GetModuleFileNameA(hSelf, path, MAX_PATH)) {
            char* p = strrchr(path, '\\');
            if (p) strcpy(p + 1, "ghost_triggers_log.txt");
            else strcpy(path, "ghost_triggers_log.txt");
        } else {
            strcpy(path, "ghost_triggers_log.txt");
        }
        strncpy(g_logPath, path, MAX_PATH - 1);
        g_logPath[MAX_PATH - 1] = '\0';
        log_msg("[MOD] Ghost Triggers (HB+) loaded");
    }

    void onLevelStart() override {
        App* app = api->GetApp();
        if (!app) return;
        DWORD profile = *(DWORD*)((char*)app + 0x220);
        if (!profile) return;
        DWORD board = *(DWORD*)((char*)profile + 0x0C);
        if (!board || IsBadReadPtr((void*)board, 0x4000)) {
            g_currentBoard = 0;
            return;
        }
        g_currentBoard = board;
        g_frameCount = 0;
        scan_s1_ref_points(board);
    }

    void onGameUpdate() override {
        if (!api->GetButtonState("GHOST_TRIGGERS")) return;
        if (!g_currentBoard) return;

        // Re-validate board pointer (App+0x220->profile+0x0C)
        App* app = api->GetApp();
        if (!app) return;
        DWORD profile = *(DWORD*)((char*)app + 0x220);
        if (!profile) return;
        DWORD board = *(DWORD*)((char*)profile + 0x0C);
        if (board != g_currentBoard) {
            g_currentBoard = board;
            g_frameCount = 0;
            scan_s1_ref_points(board);
        }
        if (g_currentBoard)
            check_triggers(g_currentBoard);
    }

    void onSceneEnd() override {
        g_triggerCount = 0;
        g_currentBoard = 0;
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance_stub() {
    return new GhostTriggersMod();
}

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return CreateModInstance_stub();
}
