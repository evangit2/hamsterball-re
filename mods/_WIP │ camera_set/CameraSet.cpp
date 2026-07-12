#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <cmath>

#define PI_F 3.14159265358979323846f

class CameraSetMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    HANDLE threadHandle = NULL;
    bool running = true;

    // PITCH_PATCH_ADDR: address of the 0.9f immediate in Scene_SetCamera (0x419FA0)
    // Instruction at 0x41A2B1: C7 44 24 2C 66 66 66 3F (MOV dword [ESP+0x2C], 0x3F666666)
    // The imm32 (0x3F666666 = 0.9f) is at 0x41A2B1 + 4 = 0x41A2B5
    static constexpr DWORD PITCH_PATCH_ADDR = 0x41A2B5;

    // Default values
    static constexpr float DEF_ANGLE = 2.356f;      // 135 degrees in radians
    static constexpr float DEF_DISTANCE = 1100.0f;   // race default
    static constexpr float DEF_PITCH = 0.9f;
    static constexpr float DEF_OFFSET = 0.0f;

    float lastPitch = -1.0f;

    Scene* getScene() {
        App* app = api->GetApp();
        if (!app) return nullptr;
        DWORD a = (DWORD)app;
        if (IsBadReadPtr((void*)(a + 0x220), 4)) return nullptr;
        DWORD profile = *(DWORD*)(a + 0x220);
        if (!profile || profile < 0x10000) return nullptr;
        if (IsBadReadPtr((void*)(profile + 0x0C), 4)) return nullptr;
        DWORD board = *(DWORD*)(profile + 0x0C);
        if (!board || board < 0x10000) return nullptr;
        if (IsBadReadPtr((void*)(board + 0x878), 4)) return nullptr;
        DWORD scene = *(DWORD*)(board + 0x878);
        if (!scene || scene < 0x10000) return nullptr;
        return (Scene*)scene;
    }

    void applyCamera() {
        if (!api) return;
        bool enabled = api->GetButtonState("CAM_ENABLED");
        if (!enabled) {
            // Restore default pitch if we had patched it
            if (lastPitch != DEF_PITCH && lastPitch != -1.0f) {
                float pitch = DEF_PITCH;
                api->PatchMemory(PITCH_PATCH_ADDR, (const char*)&pitch, 4);
                lastPitch = -1.0f;
            }
            return;
        }

        Scene* scene = getScene();
        if (!scene) return;
        DWORD s = (DWORD)scene;
        if (IsBadWritePtr((void*)s, 0x5000)) return;

        float angle = api->GetSliderState("CAM_ANGLE");
        float distance = api->GetSliderState("CAM_DISTANCE");
        float pitch = api->GetSliderState("CAM_PITCH");
        float offsetX = api->GetSliderState("CAM_OFF_X");
        float offsetY = api->GetSliderState("CAM_OFF_Y");
        float offsetZ = api->GetSliderState("CAM_OFF_Z");

        // Write orbit angle (Scene+0x29BC) and target distance (Scene+0x4384)
        *(float*)(s + 0x29BC) = angle;
        // Camera_SmoothDistance writes both +0x29C0 and +0x4384(target) per-frame,
        // so we must write BOTH to prevent the game from fighting us
        *(float*)(s + 0x29C0) = distance;
        if (!IsBadWritePtr((void*)(s + 0x4384), 4))
            *(float*)(s + 0x4384) = distance;

        // Write camera offset (Scene+0x434C/4350/4354)
        *(float*)(s + 0x434C) = offsetX;
        *(float*)(s + 0x4350) = offsetY;
        *(float*)(s + 0x4354) = offsetZ;

        // Patch pitch immediate via PatchMemory (only when changed)
        if (pitch != lastPitch) {
            api->PatchMemory(PITCH_PATCH_ADDR, (const char*)&pitch, 4);
            lastPitch = pitch;
        }
    }

    static DWORD WINAPI CameraThread(LPVOID param) {
        CameraSetMod* mod = (CameraSetMod*)param;
        while (mod->running) {
            mod->applyCamera();
            Sleep(16);
        }
        return 0;
    }

public:
    const char* GetModName() override    { return "Camera Set"; }
    const char* GetAuthorName() override { return "umans"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        api->CreateToggleButton(
            CustomButton("CAM_ENABLED", "Custom Camera"),
            this);

        api->CreateSlider(
            CustomSlider("CAM_ANGLE", "Orbit Angle (rad)", DEF_ANGLE),
            this);
        // Reconfigure bounds after construction
        // (CustomSlider doesn't support bounds in constructor, set via slider state)

        api->CreateSlider(
            CustomSlider("CAM_DISTANCE", "Zoom Distance", DEF_DISTANCE),
            this);

        api->CreateSlider(
            CustomSlider("CAM_PITCH", "Pitch (elevation)", DEF_PITCH),
            this);

        api->CreateSlider(
            CustomSlider("CAM_OFF_X", "Offset X", DEF_OFFSET),
            this);

        api->CreateSlider(
            CustomSlider("CAM_OFF_Y", "Offset Y", DEF_OFFSET),
            this);

        api->CreateSlider(
            CustomSlider("CAM_OFF_Z", "Offset Z", DEF_OFFSET),
            this);

        running = true;
        threadHandle = CreateThread(NULL, 0, CameraThread, this, 0, NULL);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "CAM_ENABLED") == 0) {
            if (!newState && lastPitch != -1.0f) {
                // Restore default pitch when disabled
                float pitch = DEF_PITCH;
                api->PatchMemory(PITCH_PATCH_ADDR, (const char*)&pitch, 4);
                lastPitch = -1.0f;
            }
            printf("[CameraSet] Custom Camera: %s\n", newState ? "ON" : "OFF");
        }
    }

    void onSliderChange(const char* sliderId, float newValue) override {
        printf("[CameraSet] %s = %.3f\n", sliderId, newValue);
    }

    ~CameraSetMod() {
        running = false;
        if (threadHandle) {
            WaitForSingleObject(threadHandle, 2000);
            CloseHandle(threadHandle);
        }
        // Restore default pitch on unload
        if (api && lastPitch != -1.0f) {
            float pitch = DEF_PITCH;
            api->PatchMemory(PITCH_PATCH_ADDR, (const char*)&pitch, 4);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new CameraSetMod();
}
