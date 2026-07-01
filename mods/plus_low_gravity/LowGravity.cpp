// Low Gravity mod for Hamsterball Plus API
// Adds a toggle button to reduce the player's gravity.
// Uses PhysicsObject->gravity_y (the API-recommended way to modify gravity).
// Default gravity_y = 0.5f; we reduce it to 0.125f (25%) when enabled.
//
// Build: Place in ModTemplate, replace MainModFile.cpp with this, build as DLL.

#include "HamsterballAPI.h"
#include <windows.h>

class LowGravityMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

    static constexpr float LOW_GRAVITY = -0.125f;  // Y-up: negative = down. Default is ~-0.5f

public:
    const char* GetModName() override    { return "Low Gravity"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton gravityButton("CHEAT_LOWGRAV", "LOW GRAVITY");
        api->CreateToggleButton(gravityButton, this);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "CHEAT_LOWGRAV") == 0) {
            CustomText params;
            params.x = 400;
            params.y = 100;

            if (newState) {
                api->DrawTimedMessage("Low Gravity: ON", params, 2.0f);
            } else {
                api->DrawTimedMessage("Low Gravity: OFF", params, 2.0f);
            }
        }
    }

    void onBallUpdate(Ball* ball) override {
        if (!api->GetButtonState("CHEAT_LOWGRAV")) return;
        if (!ball) return;

        // Only apply to player balls, not badballs
        if (ball->playerID < 0) return;

        // Use PhysicsObject->gravity_y (API recommended approach)
        PhysicsObject* phys = ball->physics_object;
        if (phys) {
            phys->gravity_y = LOW_GRAVITY;
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new LowGravityMod();
}
