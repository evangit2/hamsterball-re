#include "HamsterballAPI.h"
#include <windows.h>

static constexpr DWORD TIMER_IMM_ADDR = 0x42185B;

class ArenaTimerMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    HANDLE m_thread = NULL;
    volatile bool m_running = true;

    void createSlider(const char* id, const char* label, int defaultVal, int minVal, int maxVal) {
        CustomSlider s(id, label, (float)defaultVal);
        s.lowerBound = (float)minVal;
        s.upperBound = (float)maxVal;
        s.stepSize = 100.0f;
        s.decimalPlaces = 0;
        api->CreateSlider(s, this);
    }

    static void patchInt(DWORD addr, int value) {
        DWORD oldProtect;
        if (VirtualProtect((void*)addr, 4, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            memcpy((void*)addr, &value, sizeof(int));
            VirtualProtect((void*)addr, 4, oldProtect, &oldProtect);
            FlushInstructionCache(GetCurrentProcess(), (void*)addr, 4);
        }
    }

    static DWORD WINAPI timerThread(LPVOID param) {
        ArenaTimerMod* self = (ArenaTimerMod*)param;
        IModAPI* api = self->api;

        Sleep(5000);

        while (self->m_running) {
            Sleep(500);
            int timerVal = (int)api->GetSliderState("ARENA_TIMER");
            if (timerVal < 100) timerVal = 100;
            patchInt(TIMER_IMM_ADDR, timerVal);
        }
        return 0;
    }

public:
    const char* GetModName() override    { return "Arena Timer"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "v2: VirtualProtect code patching"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        createSlider("ARENA_TIMER", "Arena Timer (ticks)", 6000, 100, 60000);

        m_thread = CreateThread(NULL, 0, timerThread, this, 0, NULL);
    }

    ~ArenaTimerMod() {
        m_running = false;
        if (m_thread) {
            WaitForSingleObject(m_thread, 1000);
            CloseHandle(m_thread);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new ArenaTimerMod();
}
