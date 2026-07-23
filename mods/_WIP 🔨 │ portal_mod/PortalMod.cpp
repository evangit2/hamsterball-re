#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

class PortalMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    int cooldowns[4] = {0, 0, 0, 0};

public:
    const char* GetModName() override    { return "Portal Mod"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
    }

    void onBallUpdate(Ball* ball) override {
        if (!ball || ball->playerID < 0 || ball->playerID >= 4) return;
        if (cooldowns[ball->playerID] > 0)
            cooldowns[ball->playerID]--;
    }

    void onEventPlaneCollide(Ball* ball, char* eventPlaneID) override {
        if (!ball || !eventPlaneID) return;
        if (ball->playerID < 0 || ball->playerID >= 4) return;
        if (cooldowns[ball->playerID] > 0) return;

        // Parse E:PORTAL(N)
        if (_strnicmp(eventPlaneID, "E:PORTAL(", 9) != 0) return;

        int portalNum = 0;
        if (sscanf(eventPlaneID + 9, "%d", &portalNum) != 1 || portalNum < 1) return;

        // Look up PORTALPOS(N) ref point directly
        Scene* scene = api->GetScene();
        if (!scene) return;

        DWORD sceneAddr = (DWORD)scene;
        if (IsBadReadPtr((void*)(sceneAddr + 0x8AC), 4)) return;
        DWORD levelPtr = *(DWORD*)(sceneAddr + 0x8AC);
        if (!levelPtr || levelPtr < 0x10000) return;

        char posName[64];
        snprintf(posName, sizeof(posName), "PORTALPOS(%d)", portalNum);
        float destX, destY, destZ;
        if (!lookupRefPoint(levelPtr, posName, &destX, &destY, &destZ)) return;

        // Try to look up PORTALVEC(N) for exit direction
        char vecName[64];
        snprintf(vecName, sizeof(vecName), "PORTALVEC(%d)", portalNum);
        float vecX = 0, vecY = 0, vecZ = 0;
        bool hasVec = lookupRefPoint(levelPtr, vecName, &vecX, &vecY, &vecZ);

        // Read current velocity to preserve speed magnitude
        PhysicsObject* phys = ball->physics_object;
        float velX = 0, velY = 0, velZ = 0;
        float currentSpeed = 0;
        if (phys && !IsBadReadPtr(phys, 0x20)) {
            velX = phys->velocity_x;
            velY = phys->velocity_y;
            velZ = phys->velocity_z;
            currentSpeed = sqrtf(velX*velX + velY*velY + velZ*velZ);
        }

        // Use native teleport flag system
        // Game writes the flag as a BYTE (undefined1), matching OddBoard_CollisionHandler
        *(BYTE*)((char*)ball + 0xC3C) = 1;       // teleport flag
        *(float*)((char*)ball + 0xC40) = destX;  // dest X
        // Add ball radius to Y so the ball doesn't spawn inside the floor.
        // ToobBoard handler does: param_2[0x311] += ball+0x284 (radius) + epsilon
        *(float*)((char*)ball + 0xC44) = destY + ball->radius;  // dest Y
        *(float*)((char*)ball + 0xC48) = destZ;  // dest Z

        // Set exit velocity
        if (phys && !IsBadReadPtr(phys, 0x20)) {
            if (hasVec) {
                // Direction = PORTALVEC - PORTALPOS, normalized
                float dx = vecX - destX;
                float dy = vecY - destY;
                float dz = vecZ - destZ;
                float len = sqrtf(dx*dx + dy*dy + dz*dz);

                if (len > 0.001f) {
                    // Preserve original speed, redirect along exit vector
                    float scale = currentSpeed / len;
                    *(float*)((char*)phys + 0xCA4) = dx * scale;
                    *(float*)((char*)phys + 0xCA8) = dy * scale;
                    *(float*)((char*)phys + 0xCAC) = dz * scale;
                }
            } else {
                // No PORTALVEC — keep input direction, preserve speed
                *(float*)((char*)phys + 0xCA4) = velX;
                *(float*)((char*)phys + 0xCA8) = velY;
                *(float*)((char*)phys + 0xCAC) = velZ;
            }
        }

        cooldowns[ball->playerID] = 30;
    }

private:
    // Look up a ref point by name from the level's S1 ref point table.
    // Ghidra-verified from AthenaHashTable_Lookup (0x4605E0):
    //   level+0x480 = sceneobj
    //   sceneobj+0x898 = count
    //   *(sceneobj+0xCA0) → double-deref to array of entry pointers
    //   Each entry: [0]=name ptr, [1]=x(float), [2]=y(float), [3]=z(float)
    bool lookupRefPoint(DWORD levelPtr, const char* name, float* outX, float* outY, float* outZ) {
        if (IsBadReadPtr((void*)(levelPtr + 0x480), 4)) return false;
        DWORD sceneobj = *(DWORD*)(levelPtr + 0x480);
        if (!sceneobj || sceneobj < 0x10000) return false;

        if (IsBadReadPtr((void*)(sceneobj + 0x898), 4)) return false;
        int count = *(int*)(sceneobj + 0x898);
        if (count <= 0 || count > 10000) return false;

        if (IsBadReadPtr((void*)(sceneobj + 0xCA0), 4)) return false;
        DWORD arrayPtr = *(DWORD*)(sceneobj + 0xCA0);
        if (!arrayPtr || arrayPtr < 0x10000) return false;

        DWORD* entries = *(DWORD**)arrayPtr;
        if (!entries || IsBadReadPtr(entries, count * 4)) return false;

        for (int i = 0; i < count; i++) {
            DWORD entry = entries[i];
            if (!entry || entry < 0x10000) continue;
            if (IsBadReadPtr((void*)entry, 16)) continue;

            char* entryName = *(char**)entry;
            if (!entryName || IsBadReadPtr(entryName, strlen(name) + 1)) continue;

            if (_stricmp(entryName, name) == 0) {
                *outX = *(float*)(entry + 0x04);
                *outY = *(float*)(entry + 0x08);
                *outZ = *(float*)(entry + 0x0C);
                return true;
            }
        }
        return false;
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new PortalMod();
}
