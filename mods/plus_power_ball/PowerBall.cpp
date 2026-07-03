// Power Ball mod for Hamsterball Plus API
// Makes the player ball stronger than 8-Ball and Fun Ball.
// Increases radius (knockout power) and max speed when toggled on.
// Player radius 55.0 > 8-ball 35.0, so player can knock out 8-balls
// and 8-balls cannot knock out the player (35*0.7=24.5 < 55).

#include "HamsterballAPI.h"
#include <windows.h>

class PowerBallMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;

    static constexpr float POWER_RADIUS = 55.0f;
    static constexpr float POWER_SPEED  = 10.0f;

public:
    const char* GetModName() override    { return "Power Ball"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        CustomButton btn("CHEAT_POWERBALL", "POWER BALL");
        api->CreateToggleButton(btn, this);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "CHEAT_POWERBALL") != 0) return;
        CustomText params;
        params.x = 400;
        params.y = 100;
        if (newState)
            api->DrawTimedMessage("Power Ball: ON", params, 2.0f);
        else
            api->DrawTimedMessage("Power Ball: OFF", params, 2.0f);
    }

    void onBallUpdate(Ball* ball) override {
        if (!api->GetButtonState("CHEAT_POWERBALL")) return;
        if (!ball || ball->playerID < 0) return;
        ball->radius = POWER_RADIUS;
        ball->max_speed = POWER_SPEED;
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new PowerBallMod();
}
