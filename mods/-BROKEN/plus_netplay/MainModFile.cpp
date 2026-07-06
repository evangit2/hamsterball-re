#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════════════════
// Constants
// ═══════════════════════════════════════════════════════════════════════════

#define PIPE_NAME "\\\\.\\pipe\\hamsterball_netplay"
#define HEARTBEAT_INTERVAL 300
#define PIPE_POLL_MS 4

// Ball offsets
static constexpr DWORD BALL_POS_X = 0x164;
static constexpr DWORD BALL_POS_Y = 0x168;
static constexpr DWORD BALL_POS_Z = 0x16C;
static constexpr DWORD BALL_FACING_X = 0x190;
static constexpr DWORD BALL_FACING_Z = 0x194;
static constexpr DWORD BALL_ROT = 0x150;
static constexpr DWORD BALL_RADIUS = 0x284;
static constexpr DWORD BALL_GRAVITY_PLANE = 0x748;
static constexpr DWORD BALL_FORCE_X = 0x2BC;
static constexpr DWORD BALL_FORCE_Y = 0x2C0;
static constexpr DWORD BALL_FORCE_Z = 0x2C4;
static constexpr DWORD BALL_PHYS_PTR = 0x1A4;
static constexpr DWORD PHYS_VEL_X = 0xCA4;
static constexpr DWORD PHYS_VEL_Y = 0xCA8;
static constexpr DWORD PHYS_VEL_Z = 0xCAC;
static constexpr DWORD SCENE_P1_BALL_PTR = 0x29D0;
static constexpr DWORD SCENE_P2_BALL_LIST = 0x3204;
static constexpr DWORD GLOBAL_APP_PTR = 0x5341E0;
static constexpr DWORD APP_PROFILE_OFFSET = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;

enum NetRole { ROLE_DISABLED = 0, ROLE_HOST = 1, ROLE_GUEST = 2 };
enum ConnState { CONN_OFFLINE = 0, CONN_SEARCHING = 1, CONN_CONNECTING = 2, CONN_CONNECTED = 3, CONN_ERROR = 4 };
enum MsgType : DWORD { MSG_BALL_STATE = 2, MSG_INPUT_STATE = 3, MSG_FPS_REPORT = 4, MSG_ROLE_SET = 5, MSG_PORT_SET = 6, MSG_STATUS = 7, MSG_HOST_IP = 8 };

#pragma pack(push, 1)
struct BallStateMsg {
    DWORD frame;
    float p1_pos[3], p1_vel[3], p1_facing[2], p1_rot, p1_radius, p1_gravity;
    float p2_pos[3], p2_vel[3], p2_facing[2], p2_rot, p2_radius, p2_gravity;
};
struct InputStateMsg { DWORD frame; float force_x, force_y, force_z; };
struct FpsReportMsg { float local_fps, remote_fps; DWORD frame_count; };
struct StatusMsg { DWORD conn_state; DWORD remote_fps_raw; char remote_info[64]; };
#pragma pack(pop)

// Globals
static IModAPI* g_api = nullptr;
static volatile NetRole g_role = ROLE_DISABLED;
static volatile ConnState g_connState = CONN_OFFLINE;
static volatile int g_port = 5029;
static volatile int g_ip_octet[4] = { 127, 0, 0, 1 };
static char g_hostIP[32] = "127.0.0.1";
static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static HANDLE g_pipeThread = NULL;
static volatile bool g_pipeRunning = false;
static DWORD g_frameCount = 0, g_fpsFrameCounter = 0, g_fpsLastTick = 0, g_lastHeartbeat = 0;
static float g_localFps = 0.0f, g_remoteFps = 0.0f;
static char g_remoteInfo[128] = "";
static BallStateMsg g_latestBallState = {};
static CRITICAL_SECTION g_stateLock;
static InputStateMsg g_latestInput = {};
static CRITICAL_SECTION g_inputLock;
static volatile bool g_gameReady = false;
static volatile DWORD g_lastBallStateFrame = 0, g_lastInputFrame = 0;

static DWORD findBoard() {
    DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
    if (!appPtr || appPtr < 0x10000) return 0;
    if (IsBadReadPtr((void*)(appPtr + APP_PROFILE_OFFSET), 4)) return 0;
    DWORD profile = *(DWORD*)(appPtr + APP_PROFILE_OFFSET);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 4)) return 0;
    DWORD vtable = *(DWORD*)board;
    if (vtable < BOARD_VTABLE_MIN || vtable > BOARD_VTABLE_MAX) return 0;
    return board;
}
static DWORD getScene() {
    DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
    if (!appPtr || appPtr < 0x10000) return 0;
    if (IsBadReadPtr((void*)(appPtr + 0x0C), 4)) return 0;
    DWORD scene = *(DWORD*)(appPtr + 0x0C);
    if (!scene || scene < 0x10000) return 0;
    return scene;
}
static float readBallFloat(DWORD ball, DWORD offset) {
    if (!ball || IsBadReadPtr((void*)(ball + offset), 4)) return 0.0f;
    return *(float*)(ball + offset);
}
static void readPhysicsVelocity(DWORD ball, float* vx, float* vy, float* vz) {
    *vx = *vy = *vz = 0.0f;
    if (!ball) return;
    DWORD physPtr = readBallFloat(ball, BALL_PHYS_PTR) ? *(DWORD*)(ball + BALL_PHYS_PTR) : 0;
    if (!physPtr || physPtr < 0x10000) return;
    if (IsBadReadPtr((void*)(physPtr + PHYS_VEL_Z), 4)) return;
    *vx = *(float*)(physPtr + PHYS_VEL_X);
    *vy = *(float*)(physPtr + PHYS_VEL_Y);
    *vz = *(float*)(physPtr + PHYS_VEL_Z);
}
static void updateHostIPString() {
    sprintf_s(g_hostIP, sizeof(g_hostIP), "%d.%d.%d.%d", g_ip_octet[0], g_ip_octet[1], g_ip_octet[2], g_ip_octet[3]);
}
static bool connectToPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) return true;
    g_pipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD mode = PIPE_READMODE_BYTE;
    SetNamedPipeHandleState(g_pipe, &mode, NULL, NULL);
    return true;
}
static void disconnectPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) { CloseHandle(g_pipe); g_pipe = INVALID_HANDLE_VALUE; }
}
static bool sendPipeMsg(DWORD type, const void* data, DWORD dataLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD hdr[2] = { type, dataLen };
    DWORD written;
    if (!WriteFile(g_pipe, hdr, 8, &written, NULL) || written != 8) { disconnectPipe(); return false; }
    if (dataLen > 0 && data) {
        if (!WriteFile(g_pipe, data, dataLen, &written, NULL) || written != dataLen) { disconnectPipe(); return false; }
    }
    return true;
}
static bool readPipeMsgNonBlocking(DWORD* outType, void* outBuf, DWORD bufSize, DWORD* outLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(g_pipe, NULL, 0, NULL, &bytesAvailable, NULL)) { disconnectPipe(); return false; }
    if (bytesAvailable < 8) return false;
    DWORD bytesRead = 0;
    if (!ReadFile(g_pipe, outType, 4, &bytesRead, NULL) || bytesRead != 4) { disconnectPipe(); return false; }
    DWORD len = 0;
    if (!ReadFile(g_pipe, &len, 4, &bytesRead, NULL) || bytesRead != 4) { disconnectPipe(); return false; }
    if (len > bufSize) len = bufSize;
    if (len > 0) { if (!ReadFile(g_pipe, outBuf, len, &bytesRead, NULL) || bytesRead != len) { disconnectPipe(); return false; } }
    if (outLen) *outLen = len;
    return true;
}

static DWORD WINAPI pipeThreadFunc(LPVOID) {
    NetRole lastSentRole = ROLE_DISABLED;
    int lastSentPort = -1;
    char lastSentIP[32] = "";
    while (g_pipeRunning) {
        if (!connectToPipe()) { Sleep(500); continue; }
        if (g_role != lastSentRole) { DWORD r = (DWORD)g_role; sendPipeMsg(MSG_ROLE_SET, &r, 4); lastSentRole = g_role; }
        if (g_port != lastSentPort && g_role != ROLE_DISABLED) { DWORD p = (DWORD)g_port; sendPipeMsg(MSG_PORT_SET, &p, 4); lastSentPort = g_port; }
        updateHostIPString();
        if (g_role == ROLE_GUEST && strcmp(g_hostIP, lastSentIP) != 0) { sendPipeMsg(MSG_HOST_IP, g_hostIP, (DWORD)strlen(g_hostIP)+1); strcpy_s(lastSentIP, g_hostIP); }
        if (g_role == ROLE_HOST && g_connState >= CONN_CONNECTED && g_gameReady) {
            DWORD scene = getScene();
            if (scene) {
                DWORD p1 = 0, p2 = 0;
                if (!IsBadReadPtr((void*)(scene + SCENE_P1_BALL_PTR), 4)) p1 = *(DWORD*)(scene + SCENE_P1_BALL_PTR);
                if (!IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) { DWORD lb = scene + SCENE_P2_BALL_LIST; DWORD c = *(DWORD*)(lb+4); if (c > 0 && !IsBadReadPtr((void*)(lb+8), 4)) p2 = *(DWORD*)(lb+8); }
                if (p1 && !IsBadReadPtr((void*)p1, 0x800)) {
                    BallStateMsg msg = {}; msg.frame = g_frameCount;
                    msg.p1_pos[0]=readBallFloat(p1,BALL_POS_X); msg.p1_pos[1]=readBallFloat(p1,BALL_POS_Y); msg.p1_pos[2]=readBallFloat(p1,BALL_POS_Z);
                    readPhysicsVelocity(p1, &msg.p1_vel[0], &msg.p1_vel[1], &msg.p1_vel[2]);
                    msg.p1_facing[0]=readBallFloat(p1,BALL_FACING_X); msg.p1_facing[1]=readBallFloat(p1,BALL_FACING_Z);
                    msg.p1_rot=readBallFloat(p1,BALL_ROT); msg.p1_radius=readBallFloat(p1,BALL_RADIUS); msg.p1_gravity=readBallFloat(p1,BALL_GRAVITY_PLANE);
                    if (p2 && !IsBadReadPtr((void*)p2, 0x800)) {
                        msg.p2_pos[0]=readBallFloat(p2,BALL_POS_X); msg.p2_pos[1]=readBallFloat(p2,BALL_POS_Y); msg.p2_pos[2]=readBallFloat(p2,BALL_POS_Z);
                        readPhysicsVelocity(p2, &msg.p2_vel[0], &msg.p2_vel[1], &msg.p2_vel[2]);
                        msg.p2_facing[0]=readBallFloat(p2,BALL_FACING_X); msg.p2_facing[1]=readBallFloat(p2,BALL_FACING_Z);
                        msg.p2_rot=readBallFloat(p2,BALL_ROT); msg.p2_radius=readBallFloat(p2,BALL_RADIUS); msg.p2_gravity=readBallFloat(p2,BALL_GRAVITY_PLANE);
                    }
                    sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
                }
            }
        }
        if (g_role == ROLE_GUEST && g_connState >= CONN_CONNECTED && g_gameReady) {
            Ball* p2 = g_api ? g_api->GetPlayer() : nullptr;
            if (p2 && !IsBadReadPtr(p2, 0x300)) {
                InputStateMsg msg = {}; msg.frame = g_frameCount;
                msg.force_x=readBallFloat((DWORD)p2,BALL_FORCE_X); msg.force_y=readBallFloat((DWORD)p2,BALL_FORCE_Y); msg.force_z=readBallFloat((DWORD)p2,BALL_FORCE_Z);
                sendPipeMsg(MSG_INPUT_STATE, &msg, sizeof(msg));
            }
        }
        if (g_frameCount - g_lastHeartbeat >= HEARTBEAT_INTERVAL) { FpsReportMsg fps = {g_localFps, g_remoteFps, g_frameCount}; sendPipeMsg(MSG_FPS_REPORT, &fps, sizeof(fps)); g_lastHeartbeat = g_frameCount; }
        DWORD type; char buf[512]; DWORD len = 0;
        while (readPipeMsgNonBlocking(&type, buf, sizeof(buf), &len)) {
            switch (type) {
                case MSG_BALL_STATE: if (g_role == ROLE_GUEST && len == sizeof(BallStateMsg)) { EnterCriticalSection(&g_stateLock); memcpy(&g_latestBallState, buf, sizeof(BallStateMsg)); g_lastBallStateFrame = g_frameCount; LeaveCriticalSection(&g_stateLock); } break;
                case MSG_INPUT_STATE: if (g_role == ROLE_HOST && len == sizeof(InputStateMsg)) { EnterCriticalSection(&g_inputLock); memcpy(&g_latestInput, buf, sizeof(InputStateMsg)); g_lastInputFrame = g_frameCount; LeaveCriticalSection(&g_inputLock); } break;
                case MSG_FPS_REPORT: if (len == sizeof(FpsReportMsg)) { FpsReportMsg* f = (FpsReportMsg*)buf; g_remoteFps = f->local_fps; } break;
                case MSG_STATUS: if (len >= sizeof(StatusMsg)) { StatusMsg* s = (StatusMsg*)buf; g_connState = (ConnState)s->conn_state; if (s->remote_fps_raw > 0) g_remoteFps = (float)s->remote_fps_raw; strncpy_s(g_remoteInfo, s->remote_info, sizeof(g_remoteInfo)-1); } break;
            }
        }
        Sleep(PIPE_POLL_MS);
    }
    disconnectPipe();
    return 0;
}

class NetplayMod : public HamsterballAPI {
private:
    void createButton(const char* id, const char* label) {
        CustomButton btn(id, label); btn.defaultState = false; btn.trueText = "ON"; btn.falseText = "OFF";
        g_api->CreateToggleButton(btn, this);
    }
    void createSlider(const char* id, const char* label, float def, float lo, float hi, float step, int dec = 0) {
        CustomSlider s(id, label, def); s.lowerBound = lo; s.upperBound = hi; s.stepSize = step; s.decimalPlaces = dec;
        g_api->CreateSlider(s, this);
    }
    void setRole(NetRole r) { g_role = r; g_connState = (r == ROLE_DISABLED) ? CONN_OFFLINE : CONN_SEARCHING; }
public:
    const char* GetModName() override { return "Netplay"; }
    const char* GetAuthorName() override { return "rsks + Hamsterbot"; }
    const char* GetContributors() override { return "PoC: host-authority model"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        g_api = modApi;
        InitializeCriticalSection(&g_stateLock);
        InitializeCriticalSection(&g_inputLock);
        createButton("NETPLAY_HOST", "Netplay: HOST Mode");
        createButton("NETPLAY_GUEST", "Netplay: GUEST Mode");
        createSlider("NETPLAY_PORT", "Netplay Port", 5029, 1024, 65535, 1);
        createSlider("NETPLAY_IP1", "Host IP: Octet 1", 127, 0, 255, 1);
        createSlider("NETPLAY_IP2", "Host IP: Octet 2", 0, 0, 255, 1);
        createSlider("NETPLAY_IP3", "Host IP: Octet 3", 0, 0, 255, 1);
        createSlider("NETPLAY_IP4", "Host IP: Octet 4", 1, 0, 255, 1);
    }
    ~NetplayMod() {
        g_pipeRunning = false;
        if (g_pipeThread) { WaitForSingleObject(g_pipeThread, 2000); CloseHandle(g_pipeThread); }
        DeleteCriticalSection(&g_stateLock);
        DeleteCriticalSection(&g_inputLock);
    }
    void onButtonToggle(const char* id, bool newState) override {
        if (strcmp(id, "NETPLAY_HOST") == 0) { if (newState) setRole(ROLE_HOST); else if (g_role == ROLE_HOST) setRole(ROLE_DISABLED); }
        if (strcmp(id, "NETPLAY_GUEST") == 0) { if (newState) setRole(ROLE_GUEST); else if (g_role == ROLE_GUEST) setRole(ROLE_DISABLED); }
    }
    void onSliderChange(const char* id, float val) override {
        if (strcmp(id, "NETPLAY_PORT") == 0) g_port = (int)val;
        if (strcmp(id, "NETPLAY_IP1") == 0) g_ip_octet[0] = (int)val;
        if (strcmp(id, "NETPLAY_IP2") == 0) g_ip_octet[1] = (int)val;
        if (strcmp(id, "NETPLAY_IP3") == 0) g_ip_octet[2] = (int)val;
        if (strcmp(id, "NETPLAY_IP4") == 0) g_ip_octet[3] = (int)val;
    }
    void onGameUpdate() override {
        g_frameCount++;
        if (!g_gameReady && g_frameCount > 120) g_gameReady = true;
        if (g_gameReady && !g_pipeThread) { g_pipeRunning = true; g_pipeThread = CreateThread(NULL, 0, pipeThreadFunc, NULL, 0, NULL); }
        g_fpsFrameCounter++;
        DWORD tick = GetTickCount();
        if (g_fpsLastTick == 0) g_fpsLastTick = tick;
        DWORD elapsed = tick - g_fpsLastTick;
        if (elapsed >= 1000) { g_localFps = (float)g_fpsFrameCounter * 1000.0f / (float)elapsed; g_fpsFrameCounter = 0; g_fpsLastTick = tick; }
    }
    void onTextRenderLoop() override {
        if (!g_api || !g_gameReady) return;
        const char* roleStr = (g_role == ROLE_HOST) ? "HOST" : (g_role == ROLE_GUEST) ? "GUEST" : "OFF";
        const char* connStr = (g_connState == CONN_OFFLINE) ? "Offline" : (g_connState == CONN_SEARCHING) ? "Searching..." : (g_connState == CONN_CONNECTING) ? "Connecting..." : (g_connState == CONN_CONNECTED) ? "Connected!" : "ERROR";
        char l1[128], l2[128], l3[128], l4[128];
        sprintf_s(l1, sizeof(l1), "NETPLAY [%s] %s  Port:%d", roleStr, connStr, g_port);
        sprintf_s(l2, sizeof(l2), "Local FPS:%d  Remote FPS:%d  Frame:%d", (int)g_localFps, (int)g_remoteFps, g_frameCount);
        if (g_role == ROLE_GUEST) { updateHostIPString(); sprintf_s(l3, sizeof(l3), "Host: %s:%d", g_hostIP, g_port); }
        else if (g_remoteInfo[0]) { sprintf_s(l3, sizeof(l3), "Remote: %s", g_remoteInfo); }
        else { sprintf_s(l3, sizeof(l3), "---"); }
        l4[0] = '\0';
        if (g_role == ROLE_GUEST && g_connState == CONN_CONNECTED) { DWORD age = g_frameCount - g_lastBallStateFrame; sprintf_s(l4, sizeof(l4), "Ball data: %s (%d frames ago)", age < 10 ? "LIVE" : "STALE", age); }
        else if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED) { DWORD age = g_frameCount - g_lastInputFrame; sprintf_s(l4, sizeof(l4), "Input: %s (%d frames ago)", g_lastInputFrame > 0 && age < 10 ? "LIVE" : "WAITING", g_lastInputFrame > 0 ? age : 0); }
        CustomText params = {};
        App* app = g_api->GetApp();
        if (app && !IsBadReadPtr(app, sizeof(App))) { void* font = app->fonts.showcardGothic14; if (!font || IsBadReadPtr(font, 4)) return; params.font = font; }
        else return;
        params.x = 10; params.y = 10; params.enable_shadow = true;
        params.text_color = Color(0, 1, 0, 1); g_api->DrawCustomText(l1, params);
        params.y = 28; params.text_color = Color(1, 1, 0, 1); g_api->DrawCustomText(l2, params);
        params.y = 46; params.text_color = Color(0.5f, 0.8f, 1, 1); g_api->DrawCustomText(l3, params);
        if (l4[0]) { params.y = 64; params.text_color = Color(1, 0.5f, 1, 1); g_api->DrawCustomText(l4, params); }
        if (g_role == ROLE_GUEST && g_connState == CONN_CONNECTED && g_gameReady) {
            BallStateMsg state; EnterCriticalSection(&g_stateLock); state = g_latestBallState; LeaveCriticalSection(&g_stateLock);
            if (state.frame > 0 && state.frame != g_frameCount) { /* ghost ball injection would go here */ }
        }
        if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED && g_gameReady) {
            InputStateMsg input; EnterCriticalSection(&g_inputLock); input = g_latestInput; LeaveCriticalSection(&g_inputLock);
            if (input.frame > 0) {
                DWORD scene = getScene(); if (!scene) return;
                if (IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) return;
                DWORD lb = scene + SCENE_P2_BALL_LIST; DWORD c = *(DWORD*)(lb+4); if (c == 0) return;
                if (IsBadReadPtr((void*)(lb+8), 4)) return;
                DWORD p2 = *(DWORD*)(lb+8); if (!p2 || IsBadReadPtr((void*)p2, 0x300)) return;
                if (g_api) g_api->ApplyForce((Ball*)p2, input.force_x, input.force_y, input.force_z, 1.0f);
            }
        }
    }
    void onLevelStart() override { g_frameCount = 0; }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new NetplayMod();
}
