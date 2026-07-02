#include "HamsterballAPI.h"
#include <windows.h>

static constexpr DWORD BOARD_COLOR_BASE    = 0x3AB0;
static constexpr DWORD BOARD_COLOR_STRIDE  = 0x14;
static constexpr DWORD APP_PROFILE_OFFSET  = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;
static constexpr DWORD GLOBAL_APP_PTR = 0x5341E0;

static constexpr DWORD AB_P1_R = 0x421BFD, AB_P1_G = 0x421BF8, AB_P1_B = 0x421BF3, AB_P1_A = 0x421BEE;
static constexpr DWORD AB_P2_G = 0x421CBB, AB_P2_B = 0x421CB6, AB_P2_A = 0x421CB1;
static constexpr DWORD AB_P3_R = 0x421D85, AB_P3_G = 0x421D80, AB_P3_B = 0x421D7B, AB_P3_A = 0x421D76;
static constexpr DWORD AB_P4_R = 0x421E4A, AB_P4_G = 0x421E45, AB_P4_A = 0x421E3F;

static constexpr DWORD ALS_P1_R_2P = 0x433116, ALS_P1_G_2P = 0x433111, ALS_P1_B_2P = 0x43310C, ALS_P1_A_2P = 0x433107;
static constexpr DWORD ALS_P2_G = 0x433025, ALS_P2_B = 0x433020;
static constexpr DWORD ALS_P3_R = 0x433063, ALS_P3_G = 0x43305E, ALS_P3_B = 0x433059;
static constexpr DWORD ALS_P4_R = 0x43309C, ALS_P4_G = 0x433097;

static constexpr DWORD DM_P1_R = 0x431B3C, DM_P1_G = 0x431B37, DM_P1_B = 0x431B32, DM_P1_A = 0x431B2D;
static constexpr DWORD DM_P2_G = 0x431B6E, DM_P2_B = 0x431B69, DM_P2_A = 0x431B64;

static float g_p2_red = 0.0f;
static float g_p4_blue = 0.0f;

struct CodeCave {
    DWORD patchSite;
    int savedLen;
    BYTE savedBytes[5];
    DWORD returnAddr;
    void* caveAddr;
};

static CodeCave g_caves[5];

static void installCave(int idx, DWORD patchSite, float* globalFloat, const BYTE* savedBytes, int savedLen, DWORD returnAddr) {
    CodeCave& c = g_caves[idx];
    c.patchSite = patchSite;
    c.savedLen = savedLen;
    c.returnAddr = returnAddr;
    memcpy(c.savedBytes, savedBytes, savedLen);

    BYTE caveCode[32];
    int p = 0;
    caveCode[p++] = 0xFF;
    caveCode[p++] = 0x35;
    *(DWORD*)(caveCode + p) = (DWORD)globalFloat;
    p += 4;
    memcpy(caveCode + p, savedBytes, savedLen);
    p += savedLen;
    caveCode[p++] = 0xE9;
    *(DWORD*)(caveCode + p) = 0;
    p += 4;

    c.caveAddr = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!c.caveAddr) return;
    memcpy(c.caveAddr, caveCode, p);

    DWORD jmpSrc = (DWORD)c.caveAddr + p - 4;
    DWORD jmpRel = returnAddr - (jmpSrc + 4);
    *(DWORD*)((BYTE*)c.caveAddr + p - 4) = jmpRel;

    DWORD oldProtect;
    VirtualProtect((void*)patchSite, 6, PAGE_EXECUTE_READWRITE, &oldProtect);
    BYTE jmpPatch[6];
    jmpPatch[0] = 0xE9;
    *(DWORD*)(jmpPatch + 1) = (DWORD)c.caveAddr - (patchSite + 5);
    if (savedLen == 4) {
        memcpy((void*)patchSite, jmpPatch, 5);
    } else {
        memcpy((void*)patchSite, jmpPatch, 5);
        *(BYTE*)(patchSite + 5) = 0x90;
    }
    VirtualProtect((void*)patchSite, 6, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (void*)patchSite, 6);
}

class BallTintMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    HANDLE m_thread = NULL;
    volatile bool m_running = true;
    bool m_cavesInstalled = false;

    void createColorSlider(const char* id, const char* label, float defaultVal) {
        CustomSlider s(id, label, defaultVal);
        s.lowerBound = 0.0f;
        s.upperBound = 1.0f;
        s.stepSize = 0.05f;
        s.decimalPlaces = 2;
        api->CreateSlider(s, this);
    }

    static bool validateBoard(DWORD board) {
        if (!board || board < 0x10000) return false;
        if (IsBadReadPtr((void*)board, 4)) return false;
        DWORD vtable = *(DWORD*)board;
        return (vtable >= BOARD_VTABLE_MIN && vtable <= BOARD_VTABLE_MAX);
    }

    static void applyBoardColor(DWORD board, int playerIndex, float r, float g, float b) {
        DWORD addr = board + BOARD_COLOR_BASE + (playerIndex * BOARD_COLOR_STRIDE);
        if (IsBadWritePtr((void*)addr, 16)) return;
        *(float*)(addr + 0x00) = r;
        *(float*)(addr + 0x04) = g;
        *(float*)(addr + 0x08) = b;
        *(float*)(addr + 0x0C) = 1.0f;
    }

    static DWORD findBoard() {
        DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
        if (!appPtr || appPtr < 0x10000) return 0;
        if (IsBadReadPtr((void*)(appPtr + APP_PROFILE_OFFSET), 4)) return 0;
        DWORD profile = *(DWORD*)(appPtr + APP_PROFILE_OFFSET);
        if (!profile || profile < 0x10000) return 0;
        if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;
        DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
        if (!board || board < 0x10000) return 0;
        if (!validateBoard(board)) return 0;
        return board;
    }

    void patchFloat(DWORD addr, float value) {
        api->PatchMemory(addr, (const char*)&value, sizeof(float));
    }

    void installCaves() {
        if (m_cavesInstalled) return;

        installCave(0, 0x421CBF, &g_p2_red,
            (const BYTE*)"\xE8\x8B\x14\x03\x00", 5, 0x421CC5);
        installCave(1, 0x421E43, &g_p4_blue,
            (const BYTE*)"\x68\x00\x00\x80\x3F", 5, 0x421E49);
        installCave(2, 0x433029, &g_p2_red,
            (const BYTE*)"\x8D\x4C\x24\x28", 4, 0x43302E);
        installCave(3, 0x433095, &g_p4_blue,
            (const BYTE*)"\x68\x00\x00\x80\x3F", 5, 0x43309B);
        installCave(4, 0x431B72, &g_p2_red,
            (const BYTE*)"\xE8\xD8\x15\x02\x00", 5, 0x431B78);

        m_cavesInstalled = true;
    }

    void patchScoreballColors() {
        float p1r = api->GetSliderState("TINT_P1_R");
        float p1g = api->GetSliderState("TINT_P1_G");
        float p1b = api->GetSliderState("TINT_P1_B");
        float p2r = api->GetSliderState("TINT_P2_R");
        float p2g = api->GetSliderState("TINT_P2_G");
        float p2b = api->GetSliderState("TINT_P2_B");
        float p3r = api->GetSliderState("TINT_P3_R");
        float p3g = api->GetSliderState("TINT_P3_G");
        float p3b = api->GetSliderState("TINT_P3_B");
        float p4r = api->GetSliderState("TINT_P4_R");
        float p4g = api->GetSliderState("TINT_P4_G");
        float p4b = api->GetSliderState("TINT_P4_B");

        g_p2_red = p2r;
        g_p4_blue = p4b;

        patchFloat(AB_P1_R, p1r); patchFloat(AB_P1_G, p1g); patchFloat(AB_P1_B, p1b); patchFloat(AB_P1_A, 1.0f);
        patchFloat(AB_P2_G, p2g); patchFloat(AB_P2_B, p2b); patchFloat(AB_P2_A, 1.0f);
        patchFloat(AB_P3_R, p3r); patchFloat(AB_P3_G, p3g); patchFloat(AB_P3_B, p3b); patchFloat(AB_P3_A, 1.0f);
        patchFloat(AB_P4_R, p4r); patchFloat(AB_P4_G, p4g); patchFloat(AB_P4_A, 1.0f);

        patchFloat(ALS_P1_R_2P, p1r); patchFloat(ALS_P1_G_2P, p1g); patchFloat(ALS_P1_B_2P, p1b); patchFloat(ALS_P1_A_2P, 1.0f);
        patchFloat(ALS_P2_G, p2g); patchFloat(ALS_P2_B, p2b);
        patchFloat(ALS_P3_R, p3r); patchFloat(ALS_P3_G, p3g); patchFloat(ALS_P3_B, p3b);
        patchFloat(ALS_P4_R, p4r); patchFloat(ALS_P4_G, p4g);

        patchFloat(DM_P1_R, p1r); patchFloat(DM_P1_G, p1g); patchFloat(DM_P1_B, p1b); patchFloat(DM_P1_A, 1.0f);
        patchFloat(DM_P2_G, p2g); patchFloat(DM_P2_B, p2b); patchFloat(DM_P2_A, 1.0f);
    }

    static DWORD WINAPI tintThread(LPVOID param) {
        BallTintMod* self = (BallTintMod*)param;
        IModAPI* api = self->api;

        Sleep(3000);
        self->installCaves();

        while (self->m_running) {
            Sleep(16);
            self->patchScoreballColors();

            DWORD board = findBoard();
            if (board) {
                applyBoardColor(board, 0,
                    api->GetSliderState("TINT_P1_R"),
                    api->GetSliderState("TINT_P1_G"),
                    api->GetSliderState("TINT_P1_B"));
                applyBoardColor(board, 1,
                    api->GetSliderState("TINT_P2_R"),
                    api->GetSliderState("TINT_P2_G"),
                    api->GetSliderState("TINT_P2_B"));
                applyBoardColor(board, 2,
                    api->GetSliderState("TINT_P3_R"),
                    api->GetSliderState("TINT_P3_G"),
                    api->GetSliderState("TINT_P3_B"));
                applyBoardColor(board, 3,
                    api->GetSliderState("TINT_P4_R"),
                    api->GetSliderState("TINT_P4_G"),
                    api->GetSliderState("TINT_P4_B"));
            }
        }
        return 0;
    }

public:
    const char* GetModName() override    { return "Ball Tint"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    const char* GetContributors() override { return "v8: code caves for P2-R and P4-B"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        createColorSlider("TINT_P1_R", "P1 Red",   1.0f);
        createColorSlider("TINT_P1_G", "P1 Green", 1.0f);
        createColorSlider("TINT_P1_B", "P1 Blue",  1.0f);
        createColorSlider("TINT_P2_R", "P2 Red",   0.0f);
        createColorSlider("TINT_P2_G", "P2 Green", 0.5f);
        createColorSlider("TINT_P2_B", "P2 Blue",  1.0f);
        createColorSlider("TINT_P3_R", "P3 Red",   1.0f);
        createColorSlider("TINT_P3_G", "P3 Green", 0.25f);
        createColorSlider("TINT_P3_B", "P3 Blue",  0.25f);
        createColorSlider("TINT_P4_R", "P4 Red",   1.0f);
        createColorSlider("TINT_P4_G", "P4 Green", 1.0f);
        createColorSlider("TINT_P4_B", "P4 Blue",  0.0f);

        m_thread = CreateThread(NULL, 0, tintThread, this, 0, NULL);
    }

    ~BallTintMod() {
        m_running = false;
        if (m_thread) {
            WaitForSingleObject(m_thread, 1000);
            CloseHandle(m_thread);
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new BallTintMod();
}
