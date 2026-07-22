/*
 * KeyboardForce.cpp — Keyboard/Controller Force Scale mod (HB+ v2.1)
 *
 * Adds a slider to the HB+ options menu that lets you adjust the force
 * applied by keyboard and controller input.
 *
 *   Default: 0.12  (original game value — normal keyboard/controller force)
 *   Min:     0.10  (matches mouse at 0% sensitivity)
 *   Max:     0.26  (matches mouse at 100% sensitivity)
 *   Step:    0.01
 *
 * How it works:
 *   The game uses a float at 0x4D03B8 (KeyboardForceScale) to scale the
 *   3D force direction for keyboard/gamepad input. Mouse input uses a
 *   separate formula (sensitivity × 0.16 + 0.1) and ignores this value.
 *   This mod patches KeyboardForceScale every frame to the slider value.
 *   Mouse is completely unaffected.
 *
 * Build with Visual Studio (MSVC):
 *   1. Create a DLL project (x86, not x64)
 *   2. Add HamsterballAPI.h to the project
 *   3. Add this file as the main source
 *   4. Build as plus_keyboard_force.dll
 *   5. Place in game's Mods\ folder
 */
#include "HamsterballAPI.h"

// RVA of KeyboardForceScale in Hamsterball.exe (float, 4 bytes)
static constexpr DWORD KB_FORCE_RVA = 0xD03B8;
static constexpr float KB_FORCE_DEFAULT = 0.12f;
static constexpr float KB_FORCE_MIN = 0.10f;
static constexpr float KB_FORCE_MAX = 0.26f;

class KeyboardForceMod : public HamsterballAPI {
public:
    const char* GetModName() override { return "Keyboard Force"; }
    const char* GetAuthorName() override { return "BookwormKevin"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }
    const char* GetContributors() override { return "Hamsterbot"; }

    void Initialize(IModAPI* api) override {
        m_api = api;

        CustomSlider slider;
        slider.id = "KB_FORCE";
        slider.displayText = "Keyboard/Controller Force";
        slider.startingValue = KB_FORCE_DEFAULT;
        slider.stepSize = 0.01f;
        slider.lowerBound = KB_FORCE_MIN;
        slider.upperBound = KB_FORCE_MAX;
        slider.decimalPlaces = 2;
        slider.unitName = "";
        slider.color = Color(0.4f, 0.8f, 1.0f, 1.0f); // light blue
        api->CreateSlider(slider, this);
    }

    void onGameUpdate() override {
        if (!m_api) return;

        float value = m_api->GetSliderState("KB_FORCE");
        if (value < KB_FORCE_MIN) value = KB_FORCE_MIN;
        if (value > KB_FORCE_MAX) value = KB_FORCE_MAX;

        // Patch KeyboardForceScale at game base + RVA
        DWORD gameBase = (DWORD)GetModuleHandleA(NULL);
        float* pForceScale = (float*)(gameBase + KB_FORCE_RVA);
        *pForceScale = value;
    }

private:
    IModAPI* m_api = nullptr;
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new KeyboardForceMod();
}
