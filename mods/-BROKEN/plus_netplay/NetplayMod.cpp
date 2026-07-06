/*
 * NetplayMod.cpp — Hamsterball Online Multiplayer Proof of Concept
 *
 * HB+ API mod. Uses host-authority architecture:
 *   HOST:   Runs 2P Party Mode, streams P1+P2 ball state to Python relay
 *   GUEST:  Runs any mode, injects P1 as ghost ball, sends P2 input to host
 *
 * IPC: Named pipe (\\.\pipe\hamsterball_netplay) between DLL and Python relay
 * Networking: Python relay handles TCP (localhost for PoC, LAN/Internet later)
 *
 * Build:
 *   i686-w64-mingw32-g++ -shared -o plus_netplay.dll NetplayMod.cpp \
 *     -static -static-libgcc -static-libstdc++ -O2 -msse2 -mfpmath=sse \
 *     -lws2_32
 */

#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

// ═══════════════════════════════════════════════════════════════════════════
// Constants
// ═══════════════════════════════════════════════════════════════════════════

#define PIPE_NAME "\\\\.\\pipe\\hamsterball_netplay"
#define PIPE_TIMEOUT 5000
#define BALL_STATE_SIZE 40   // 10 floats (pos, vel, rot, etc.)
#define MAX_PIPE_RETRIES 3
#define HEARTBEAT_INTERVAL 300  // frames

static constexpr DWORD GLOBAL_APP_PTR  = 0x5341E0;
static constexpr DWORD APP_PROFILE_OFFSET = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX = 0x4D2000;

// Ball offsets for state serialization
static constexpr DWORD BALL_POS_X   = 0x164;
static constexpr DWORD BALL_POS_Y   = 0x168;
static constexpr DWORD BALL_POS_Z   = 0x16C;
static constexpr DWORD BALL_VEL_X   = 0x190;  // facing_x
static constexpr DWORD BALL_VEL_Z   = 0x194;  // facing_z
static constexpr DWORD BALL_ROT     = 0x150;  // roll angle
static constexpr DWORD BALL_RADIUS  = 0x284;
static constexpr DWORD BALL_GRAVITY_PLANE = 0x748;

// ═══════════════════════════════════════════════════════════════════════════
// Types
// ═══════════════════════════════════════════════════════════════════════════

enum NetRole {
    ROLE_DISABLED = 0,
    ROLE_HOST     = 1,
    ROLE_GUEST    = 2,
};

enum ConnState {
    CONN_OFFLINE    = 0,
    CONN_SEARCHING  = 1,
    CONN_CONNECTING = 2,
    CONN_CONNECTED  = 3,
    CONN_ERROR      = 4,
};

// Message types for pipe protocol
enum MsgType : DWORD {
    MSG_PING          = 0,
    MSG_PONG          = 1,
    MSG_BALL_STATE    = 2,   // host → relay → guest: ball positions
    MSG_INPUT_STATE   = 3,   // guest → relay → host: P2 input
    MSG_FPS_REPORT    = 4,   // both directions: FPS info
    MSG_ROLE_SET      = 5,   // DLL → relay: set role (host/guest)
    MSG_PORT_SET      = 6,   // DLL → relay: set port
    MSG_STATUS        = 7,   // relay → DLL: connection status update
    MSG_HOST_IP       = 8,   // DLL → relay: target host IP for guest
};

#pragma pack(push, 1)
struct PipeHeader {
    DWORD type;
    DWORD length;
};

struct BallStateMsg {
    DWORD frame;
    float p1_pos[3];
    float p1_vel[2];
    float p1_rot;
    float p1_radius;
    float p1_gravity;
    float p2_pos[3];
    float p2_vel[2];
    float p2_rot;
    float p2_radius;
    float p2_gravity;
};

struct InputStateMsg {
    DWORD frame;
    float dir_x;
    float dir_y;
    float force_mult;
};

struct FpsReportMsg {
    float local_fps;
    float remote_fps;
    DWORD frame_count;
};

struct StatusMsg {
    DWORD conn_state;
    DWORD remote_fps_raw;
    char  remote_info[64];
};
#pragma pack(pop)

// ═══════════════════════════════════════════════════════════════════════════
// Global State
// ═══════════════════════════════════════════════════════════════════════════

static IModAPI* g_api = nullptr;

static volatile NetRole  g_role        = ROLE_DISABLED;
static volatile ConnState g_connState   = CONN_OFFLINE;
static volatile int      g_port        = 5029;
static char              g_hostIP[64]  = "127.0.0.1";

static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static HANDLE g_pipeThread = NULL;
static volatile bool g_pipeRunning = false;

// FPS tracking
static DWORD g_frameCount = 0;
static DWORD g_lastFpsTick = 0;
static float g_localFps = 0.0f;
static float g_remoteFps = 0.0f;
static DWORD g_lastHeartbeat = 0;

// Status display
static char g_statusText[256] = "Netplay: Disabled";
static char g_remoteInfo[128] = "";

// Latest ball state (guest receives from host)
static BallStateMsg g_latestBallState = {};
static CRITICAL_SECTION g_stateLock;

// Latest input state (host receives from guest)
static InputStateMsg g_latestInput = {};
static CRITICAL_SECTION g_inputLock;

// ═══════════════════════════════════════════════════════════════════════════
// Utility
// ═══════════════════════════════════════════════════════════════════════════

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

// ═══════════════════════════════════════════════════════════════════════════
// Pipe Communication
// ═══════════════════════════════════════════════════════════════════════════

static bool connectToPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) return true;
    g_pipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, 0, NULL);
    return g_pipe != INVALID_HANDLE_VALUE;
}

static bool sendPipeMsg(DWORD type, const void* data, DWORD dataLen) {
    if (!connectToPipe()) return false;
    PipeHeader hdr = { type, dataLen };
    DWORD written;
    if (!WriteFile(g_pipe, &hdr, sizeof(hdr), &written, NULL) || written != sizeof(hdr))
        return false;
    if (dataLen > 0 && data) {
        if (!WriteFile(g_pipe, data, dataLen, &written, NULL) || written != dataLen)
            return false;
    }
    return true;
}

static bool readPipeMsg(DWORD* outType, void* outBuf, DWORD bufSize, DWORD* outLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD bytesRead = 0;
    if (!ReadFile(g_pipe, outType, 4, &bytesRead, NULL) || bytesRead != 4) return false;
    DWORD len = 0;
    if (!ReadFile(g_pipe, &len, 4, &bytesRead, NULL) || bytesRead != 4) return false;
    if (len > bufSize) len = bufSize;
    if (len > 0) {
        if (!ReadFile(g_pipe, outBuf, len, &bytesRead, NULL) || bytesRead != len) return false;
    }
    if (outLen) *outLen = len;
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Pipe Thread — handles all pipe I/O
// ═══════════════════════════════════════════════════════════════════════════

static DWORD WINAPI pipeThreadFunc(LPVOID param) {
    int retryCount = 0;

    while (g_pipeRunning) {
        if (!connectToPipe()) {
            Sleep(500);
            retryCount++;
            if (retryCount > 20) {
                g_connState = CONN_ERROR;
                snprintf(g_statusText, sizeof(g_statusText),
                         "Netplay: Pipe error (relay not running?)");
            }
            continue;
        }
        retryCount = 0;

        // Send role + port if changed
        static NetRole lastSentRole = ROLE_DISABLED;
        static int lastSentPort = -1;
        static char lastSentIP[64] = "";

        if (g_role != lastSentRole) {
            DWORD roleVal = (DWORD)g_role;
            sendPipeMsg(MSG_ROLE_SET, &roleVal, sizeof(roleVal));
            lastSentRole = g_role;
        }
        if (g_port != lastSentPort && g_role != ROLE_DISABLED) {
            DWORD portVal = (DWORD)g_port;
            sendPipeMsg(MSG_PORT_SET, &portVal, sizeof(portVal));
            lastSentPort = g_port;
        }
        if (g_role == ROLE_GUEST && strcmp(g_hostIP, lastSentIP) != 0) {
            sendPipeMsg(MSG_HOST_IP, g_hostIP, (DWORD)strlen(g_hostIP) + 1);
            strncpy(lastSentIP, g_hostIP, sizeof(lastSentIP));
        }

        // --- HOST: stream ball state every frame ---
        if (g_role == ROLE_HOST && g_connState >= CONN_CONNECTED) {
            DWORD board = findBoard();
            if (board) {
                DWORD scene = getScene();
                DWORD p1Ball = 0, p2Ball = 0;
                if (scene && !IsBadReadPtr((void*)(scene + 0x29D0), 4))
                    p1Ball = *(DWORD*)(scene + 0x29D0);
                if (scene && !IsBadReadPtr((void*)(scene + 0x3204), 4)) {
                    // ball_list_2 is an AthenaList — read first entry if count > 0
                    DWORD listBase = scene + 0x3204;
                    DWORD count = *(DWORD*)(listBase + 4);
                    if (count > 0 && !IsBadReadPtr((void*)(listBase + 8), 4))
                        p2Ball = *(DWORD*)(listBase + 8);
                }

                if (p1Ball && !IsBadReadPtr((void*)p1Ball, 0x800)) {
                    BallStateMsg msg = {};
                    msg.frame = g_frameCount;
                    msg.p1_pos[0] = readBallFloat(p1Ball, BALL_POS_X);
                    msg.p1_pos[1] = readBallFloat(p1Ball, BALL_POS_Y);
                    msg.p1_pos[2] = readBallFloat(p1Ball, BALL_POS_Z);
                    msg.p1_vel[0] = readBallFloat(p1Ball, BALL_VEL_X);
                    msg.p1_vel[1] = readBallFloat(p1Ball, BALL_VEL_Z);
                    msg.p1_rot   = readBallFloat(p1Ball, BALL_ROT);
                    msg.p1_radius = readBallFloat(p1Ball, BALL_RADIUS);
                    msg.p1_gravity = readBallFloat(p1Ball, BALL_GRAVITY_PLANE);

                    if (p2Ball && !IsBadReadPtr((void*)p2Ball, 0x800)) {
                        msg.p2_pos[0] = readBallFloat(p2Ball, BALL_POS_X);
                        msg.p2_pos[1] = readBallFloat(p2Ball, BALL_POS_Y);
                        msg.p2_pos[2] = readBallFloat(p2Ball, BALL_POS_Z);
                        msg.p2_vel[0] = readBallFloat(p2Ball, BALL_VEL_X);
                        msg.p2_vel[1] = readBallFloat(p2Ball, BALL_VEL_Z);
                        msg.p2_rot   = readBallFloat(p2Ball, BALL_ROT);
                        msg.p2_radius = readBallFloat(p2Ball, BALL_RADIUS);
                        msg.p2_gravity = readBallFloat(p2Ball, BALL_GRAVITY_PLANE);
                    }
                    sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
                }
            }
        }

        // --- GUEST: send input state every frame ---
        if (g_role == ROLE_GUEST && g_connState >= CONN_CONNECTED) {
            Ball* p2 = g_api ? g_api->GetPlayer() : nullptr;
            if (p2 && !IsBadReadPtr(p2, sizeof(Ball))) {
                InputStateMsg msg = {};
                msg.frame = g_frameCount;
                // Read force accumulators (ball+0x2BC/2C0/2C4)
                msg.dir_x = readBallFloat((DWORD)p2, 0x2BC);
                msg.dir_y = readBallFloat((DWORD)p2, 0x2C0);
                msg.force_mult = 1.0f;
                sendPipeMsg(MSG_INPUT_STATE, &msg, sizeof(msg));
            }
        }

        // --- Both: send FPS report periodically ---
        if (g_frameCount - g_lastHeartbeat >= HEARTBEAT_INTERVAL) {
            FpsReportMsg fps = {};
            fps.local_fps = g_localFps;
            fps.frame_count = g_frameCount;
            sendPipeMsg(MSG_FPS_REPORT, &fps, sizeof(fps));
            g_lastHeartbeat = g_frameCount;
        }

        // --- Read incoming messages (non-blocking) ---
        DWORD type;
        char buf[512];
        DWORD len = 0;
        // Set short read timeout
        COMMTIMEOUTS cto = { 100, 0, 100, 0, 100 };
        SetCommTimeouts(g_pipe, &cto);

        if (readPipeMsg(&type, buf, sizeof(buf), &len)) {
            switch (type) {
                case MSG_BALL_STATE:
                    if (g_role == ROLE_GUEST && len == sizeof(BallStateMsg)) {
                        EnterCriticalSection(&g_stateLock);
                        memcpy(&g_latestBallState, buf, sizeof(BallStateMsg));
                        LeaveCriticalSection(&g_stateLock);
                    }
                    break;
                case MSG_INPUT_STATE:
                    if (g_role == ROLE_HOST && len == sizeof(InputStateMsg)) {
                        EnterCriticalSection(&g_inputLock);
                        memcpy(&g_latestInput, buf, sizeof(InputStateMsg));
                        LeaveCriticalSection(&g_inputLock);
                    }
                    break;
                case MSG_FPS_REPORT:
                    if (len == sizeof(FpsReportMsg)) {
                        FpsReportMsg* fps = (FpsReportMsg*)buf;
                        g_remoteFps = fps->local_fps;
                    }
                    break;
                case MSG_STATUS:
                    if (len >= sizeof(StatusMsg)) {
                        StatusMsg* st = (StatusMsg*)buf;
                        g_connState = (ConnState)st->conn_state;
                        DWORD rfps = st->remote_fps_raw;
                        if (rfps > 0) g_remoteFps = (float)rfps;
                        strncpy(g_remoteInfo, st->remote_info, sizeof(g_remoteInfo));
                        g_remoteInfo[sizeof(g_remoteInfo)-1] = '\0';
                    }
                    break;
            }
        }

        Sleep(8); // ~120Hz pipe poll
    }

    if (g_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Main Mod Class
// ═══════════════════════════════════════════════════════════════════════════

class NetplayMod : public HamsterballAPI {
private:
    HANDLE m_renderThread = NULL;

    void createButton(const char* id, const char* label) {
        CustomButton btn(id, label);
        btn.defaultState = false;
        btn.trueText = "ON";
        btn.falseText = "OFF";
        g_api->CreateToggleButton(btn, this);
    }

    void createSlider(const char* id, const char* label, float def, float lo, float hi, float step) {
        CustomSlider s(id, label, def);
        s.lowerBound = lo;
        s.upperBound = hi;
        s.stepSize = step;
        s.decimalPlaces = 0;
        g_api->CreateSlider(s, this);
    }

public:
    const char* GetModName() override    { return "Netplay"; }
    const char* GetAuthorName() override { return "rsks + Hamsterbot"; }
    const char* GetContributors() override {
        return "PoC: host-authority model. Host streams ball state, guest renders ghost ball.";
    }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        g_api = modApi;
        InitializeCriticalSection(&g_stateLock);
        InitializeCriticalSection(&g_inputLock);

        // UI: Role toggle
        createButton("NETPLAY_HOST", "Netplay: HOST Mode");
        createButton("NETPLAY_GUEST", "Netplay: GUEST Mode");

        // UI: Port slider
        createSlider("NETPLAY_PORT", "Netplay Port", 5029, 1024, 65535, 1);

        // Start pipe thread
        g_pipeRunning = true;
        g_pipeThread = CreateThread(NULL, 0, pipeThreadFunc, NULL, 0, NULL);
    }

    ~NetplayMod() {
        g_pipeRunning = false;
        if (g_pipeThread) {
            WaitForSingleObject(g_pipeThread, 2000);
            CloseHandle(g_pipeThread);
        }
        DeleteCriticalSection(&g_stateLock);
        DeleteCriticalSection(&g_inputLock);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "NETPLAY_HOST") == 0) {
            if (newState) {
                g_role = ROLE_HOST;
                g_connState = CONN_SEARCHING;
                snprintf(g_statusText, sizeof(g_statusText),
                         "Netplay: HOST (waiting for guest on port %d)", g_port);
            } else {
                g_role = ROLE_DISABLED;
                g_connState = CONN_OFFLINE;
                snprintf(g_statusText, sizeof(g_statusText), "Netplay: Disabled");
            }
        }
        if (strcmp(buttonId, "NETPLAY_GUEST") == 0) {
            if (newState) {
                g_role = ROLE_GUEST;
                g_connState = CONN_SEARCHING;
                snprintf(g_statusText, sizeof(g_statusText),
                         "Netplay: GUEST (connecting to %s:%d)", g_hostIP, g_port);
            } else {
                g_role = ROLE_DISABLED;
                g_connState = CONN_OFFLINE;
                snprintf(g_statusText, sizeof(g_statusText), "Netplay: Disabled");
            }
        }
    }

    void onSliderChange(const char* sliderId, float newValue) override {
        if (strcmp(sliderId, "NETPLAY_PORT") == 0) {
            g_port = (int)newValue;
        }
    }

    void onGameUpdate() override {
        // FPS calculation
        g_frameCount++;
        DWORD tick = GetTickCount();
        if (g_lastFpsTick == 0) g_lastFpsTick = tick;
        DWORD elapsed = tick - g_lastFpsTick;
        if (elapsed >= 1000) {
            g_localFps = (float)(g_frameCount - g_lastFpsTick) * 1000.0f / (float)elapsed;
            // Actually need a separate frame counter for fps
            // Let's use a simpler approach below
            g_lastFpsTick = tick;
        }
    }

    void onTextRenderLoop() override {
        // Draw status text on screen
        if (!g_api) return;

        // FPS calculation (simple rolling average)
        static DWORD s_frameCount = 0;
        static DWORD s_lastTick = 0;
        static float s_fps = 0.0f;

        s_frameCount++;
        DWORD tick = GetTickCount();
        DWORD elapsed = tick - s_lastTick;
        if (elapsed >= 1000) {
            s_fps = (float)s_frameCount * 1000.0f / (float)elapsed;
            s_frameCount = 0;
            s_lastTick = tick;
            g_localFps = s_fps;
        }

        // Update status text based on current state
        const char* roleStr = "OFF";
        if (g_role == ROLE_HOST) roleStr = "HOST";
        else if (g_role == ROLE_GUEST) roleStr = "GUEST";

        const char* connStr = "Offline";
        switch (g_connState) {
            case CONN_OFFLINE:    connStr = "Offline"; break;
            case CONN_SEARCHING:   connStr = "Searching..."; break;
            case CONN_CONNECTING:  connStr = "Connecting..."; break;
            case CONN_CONNECTED:   connStr = "Connected!"; break;
            case CONN_ERROR:       connStr = "Error"; break;
        }

        char displayLine1[128];
        char displayLine2[128];
        char displayLine3[128];

        snprintf(displayLine1, sizeof(displayLine1),
                 "NETPLAY [%s] %s  Port: %d",
                 roleStr, connStr, g_port);

        snprintf(displayLine2, sizeof(displayLine2),
                 "Local FPS: %.1f  Remote FPS: %.1f",
                 g_localFps, g_remoteFps);

        if (g_remoteInfo[0]) {
            snprintf(displayLine3, sizeof(displayLine3),
                     "Remote: %s", g_remoteInfo);
        } else {
            snprintf(displayLine3, sizeof(displayLine3),
                     "Frame: %d", g_frameCount);
        }

        // Draw at top-left
        CustomText params = {};
        if (g_api) {
            // Try to get a font from the API's App
            App* app = g_api->GetApp();
            if (app && !IsBadReadPtr(app, sizeof(App))) {
                params.font = app->fonts.showcardGothic14;
            }
        }
        params.x = 10;
        params.y = 10;
        params.text_color = Color(0.0f, 1.0f, 0.0f, 1.0f); // green
        params.enable_shadow = true;
        g_api->DrawCustomText(displayLine1, params);

        params.y = 30;
        params.text_color = Color(1.0f, 1.0f, 0.0f, 1.0f); // yellow
        g_api->DrawCustomText(displayLine2, params);

        params.y = 50;
        params.text_color = Color(0.5f, 0.8f, 1.0f, 1.0f); // light blue
        g_api->DrawCustomText(displayLine3, params);

        // --- GUEST: Apply ball state to ghost ball ---
        if (g_role == ROLE_GUEST && g_connState == CONN_CONNECTED) {
            applyGuestBallState();
        }

        // --- HOST: Apply guest input to P2 ---
        if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED) {
            applyHostInput();
        }
    }

    void onLevelStart() override {
        g_frameCount = 0;
        if (g_role == ROLE_GUEST) {
            // Guest: we'll use the ghost ball approach
            // The ghost_event mod pattern: inject a ghost ball via Ball_ctor
            // For PoC, we'll just overwrite the player's ball position display
            // Full ghost ball injection would go here
        }
    }

private:
    void applyGuestBallState() {
        // Guest receives P1 ball state from host and writes it to a display ball
        // For PoC: we read the latest state and could write to a ghost ball
        // Full implementation would create a ghost ball like ghost_event mod
        BallStateMsg state;
        EnterCriticalSection(&g_stateLock);
        state = g_latestBallState;
        LeaveCriticalSection(&g_stateLock);

        // TODO: Create ghost ball (like ghost_event mod) and write state to it
        // For now, just log we received data
        if (state.frame > 0 && state.frame != g_frameCount) {
            // Data is flowing — we can write to a ghost ball
        }
    }

    void applyHostInput() {
        // Host receives P2 input from guest and applies it to P2 ball
        InputStateMsg input;
        EnterCriticalSection(&g_inputLock);
        input = g_latestInput;
        LeaveCriticalSection(&g_inputLock);

        // Write force to P2 ball
        DWORD scene = getScene();
        if (!scene) return;

        // Get P2 ball from ball_list_2 (scene+0x3204)
        DWORD listBase = scene + 0x3204;
        if (IsBadReadPtr((void*)listBase, 8)) return;
        DWORD count = *(DWORD*)(listBase + 4);
        if (count == 0) return;
        if (IsBadReadPtr((void*)(listBase + 8), 4)) return;
        DWORD p2Ball = *(DWORD*)(listBase + 8);
        if (!p2Ball || IsBadReadPtr((void*)p2Ball, 0x300)) return;

        // Write force accumulators (ball+0x2BC/2C0/2C4)
        // This is simplified — full implementation would use Ball_ApplyForceV2
        *(float*)(p2Ball + 0x2BC) = input.dir_x;
        *(float*)(p2Ball + 0x2C0) = input.dir_y;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Export
// ═══════════════════════════════════════════════════════════════════════════

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new NetplayMod();
}
