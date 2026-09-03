// WobblyBridgeFix_MinGW.cpp — HB+ MinGW build (17-entry vtable, nocrt-friendly)
// Same logic as WobblyBridgeFix.cpp but with MinGW static_assert workaround
// Build with build.sh
#include "HamsterballAPI.h"
#include <windows.h>
#include <string.h>

class WobblyBridgeFix : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    bool patched = false;
    static const DWORD PATCH_ADDR = 0x00440752;
    static const BYTE PATCH_ON[5];
    static const BYTE PATCH_OFF[5];

    bool writePatch(const BYTE* bytes) {
        if (api) {
            api->PatchMemory(PATCH_ADDR, (const char*)bytes, 5);
            return true;
        }
        DWORD old;
        if (!VirtualProtect((void*)PATCH_ADDR, 5, PAGE_EXECUTE_READWRITE, &old)) return false;
        memcpy((void*)PATCH_ADDR, bytes, 5);
        VirtualProtect((void*)PATCH_ADDR, 5, old, &old);
        FlushInstructionCache(GetCurrentProcess(), (void*)PATCH_ADDR, 5);
        return true;
    }

public:
    const char* GetModName() override { return "Wobbly Bridge Fix"; }
    const char* GetAuthorName() override { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        CustomButton btn("WOBBLY_FIX", "Wobbly Bridge Fix");
        btn.defaultState = true;
        api->CreateToggleButton(btn, this);
        writePatch(PATCH_ON);
        patched = true;
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "WOBBLY_FIX") != 0) return;
        if (newState && !patched) { writePatch(PATCH_ON); patched = true; }
        else if (!newState && patched) { writePatch(PATCH_OFF); patched = false; }
    }
};

const BYTE WobblyBridgeFix::PATCH_ON[5]  = {0x68, 0x00, 0x00, 0x00, 0x00};
const BYTE WobblyBridgeFix::PATCH_OFF[5] = {0x68, 0x00, 0x20, 0x00, 0x00};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new WobblyBridgeFix();
}
// Ensure 17-entry vtable (HB+ v2.1) — MinGW needs no extra work if header is patched

