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
 *     -I. -static -static-libgcc -static-libstdc++ -O2 -msse2 -mfpmath=sse \
 *     -lws2_32 -lwinmm -Wl,--enable-stdcall-fixup
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
#define HEARTBEAT_INTERVAL 300  // frames between FPS reports
#define PIPE_POLL_MS 4          // pipe thread sleep between cycles

static constexpr DWORD GLOBAL_APP_PTR       = 0x5341E0;
static constexpr DWORD APP_PROFILE_OFFSET   = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN     = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX     = 0x4D2000;

// Ball offsets for state serialization
static constexpr DWORD BALL_POS_X          = 0x164;
static constexpr DWORD BALL_POS_Y          = 0x168;
static constexpr DWORD BALL_POS_Z          = 0x16C;
static constexpr DWORD BALL_FACING_X       = 0x190;  // direction vector x
static constexpr DWORD BALL_FACING_Z       = 0x194;  // direction vector z
static constexpr DWORD BALL_ROT            = 0x150;  // roll angle
static constexpr DWORD BALL_RADIUS         = 0x284;
static constexpr DWORD BALL_GRAVITY_PLANE  = 0x748;
static constexpr DWORD BALL_FORCE_X        = 0x2BC;  // force accumulator
static constexpr DWORD BALL_FORCE_Y        = 0x2C0;
static constexpr DWORD BALL_FORCE_Z        = 0x2C4;
static constexpr DWORD BALL_PHYS_PTR        = 0x1A4;  // PhysicsObject*

// PhysicsObject velocity offsets
static constexpr DWORD PHYS_VEL_X           = 0xCA4;
static constexpr DWORD PHYS_VEL_Y           = 0xCA8;
static constexpr DWORD PHYS_VEL_Z           = 0xCAC;

// Scene offsets
static constexpr DWORD SCENE_P1_BALL_PTR   = 0x29D0;
static constexpr DWORD SCENE_P2_BALL_LIST   = 0x3204;

// ═══════════════════════════════════════════════════════════════════════════
// Types
// ═══════════════════════════════════════════════════════════════════════════

enum NetRole : int {
    ROLE_DISABLED = 0,
    ROLE_HOST     = 1,
    ROLE_GUEST    = 2,
};

enum ConnState : int {
    CONN_OFFLINE    = 0,
    CONN_SEARCHING  = 1,
    CONN_CONNECTING = 2,
    CONN_CONNECTED  = 3,
    CONN_ERROR      = 4,
};

// Message types for pipe protocol (DLL ↔ relay)
enum MsgType : DWORD {
    MSG_PING          = 0,
    MSG_PONG          = 1,
    MSG_BALL_STATE    = 2,   // host → relay → guest: ball positions
    MSG_INPUT_STATE   = 3,   // guest → relay → host: P2 input force
    MSG_FPS_REPORT    = 4,   // both directions: FPS info
    MSG_ROLE_SET      = 5,   // DLL → relay: set role (host/guest)
    MSG_PORT_SET      = 6,   // DLL → relay: set TCP port
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
    // P1 (host's local player)
    float p1_pos[3];       // x, y, z
    float p1_vel[3];       // actual velocity from PhysicsObject
    float p1_facing[2];    // facing_x, facing_z
    float p1_rot;          // roll angle
    float p1_radius;
    float p1_gravity;
    // P2 (host's remote player, driven by guest input)
    float p2_pos[3];
    float p2_vel[3];
    float p2_facing[2];
    float p2_rot;
    float p2_radius;
    float p2_gravity;
};

struct InputStateMsg {
    DWORD frame;
    float force_x;         // force accumulator x (ball+0x2BC)
    float force_y;         // force accumulator y (ball+0x2C0)
    float force_z;         // force accumulator z (ball+0x2C4)
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

static volatile NetRole   g_role       = ROLE_DISABLED;
static volatile ConnState g_connState  = CONN_OFFLINE;
static volatile int       g_port       = 5029;

// Host IP for guest mode — stored as dotted quad components for slider UI
// We use 4 sliders (octets) since HB+ doesn't have text input
static volatile int g_ip_octet[4] = {127, 0, 0, 1};
static char g_hostIP[32] = "127.0.0.1";

static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static HANDLE g_pipeThread = NULL;
static volatile bool g_pipeRunning = false;

// FPS tracking (single source of truth)
static DWORD g_frameCount = 0;
static DWORD g_fpsFrameCounter = 0;
static DWORD g_fpsLastTick = 0;
static float g_localFps = 0.0f;
static float g_remoteFps = 0.0f;
static DWORD g_lastHeartbeat = 0;

// Status display
static char g_remoteInfo[128] = "";

// Latest ball state (guest receives from host)
static BallStateMsg g_latestBallState = {};
static CRITICAL_SECTION g_stateLock;
static volatile DWORD g_lastBallStateFrame = 0;

// Latest input state (host receives from guest)
static InputStateMsg g_latestInput = {};
static CRITICAL_SECTION g_inputLock;
static volatile DWORD g_lastInputFrame = 0;

// Ghost ball state (guest side)
static DWORD g_ghostBall = 0;  // allocated ghost ball pointer
static bool g_ghostBallInitialized = false;

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

static DWORD readBallDword(DWORD ball, DWORD offset) {
    if (!ball || IsBadReadPtr((void*)(ball + offset), 4)) return 0;
    return *(DWORD*)(ball + offset);
}

static void readPhysicsVelocity(DWORD ball, float* vx, float* vy, float* vz) {
    *vx = *vy = *vz = 0.0f;
    if (!ball) return;
    DWORD physPtr = readBallDword(ball, BALL_PHYS_PTR);
    if (!physPtr || physPtr < 0x10000) return;
    if (IsBadReadPtr((void*)(physPtr + PHYS_VEL_Z), 4)) return;
    *vx = *(float*)(physPtr + PHYS_VEL_X);
    *vy = *(float*)(physPtr + PHYS_VEL_Y);
    *vz = *(float*)(physPtr + PHYS_VEL_Z);
}

static void updateHostIPString() {
    snprintf(g_hostIP, sizeof(g_hostIP), "%d.%d.%d.%d",
             g_ip_octet[0], g_ip_octet[1], g_ip_octet[2], g_ip_octet[3]);
}

// ═══════════════════════════════════════════════════════════════════════════
// Pipe Communication
// ═══════════════════════════════════════════════════════════════════════════

static bool connectToPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) return true;
    g_pipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, 0, NULL);
    if (g_pipe == INVALID_HANDLE_VALUE) return false;

    // Set pipe to message mode for cleaner framing
    DWORD mode = PIPE_READMODE_BYTE;
    SetNamedPipeHandleState(g_pipe, &mode, NULL, NULL);
    return true;
}

static void disconnectPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }
}

static bool sendPipeMsg(DWORD type, const void* data, DWORD dataLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    PipeHeader hdr = { type, dataLen };
    DWORD written;
    if (!WriteFile(g_pipe, &hdr, sizeof(hdr), &written, NULL) || written != sizeof(hdr)) {
        disconnectPipe();
        return false;
    }
    if (dataLen > 0 && data) {
        if (!WriteFile(g_pipe, data, dataLen, &written, NULL) || written != dataLen) {
            disconnectPipe();
            return false;
        }
    }
    return true;
}

// Non-blocking read: peek first, only read if data available
static bool readPipeMsgNonBlocking(DWORD* outType, void* outBuf, DWORD bufSize, DWORD* outLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;

    // Peek to see if we have at least a header (8 bytes)
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(g_pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
        // Pipe broken — relay likely closed
        disconnectPipe();
        return false;
    }
    if (bytesAvailable < sizeof(PipeHeader)) return false;  // not enough data yet

    // Read header
    DWORD bytesRead = 0;
    if (!ReadFile(g_pipe, outType, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe();
        return false;
    }
    DWORD len = 0;
    if (!ReadFile(g_pipe, &len, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe();
        return false;
    }
    if (len > bufSize) len = bufSize;
    if (len > 0) {
        if (!ReadFile(g_pipe, outBuf, len, &bytesRead, NULL) || bytesRead != len) {
            disconnectPipe();
            return false;
        }
    }
    if (outLen) *outLen = len;
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Pipe Thread — handles all pipe I/O (non-blocking)
// ═══════════════════════════════════════════════════════════════════════════

static DWORD WINAPI pipeThreadFunc(LPVOID param) {
    int noPipeCount = 0;
    NetRole lastSentRole = ROLE_DISABLED;
    int lastSentPort = -1;
    char lastSentIP[32] = "";

    while (g_pipeRunning) {
        // Try to connect to pipe
        if (!connectToPipe()) {
            Sleep(500);
            noPipeCount++;
            if (noPipeCount == 10) {
                g_connState = CONN_ERROR;
            }
            continue;
        }
        noPipeCount = 0;

        // ── Send config changes to relay ───────────────────────────
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
        updateHostIPString();
        if (g_role == ROLE_GUEST && strcmp(g_hostIP, lastSentIP) != 0) {
            sendPipeMsg(MSG_HOST_IP, g_hostIP, (DWORD)strlen(g_hostIP) + 1);
            strncpy(lastSentIP, g_hostIP, sizeof(lastSentIP));
            lastSentIP[sizeof(lastSentIP)-1] = '\0';
        }

        // ── HOST: stream ball state every cycle ───────────────────
        if (g_role == ROLE_HOST && g_connState >= CONN_CONNECTED) {
            DWORD scene = getScene();
            if (scene) {
                DWORD p1Ball = 0, p2Ball = 0;
                if (!IsBadReadPtr((void*)(scene + SCENE_P1_BALL_PTR), 4))
                    p1Ball = *(DWORD*)(scene + SCENE_P1_BALL_PTR);

                // P2 ball from AthenaList at scene+0x3204
                if (!IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) {
                    DWORD listBase = scene + SCENE_P2_BALL_LIST;
                    DWORD count = *(DWORD*)(listBase + 4);
                    if (count > 0 && !IsBadReadPtr((void*)(listBase + 8), 4))
                        p2Ball = *(DWORD*)(listBase + 8);
                }

                if (p1Ball && !IsBadReadPtr((void*)p1Ball, 0x800)) {
                    BallStateMsg msg = {};
                    msg.frame = g_frameCount;

                    // P1 state
                    msg.p1_pos[0] = readBallFloat(p1Ball, BALL_POS_X);
                    msg.p1_pos[1] = readBallFloat(p1Ball, BALL_POS_Y);
                    msg.p1_pos[2] = readBallFloat(p1Ball, BALL_POS_Z);
                    readPhysicsVelocity(p1Ball, &msg.p1_vel[0], &msg.p1_vel[1], &msg.p1_vel[2]);
                    msg.p1_facing[0] = readBallFloat(p1Ball, BALL_FACING_X);
                    msg.p1_facing[1] = readBallFloat(p1Ball, BALL_FACING_Z);
                    msg.p1_rot    = readBallFloat(p1Ball, BALL_ROT);
                    msg.p1_radius = readBallFloat(p1Ball, BALL_RADIUS);
                    msg.p1_gravity = readBallFloat(p1Ball, BALL_GRAVITY_PLANE);

                    // P2 state (if exists)
                    if (p2Ball && !IsBadReadPtr((void*)p2Ball, 0x800)) {
                        msg.p2_pos[0] = readBallFloat(p2Ball, BALL_POS_X);
                        msg.p2_pos[1] = readBallFloat(p2Ball, BALL_POS_Y);
                        msg.p2_pos[2] = readBallFloat(p2Ball, BALL_POS_Z);
                        readPhysicsVelocity(p2Ball, &msg.p2_vel[0], &msg.p2_vel[1], &msg.p2_vel[2]);
                        msg.p2_facing[0] = readBallFloat(p2Ball, BALL_FACING_X);
                        msg.p2_facing[1] = readBallFloat(p2Ball, BALL_FACING_Z);
                        msg.p2_rot    = readBallFloat(p2Ball, BALL_ROT);
                        msg.p2_radius = readBallFloat(p2Ball, BALL_RADIUS);
                        msg.p2_gravity = readBallFloat(p2Ball, BALL_GRAVITY_PLANE);
                    }
                    sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
                }
            }
        }

        // ── GUEST: send P2 input state every cycle ─────────────────
        if (g_role == ROLE_GUEST && g_connState >= CONN_CONNECTED) {
            Ball* p2 = g_api ? g_api->GetPlayer() : nullptr;
            if (p2 && !IsBadReadPtr(p2, 0x300)) {
                InputStateMsg msg = {};
                msg.frame = g_frameCount;
                msg.force_x = readBallFloat((DWORD)p2, BALL_FORCE_X);
                msg.force_y = readBallFloat((DWORD)p2, BALL_FORCE_Y);
                msg.force_z = readBallFloat((DWORD)p2, BALL_FORCE_Z);
                sendPipeMsg(MSG_INPUT_STATE, &msg, sizeof(msg));
            }
        }

        // ── Both: send FPS report periodically ─────────────────────
        if (g_frameCount - g_lastHeartbeat >= HEARTBEAT_INTERVAL) {
            FpsReportMsg fps = {};
            fps.local_fps = g_localFps;
            fps.remote_fps = g_remoteFps;
            fps.frame_count = g_frameCount;
            sendPipeMsg(MSG_FPS_REPORT, &fps, sizeof(fps));
            g_lastHeartbeat = g_frameCount;
        }

        // ── Read incoming messages (non-blocking) ─────────────────
        DWORD type;
        char buf[512];
        DWORD len = 0;

        // Process ALL available messages (drain the pipe)
        while (readPipeMsgNonBlocking(&type, buf, sizeof(buf), &len)) {
            switch (type) {
                case MSG_BALL_STATE:
                    if (g_role == ROLE_GUEST && len == sizeof(BallStateMsg)) {
                        EnterCriticalSection(&g_stateLock);
                        memcpy(&g_latestBallState, buf, sizeof(BallStateMsg));
                        g_lastBallStateFrame = g_frameCount;
                        LeaveCriticalSection(&g_stateLock);
                    }
                    break;
                case MSG_INPUT_STATE:
                    if (g_role == ROLE_HOST && len == sizeof(InputStateMsg)) {
                        EnterCriticalSection(&g_inputLock);
                        memcpy(&g_latestInput, buf, sizeof(InputStateMsg));
                        g_lastInputFrame = g_frameCount;
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
                        if (st->remote_fps_raw > 0)
                            g_remoteFps = (float)st->remote_fps_raw;
                        strncpy(g_remoteInfo, st->remote_info, sizeof(g_remoteInfo));
                        g_remoteInfo[sizeof(g_remoteInfo)-1] = '\0';
                    }
                    break;
            }
        }

        Sleep(PIPE_POLL_MS);
    }

    disconnectPipe();
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════════
// Main Mod Class
// ═══════════════════════════════════════════════════════════════════════════

class NetplayMod : public HamsterballAPI {
private:
    void createButton(const char* id, const char* label) {
        CustomButton btn(id, label);
        btn.defaultState = false;
        btn.trueText = "ON";
        btn.falseText = "OFF";
        g_api->CreateToggleButton(btn, this);
    }

    void createSlider(const char* id, const char* label, float def,
                      float lo, float hi, float step, int decimals = 0) {
        CustomSlider s(id, label, def);
        s.lowerBound = lo;
        s.upperBound = hi;
        s.stepSize = step;
        s.decimalPlaces = decimals;
        g_api->CreateSlider(s, this);
    }

    // Mutually exclusive role switching
    void setRole(NetRole newRole) {
        g_role = newRole;
        if (newRole == ROLE_DISABLED) {
            g_connState = CONN_OFFLINE;
        } else {
            g_connState = CONN_SEARCHING;
        }
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

        // UI: Role toggles (mutually exclusive)
        createButton("NETPLAY_HOST", "Netplay: HOST Mode");
        createButton("NETPLAY_GUEST", "Netplay: GUEST Mode");

        // UI: Port slider
        createSlider("NETPLAY_PORT", "Netplay Port", 5029, 1024, 65535, 1);

        // UI: Host IP octets (guest mode)
        createSlider("NETPLAY_IP1", "Host IP: Octet 1", 127, 0, 255, 1);
        createSlider("NETPLAY_IP2", "Host IP: Octet 2", 0, 0, 255, 1);
        createSlider("NETPLAY_IP3", "Host IP: Octet 3", 0, 0, 255, 1);
        createSlider("NETPLAY_IP4", "Host IP: Octet 4", 1, 0, 255, 1);

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
        // Mutually exclusive: turning one on turns the other off
        if (strcmp(buttonId, "NETPLAY_HOST") == 0) {
            if (newState) {
                setRole(ROLE_HOST);
                // Visual: the framework manages button state, but we can't
                // programmatically toggle the guest button off here.
                // User must toggle it off manually.
            } else {
                if (g_role == ROLE_HOST) setRole(ROLE_DISABLED);
            }
        }
        if (strcmp(buttonId, "NETPLAY_GUEST") == 0) {
            if (newState) {
                setRole(ROLE_GUEST);
            } else {
                if (g_role == ROLE_GUEST) setRole(ROLE_DISABLED);
            }
        }
    }

    void onSliderChange(const char* sliderId, float newValue) override {
        if (strcmp(sliderId, "NETPLAY_PORT") == 0) {
            g_port = (int)newValue;
        }
        if (strcmp(sliderId, "NETPLAY_IP1") == 0) g_ip_octet[0] = (int)newValue;
        if (strcmp(sliderId, "NETPLAY_IP2") == 0) g_ip_octet[1] = (int)newValue;
        if (strcmp(sliderId, "NETPLAY_IP3") == 0) g_ip_octet[2] = (int)newValue;
        if (strcmp(sliderId, "NETPLAY_IP4") == 0) g_ip_octet[3] = (int)newValue;
    }

    void onGameUpdate() override {
        g_frameCount++;

        // Single FPS calculation (1-second window)
        g_fpsFrameCounter++;
        DWORD tick = GetTickCount();
        if (g_fpsLastTick == 0) g_fpsLastTick = tick;
        DWORD elapsed = tick - g_fpsLastTick;
        if (elapsed >= 1000) {
            g_localFps = (float)g_fpsFrameCounter * 1000.0f / (float)elapsed;
            g_fpsFrameCounter = 0;
            g_fpsLastTick = tick;
        }
    }

    void onTextRenderLoop() override {
        if (!g_api) return;

        // Build status display
        const char* roleStr = "OFF";
        if (g_role == ROLE_HOST) roleStr = "HOST";
        else if (g_role == ROLE_GUEST) roleStr = "GUEST";

        const char* connStr = "Offline";
        switch (g_connState) {
            case CONN_OFFLINE:    connStr = "Offline"; break;
            case CONN_SEARCHING:   connStr = "Searching..."; break;
            case CONN_CONNECTING:  connStr = "Connecting..."; break;
            case CONN_CONNECTED:   connStr = "Connected!"; break;
            case CONN_ERROR:       connStr = "ERROR (relay not running?)"; break;
        }

        // Format display lines
        char line1[128], line2[128], line3[128], line4[128];

        snprintf(line1, sizeof(line1),
                 "NETPLAY [%s] %s  Port:%d",
                 roleStr, connStr, g_port);

        snprintf(line2, sizeof(line2),
                 "Local FPS:%.1f  Remote FPS:%.1f  Frame:%d",
                 g_localFps, g_remoteFps, g_frameCount);

        if (g_role == ROLE_GUEST) {
            updateHostIPString();
            snprintf(line3, sizeof(line3),
                     "Host: %s:%d", g_hostIP, g_port);
        } else if (g_remoteInfo[0]) {
            snprintf(line3, sizeof(line3),
                     "Remote: %s", g_remoteInfo);
        } else {
            snprintf(line3, sizeof(line3), "---");
        }

        // Data flow indicators
        if (g_role == ROLE_GUEST && g_connState == CONN_CONNECTED) {
            DWORD age = g_frameCount - g_lastBallStateFrame;
            snprintf(line4, sizeof(line4),
                     "Ball data: %s (last %d frames ago)",
                     age < 10 ? "LIVE" : "STALE", age);
        } else if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED) {
            DWORD age = g_frameCount - g_lastInputFrame;
            snprintf(line4, sizeof(line4),
                     "Input: %s (last %d frames ago)",
                     g_lastInputFrame > 0 && age < 10 ? "LIVE" : "WAITING",
                     g_lastInputFrame > 0 ? age : 0);
        } else {
            line4[0] = '\0';
        }

        // Render text overlay
        CustomText params = {};
        App* app = g_api->GetApp();
        if (app && !IsBadReadPtr(app, sizeof(App))) {
            params.font = app->fonts.showcardGothic14;
        }

        params.x = 10;
        params.y = 10;
        params.enable_shadow = true;

        params.text_color = Color(0.0f, 1.0f, 0.0f, 1.0f);  // green
        g_api->DrawCustomText(line1, params);

        params.y = 28;
        params.text_color = Color(1.0f, 1.0f, 0.0f, 1.0f);  // yellow
        g_api->DrawCustomText(line2, params);

        params.y = 46;
        params.text_color = Color(0.5f, 0.8f, 1.0f, 1.0f);  // light blue
        g_api->DrawCustomText(line3, params);

        if (line4[0]) {
            params.y = 64;
            params.text_color = Color(1.0f, 0.5f, 1.0f, 1.0f);  // magenta
            g_api->DrawCustomText(line4, params);
        }

        // ── GUEST: Apply ball state from host to ghost ball ───────
        if (g_role == ROLE_GUEST && g_connState == CONN_CONNECTED) {
            applyGuestBallState();
        }

        // ── HOST: Apply guest input to P2 ball ────────────────────
        if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED) {
            applyHostInput();
        }
    }

    void onLevelStart() override {
        g_frameCount = 0;
        g_ghostBallInitialized = false;
        // Guest: ghost ball will be created on first applyGuestBallState call
    }

private:
    // ── GUEST: Write host's P1 state to a display ball ──────────────
    void applyGuestBallState() {
        BallStateMsg state;
        EnterCriticalSection(&g_stateLock);
        state = g_latestBallState;
        LeaveCriticalSection(&g_stateLock);

        if (state.frame == 0) return;  // no data yet

        // For PoC: write P1 position to the scene's ghost ball pointer
        // (scene+0x361C). If no ghost ball exists, we write directly
        // to the player's ball for visual testing.
        //
        // Full implementation would:
        // 1. Allocate a Ball via Ball_ctor (0x40AFE0) on first frame
        // 2. Store at scene+0x361C
        // 3. Each frame: write pos/vel/rot from network state
        //
        // For now: if we have a local ball, just write P1's position
        // to a secondary display by drawing a marker at P1's coordinates.
        // The actual ghost ball creation requires careful memory management
        // (Ball_ctor, vtable setup, AthenaList append) that should be
        // tested in-game before enabling.

        // Safe PoC: just track that we received valid data
        // Real ball state injection would go here in next iteration
    }

    // ── HOST: Write guest's input to P2 ball ────────────────────────
    void applyHostInput() {
        InputStateMsg input;
        EnterCriticalSection(&g_inputLock);
        input = g_latestInput;
        LeaveCriticalSection(&g_inputLock);

        if (input.frame == 0) return;

        // Get P2 ball from scene's ball_list_2
        DWORD scene = getScene();
        if (!scene) return;

        if (IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) return;
        DWORD listBase = scene + SCENE_P2_BALL_LIST;
        DWORD count = *(DWORD*)(listBase + 4);
        if (count == 0) return;
        if (IsBadReadPtr((void*)(listBase + 8), 4)) return;
        DWORD p2Ball = *(DWORD*)(listBase + 8);
        if (!p2Ball || IsBadReadPtr((void*)p2Ball, 0x300)) return;

        // Use the HB+ API to apply force properly (if available)
        Ball* p2BallObj = (Ball*)p2Ball;
        if (g_api) {
            // Apply force via the API's ApplyForce method
            // This uses Ball_ApplyForceV2 internally with proper guards
            g_api->ApplyForce(p2BallObj, input.force_x, input.force_y,
                              input.force_z, 1.0f);
        } else {
            // Fallback: direct write to force accumulators
            *(float*)(p2Ball + BALL_FORCE_X) = input.force_x;
            *(float*)(p2Ball + BALL_FORCE_Y) = input.force_y;
            *(float*)(p2Ball + BALL_FORCE_Z) = input.force_z;
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Export
// ═══════════════════════════════════════════════════════════════════════════

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new NetplayMod();
}
