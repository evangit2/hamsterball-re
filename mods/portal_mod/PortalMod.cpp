#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <vector>

struct PortalEntry {
    int setNum;
    int portalNum;
    float x, y, z;
};

class PortalMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    std::vector<PortalEntry> portals;
    int cooldowns[4] = {0, 0, 0, 0};
    bool scanned = false;

public:
    const char* GetModName() override    { return "Portal Mod"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
    }

    void onLevelStart() override {
        portals.clear();
        scanned = false;
        for (int i = 0; i < 4; i++) cooldowns[i] = 0;
        scanPortals();
    }

    void onSceneEnd() override {
        portals.clear();
        scanned = false;
    }

    void onBallUpdate(Ball* ball) override {
        if (!ball || ball->playerID < 0 || ball->playerID >= 4) return;
        if (cooldowns[ball->playerID] > 0)
            cooldowns[ball->playerID]--;
    }

    void onGameUpdate() override {
        if (!scanned) {
            scanPortals();
            scanned = true;
        }
    }

    void onEventPlaneCollide(Ball* ball, char* eventPlaneID) override {
        if (!ball || !eventPlaneID) return;
        if (ball->playerID < 0 || ball->playerID >= 4) return;
        if (cooldowns[ball->playerID] > 0) return;

        if (_strnicmp(eventPlaneID, "E:PORTAL(", 9) != 0) return;

        int setNum = 0, portalNum = 0;
        if (sscanf(eventPlaneID + 9, "%d.%d", &setNum, &portalNum) != 2) return;

        if (portals.empty()) scanPortals();

        PortalEntry* target = nullptr;
        for (size_t i = 0; i < portals.size(); i++) {
            if (portals[i].setNum == setNum && portals[i].portalNum != portalNum) {
                target = &portals[i];
                break;
            }
        }

        if (!target) return;

        ball->pos_x = target->x;
        ball->pos_y = target->y;
        ball->pos_z = target->z;
        ball->prev_pos_x = target->x;
        ball->prev_pos_y = target->y;
        ball->prev_pos_z = target->z;

        cooldowns[ball->playerID] = 30;
    }

private:
    void scanPortals() {
        Scene* scene = api->GetScene();
        if (!scene) return;

        DWORD sceneAddr = (DWORD)scene;
        if (IsBadReadPtr((void*)(sceneAddr + 0x8AC), 4)) return;
        DWORD levelPtr = *(DWORD*)(sceneAddr + 0x8AC);
        if (!levelPtr || levelPtr < 0x10000) return;

        if (IsBadReadPtr((void*)(levelPtr + 0x480), 4)) return;
        DWORD spatialTree = *(DWORD*)(levelPtr + 0x480);
        if (!spatialTree || spatialTree < 0x10000) return;

        if (IsBadReadPtr((void*)(spatialTree + 0x10B8), 8)) return;
        int count = *(int*)(spatialTree + 0xCB0);
        if (count <= 0 || count > 10000) return;

        int* data = *(int**)(spatialTree + 0x10B8);
        if (!data || IsBadReadPtr(data, count * 4)) return;

        for (int i = 0; i < count; i++) {
            DWORD obj = data[i];
            if (!obj || obj < 0x10000) continue;
            if (IsBadReadPtr((void*)obj, 0x460)) continue;

            char* name = *(char**)(obj + 0x50);
            if (!name || IsBadReadPtr(name, 16)) continue;

            if (_strnicmp(name, "E:PORTAL(", 9) == 0) {
                int sn = 0, pn = 0;
                if (sscanf(name + 9, "%d.%d", &sn, &pn) == 2 && sn > 0 && pn > 0) {
                    PortalEntry entry;
                    entry.setNum = sn;
                    entry.portalNum = pn;
                    entry.x = *(float*)(obj + 0x454);
                    entry.y = *(float*)(obj + 0x458);
                    entry.z = *(float*)(obj + 0x45C);
                    portals.push_back(entry);
                }
            }
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new PortalMod();
}
