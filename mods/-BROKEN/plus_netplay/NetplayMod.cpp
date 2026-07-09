#include "nocrt.h"
#include "HamsterballAPI.h"
#include "hbplus_api.h"

#define malloc nc_malloc
#define free nc_free
#define memcpy nc_memcpy
#define memset nc_memset
#define strlen nc_strlen
#define strcmp nc_strcmp
#define strncpy nc_strncpy
#define snprintf nc_snprintf

#define PIPE_NAME "\\\\.\\pipe\\hamsterball_netplay"
#define PIPE_POLL_MS 4
#define HEARTBEAT_INTERVAL 300
#define STREAM_INTERVAL_MS 33    // ~30Hz stream rate
#define INTERP_SMOOTH 0.3f       // lerp factor for position smoothing

// Direct memory constants
static constexpr DWORD GLOBAL_APP_PTR       = 0x5341E0;
static constexpr DWORD APP_PROFILE_OFFSET   = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN     = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX     = 0x4D2000;
static constexpr DWORD BALL_VTABLE           = 0x4CF3A0;

// Ball offsets
static constexpr DWORD BALL_POS_X     = 0x164;
static constexpr DWORD BALL_POS_Y     = 0x168;
static constexpr DWORD BALL_POS_Z     = 0x16C;
static constexpr DWORD BALL_FACING_X  = 0x190;
static constexpr DWORD BALL_FACING_Z  = 0x194;
static constexpr DWORD BALL_ROT       = 0x150;
static constexpr DWORD BALL_RADIUS    = 0x284;
static constexpr DWORD BALL_GRAVITY   = 0x748;
static constexpr DWORD BALL_PHYS_PTR   = 0x1A4;
static constexpr DWORD BALL_PLAYER_ID  = 0x018;

// Physics velocity (persists — not consumed like force accumulators)
static constexpr DWORD PHYS_VEL_X     = 0xCA4;
static constexpr DWORD PHYS_VEL_Y     = 0xCA8;
static constexpr DWORD PHYS_VEL_Z     = 0xCAC;

// Scene ball pointers
static constexpr DWORD SCENE_P1_BALL  = 0x29D0;
static constexpr DWORD SCENE_P2_LIST  = 0x3204;

// Input control slots
static constexpr DWORD APP_CONTROL_SLOTS = 0xB28; // App+0xB28 + playerIndex*4

// Pause flag
static constexpr DWORD BOARD_PAUSE_FLAG = 0x874;

enum NetRole { ROLE_DISABLED = 0, ROLE_HOST = 1, ROLE_GUEST = 2 };
enum ConnState { CONN_OFFLINE = 0, CONN_SEARCHING = 1, CONN_CONNECTING = 2, CONN_CONNECTED = 3, CONN_ERROR = 4 };
enum MsgType {
    MSG_PING = 0, MSG_PONG = 1, MSG_BALL_STATE = 2, MSG_INPUT_STATE = 3,
    MSG_FPS_REPORT = 4, MSG_ROLE_SET = 5, MSG_PORT_SET = 6, MSG_STATUS = 7, MSG_HOST_IP = 8,
};

#pragma pack(push, 1)
struct PipeHeader { DWORD type; DWORD length; };
struct BallStateMsg {
    DWORD frame;
    float pos[3];
    float vel[3];
    float facing[2];
    float rot;
    float radius;
    float gravity;
};
struct FpsReportMsg { float local_fps, remote_fps; DWORD frame_count; };
struct StatusMsg { DWORD conn_state; DWORD remote_fps_raw; char remote_info[64]; };
#pragma pack(pop)

// ── Global State ───────────────────────────────────────────────────

static IModAPI* g_api = NULL;
static volatile NetRole g_role = ROLE_DISABLED;
static volatile ConnState g_connState = CONN_OFFLINE;
static volatile int g_port = 5029;
static volatile int g_ip_octet[4] = {127, 0, 0, 1};
static char g_hostIP[32] = "127.0.0.1";
static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static HANDLE g_pipeThread = NULL;
static volatile bool g_pipeRunning = false;
static DWORD g_frameCount = 0;
static DWORD g_fpsFrameCounter = 0;
static DWORD g_fpsLastTick = 0;
static float g_localFps = 0.0f;
static float g_remoteFps = 0.0f;
static DWORD g_lastHeartbeat = 0;
static char g_remoteInfo[128] = "";
static volatile bool g_gameReady = false;
static volatile bool g_localPaused = false;
static DWORD g_lastPauseFlag = 0;
static volatile bool g_inRace = false;

// Network state buffers with interpolation
static BallStateMsg g_remoteState = {};
static BallStateMsg g_prevRemoteState = {};  // for interpolation
static CRITICAL_SECTION g_stateLock;
static volatile DWORD g_remoteStateFrame = 0;
static volatile DWORD g_lastAppliedFrame = 0;
static DWORD g_lastStreamTime = 0;

// Control slot backup (for restoring on race exit)
static int g_savedControlSlot = -1;

// ── Direct Memory Access ───────────────────────────────────────────

static DWORD getApp() {
    DWORD appPtr = *(DWORD*)GLOBAL_APP_PTR;
    if (!appPtr || appPtr < 0x10000) return 0;
    if (IsBadReadPtr((void*)appPtr, 0x1000)) return 0;
    return appPtr;
}

static DWORD findBoard() {
    DWORD app = getApp();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_PROFILE_OFFSET), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_PROFILE_OFFSET);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + PROFILE_BOARD_OFFSET), 4)) return 0;
    DWORD board = *(DWORD*)(profile + PROFILE_BOARD_OFFSET);
    if (!board || board < 0x10000) return 0;
    if (IsBadReadPtr((void*)board, 4)) return 0;
    DWORD vtable = *(DWORD*)board;
    if (vtable < BOARD_VTABLE_MIN || vtable > BOARD_VTABLE_MAX) return 0;
    return board;
}

static DWORD getBall(int playerIndex) {
    DWORD board = findBoard();
    if (!board) return 0;
    if (playerIndex == 0) {
        if (IsBadReadPtr((void*)(board + SCENE_P1_BALL), 4)) return 0;
        DWORD ball = *(DWORD*)(board + SCENE_P1_BALL);
        if (!ball || ball < 0x10000) return 0;
        if (IsBadReadPtr((void*)ball, 0x800)) return 0;
        if (*(DWORD*)ball != BALL_VTABLE) return 0;
        return ball;
    }
    // P2 from AthenaList
    if (IsBadReadPtr((void*)(board + SCENE_P2_LIST), 8)) return 0;
    DWORD listBase = board + SCENE_P2_LIST;
    DWORD count = *(DWORD*)(listBase + 4);
    if (count == 0) return 0;
    if (IsBadReadPtr((void*)(listBase + 8), 4)) return 0;
    DWORD ball = *(DWORD*)(listBase + 8);
    if (!ball || ball < 0x10000) return 0;
    if (IsBadReadPtr((void*)ball, 0x800)) return 0;
    return ball;
}

static float readF(DWORD addr, DWORD off) {
    if (!addr || IsBadReadPtr((void*)(addr + off), 4)) return 0.0f;
    return *(float*)(addr + off);
}

static DWORD readD(DWORD addr, DWORD off) {
    if (!addr || IsBadReadPtr((void*)(addr + off), 4)) return 0;
    return *(DWORD*)(addr + off);
}

static void readVel(DWORD ball, float* vx, float* vy, float* vz) {
    *vx = *vy = *vz = 0.0f;
    if (!ball) return;
    DWORD phys = readD(ball, BALL_PHYS_PTR);
    if (!phys || phys < 0x10000) return;
    if (IsBadReadPtr((void*)(phys + PHYS_VEL_Z), 4)) return;
    *vx = *(float*)(phys + PHYS_VEL_X);
    *vy = *(float*)(phys + PHYS_VEL_Y);
    *vz = *(float*)(phys + PHYS_VEL_Z);
}

static void writeVel(DWORD ball, float vx, float vy, float vz) {
    if (!ball) return;
    DWORD phys = readD(ball, BALL_PHYS_PTR);
    if (!phys || phys < 0x10000) return;
    if (IsBadWritePtr((void*)(phys + PHYS_VEL_X), 12)) return;
    *(float*)(phys + PHYS_VEL_X) = vx;
    *(float*)(phys + PHYS_VEL_Y) = vy;
    *(float*)(phys + PHYS_VEL_Z) = vz;
}

static void writePos(DWORD ball, float x, float y, float z) {
    if (!ball || IsBadWritePtr((void*)(ball + BALL_POS_X), 12)) return;
    *(float*)(ball + BALL_POS_X) = x;
    *(float*)(ball + BALL_POS_Y) = y;
    *(float*)(ball + BALL_POS_Z) = z;
}

// ── Input Suppression ─────────────────────────────────────────────
// Zero the force_mult on the remote player's input handler so the game
// reads zero local input for that player. Our velocity overwrite then
// has full control of the remote ball.
//
// Input handler struct (at App+0x550 + controlSlot*4 → pointer):
//   +0x00: vtable
//   +0x04: App/Scene pointer
//   +0x08: input mode (1=keyboard, 2=mouse, 4-7=gamepad)
//   +0x0C: force_mult (float, scales raw input direction)
//
// Control slot table: App+0xB28 + playerIndex*4 = slot index (0-3)
// App+0x550 + slotIndex*4 = pointer to input handler

static float g_savedForceMult = -1.0f;
static DWORD g_suppressedHandler = 0;

static void suppressLocalInput(int playerIndex) {
    DWORD app = getApp();
    if (!app) return;
    // Read control slot for this player
    if (IsBadReadPtr((void*)(app + 0xB28 + playerIndex * 4), 4)) return;
    DWORD slot = *(DWORD*)(app + 0xB28 + playerIndex * 4);
    if (slot > 3) return; // 99=CPU, 100=CPU — skip
    // Get input handler pointer
    if (IsBadReadPtr((void*)(app + 0x550 + slot * 4), 4)) return;
    DWORD handler = *(DWORD*)(app + 0x550 + slot * 4);
    if (!handler || handler < 0x10000) return;
    if (IsBadReadPtr((void*)(handler + 0x0C), 4)) return;
    // Save original force_mult and zero it
    g_savedForceMult = *(float*)(handler + 0x0C);
    g_suppressedHandler = handler;
    *(float*)(handler + 0x0C) = 0.0f;
}

static void restoreLocalInput(int playerIndex) {
    if (g_suppressedHandler && g_savedForceMult >= 0.0f) {
        if (!IsBadWritePtr((void*)(g_suppressedHandler + 0x0C), 4)) {
            *(float*)(g_suppressedHandler + 0x0C) = g_savedForceMult;
        }
    }
    g_suppressedHandler = 0;
    g_savedForceMult = -1.0f;
}

// ── Config File ───────────────────────────────────────────────────

static void updateHostIPString() {
    snprintf(g_hostIP, sizeof(g_hostIP), "%d.%d.%d.%d",
             g_ip_octet[0], g_ip_octet[1], g_ip_octet[2], g_ip_octet[3]);
}

static void loadConfigFile() {
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (len == 0) return;
    for (DWORD i = len; i > 0; i--) {
        if (path[i] == '\\') { path[i+1] = '\0'; break; }
    }
    char* p = path;
    while (*p) p++;
    const char* fname = "netplay.txt";
    while (*fname) *p++ = *fname++;
    *p = '\0';

    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    char buf[256];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf)-1, &bytesRead, NULL);
    CloseHandle(h);
    if (bytesRead == 0) return;
    buf[bytesRead] = '\0';

    char* line = buf;
    if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
        line += 3;
    char* end = line;
    while (*end && *end != '\n' && *end != '\r') end++;
    *end = '\0';
    while (*line == ' ' || *line == '\t') line++;

    // Check for :port suffix
    char* colon = line;
    while (*colon && *colon != ':') colon++;
    if (*colon == ':') {
        *colon = '\0';
        int port = 0;
        char* cp = colon + 1;
        while (*cp >= '0' && *cp <= '9') { port = port * 10 + (*cp - '0'); cp++; }
        if (port >= 1024 && port <= 65535) g_port = port;
    }

    // Parse IP octets
    int octets[4] = {0, 0, 0, 1};
    int idx = 0;
    char* ip = line;
    while (*ip && idx < 4) {
        int val = 0;
        while (*ip >= '0' && *ip <= '9') { val = val * 10 + (*ip - '0'); ip++; }
        if (val > 255) val = 255;
        octets[idx++] = val;
        if (*ip == '.') ip++;
    }
    if (idx == 4) {
        g_ip_octet[0] = octets[0];
        g_ip_octet[1] = octets[1];
        g_ip_octet[2] = octets[2];
        g_ip_octet[3] = octets[3];
    }
    updateHostIPString();
}

// ── Pipe Communication ────────────────────────────────────────────

static bool connectToPipe() {
    if (g_pipe != INVALID_HANDLE_VALUE) return true;
    g_pipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, 0, NULL);
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
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
        disconnectPipe(); return false;
    }
    if (dataLen > 0 && data) {
        if (!WriteFile(g_pipe, data, dataLen, &written, NULL) || written != dataLen) {
            disconnectPipe(); return false;
        }
    }
    return true;
}

static bool readPipeMsgNonBlocking(DWORD* outType, void* outBuf, DWORD bufSize, DWORD* outLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(g_pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
        disconnectPipe(); return false;
    }
    if (bytesAvailable < sizeof(PipeHeader)) return false;
    DWORD bytesRead = 0;
    if (!ReadFile(g_pipe, outType, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe(); return false;
    }
    DWORD len = 0;
    if (!ReadFile(g_pipe, &len, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe(); return false;
    }
    if (len > bufSize) len = bufSize;
    if (len > 0) {
        if (!ReadFile(g_pipe, outBuf, len, &bytesRead, NULL) || bytesRead != len) {
            disconnectPipe(); return false;
        }
    }
    if (outLen) *outLen = len;
    return true;
}

// ── Pipe Thread ───────────────────────────────────────────────────

static DWORD WINAPI pipeThreadFunc(LPVOID param) {
    int noPipeCount = 0;
    NetRole lastSentRole = ROLE_DISABLED;
    int lastSentPort = -1;
    char lastSentIP[32] = "";

    while (g_pipeRunning) {
        if (!connectToPipe()) {
            Sleep(500);
            noPipeCount++;
            if (noPipeCount == 10) g_connState = CONN_ERROR;
            continue;
        }
        noPipeCount = 0;

        // Send config
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

        // Stream local player's ball state at 30Hz
        DWORD now = GetTickCount();
        if (g_connState >= CONN_CONNECTED && g_inRace && (now - g_lastStreamTime) >= STREAM_INTERVAL_MS) {
            g_lastStreamTime = now;
            int localPlayer = (g_role == ROLE_HOST) ? 0 : 1;
            DWORD ball = getBall(localPlayer);
            if (ball) {
                BallStateMsg msg;
                memset(&msg, 0, sizeof(msg));
                msg.frame = g_frameCount;
                msg.pos[0] = readF(ball, BALL_POS_X);
                msg.pos[1] = readF(ball, BALL_POS_Y);
                msg.pos[2] = readF(ball, BALL_POS_Z);
                readVel(ball, &msg.vel[0], &msg.vel[1], &msg.vel[2]);
                msg.facing[0] = readF(ball, BALL_FACING_X);
                msg.facing[1] = readF(ball, BALL_FACING_Z);
                msg.rot = readF(ball, BALL_ROT);
                msg.radius = readF(ball, BALL_RADIUS);
                msg.gravity = readF(ball, BALL_GRAVITY);
                sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
            }
        }

        // FPS heartbeat
        if (g_frameCount - g_lastHeartbeat >= HEARTBEAT_INTERVAL) {
            FpsReportMsg fps;
            fps.local_fps = g_localFps;
            fps.remote_fps = g_remoteFps;
            fps.frame_count = g_frameCount;
            sendPipeMsg(MSG_FPS_REPORT, &fps, sizeof(fps));
            g_lastHeartbeat = g_frameCount;
        }

        // Read incoming
        DWORD type;
        char buf[512];
        DWORD len = 0;
        while (readPipeMsgNonBlocking(&type, buf, sizeof(buf), &len)) {
            switch (type) {
                case MSG_BALL_STATE:
                    if (len == sizeof(BallStateMsg)) {
                        BallStateMsg* incoming = (BallStateMsg*)buf;
                        // Frame sequencing: only accept newer frames
                        if (incoming->frame > g_remoteStateFrame || g_remoteStateFrame == 0) {
                            EnterCriticalSection(&g_stateLock);
                            g_prevRemoteState = g_remoteState; // save old for interpolation
                            memcpy(&g_remoteState, buf, sizeof(BallStateMsg));
                            g_remoteStateFrame = incoming->frame;
                            LeaveCriticalSection(&g_stateLock);
                        }
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

// ── Apply remote state to remote player's ball ────────────────────

static void applyRemoteState() {
    if (g_remoteStateFrame == 0) return;

    BallStateMsg state;
    BallStateMsg prevState;
    EnterCriticalSection(&g_stateLock);
    state = g_remoteState;
    prevState = g_prevRemoteState;
    LeaveCriticalSection(&g_stateLock);

    int remotePlayer = (g_role == ROLE_HOST) ? 1 : 0;
    DWORD ball = getBall(remotePlayer);
    if (!ball) return;

    // Interpolate position for smooth movement
    // Lerp from previous received position toward current target
    float curX = readF(ball, BALL_POS_X);
    float curY = readF(ball, BALL_POS_Y);
    float curZ = readF(ball, BALL_POS_Z);

    // Smooth toward target position (reduces snapping)
    float targetX = prevState.pos[0] + (state.pos[0] - prevState.pos[0]) * INTERP_SMOOTH;
    float targetY = prevState.pos[1] + (state.pos[1] - prevState.pos[1]) * INTERP_SMOOTH;
    float targetZ = prevState.pos[2] + (state.pos[2] - prevState.pos[2]) * INTERP_SMOOTH;

    // If position delta is large (>500 units), snap instead of lerp (teleport)
    float dx = state.pos[0] - curX;
    float dy = state.pos[1] - curY;
    float dz = state.pos[2] - curZ;
    float distSq = dx*dx + dy*dy + dz*dz;

    if (distSq > 250000.0f) {
        // Large jump — snap directly
        writePos(ball, state.pos[0], state.pos[1], state.pos[2]);
    } else {
        // Smooth lerp toward target
        float lerpX = curX + (targetX - curX) * INTERP_SMOOTH;
        float lerpY = curY + (targetY - curY) * INTERP_SMOOTH;
        float lerpZ = curZ + (targetZ - curZ) * INTERP_SMOOTH;
        writePos(ball, lerpX, lerpY, lerpZ);
    }

    // Always write velocity directly (no interpolation — velocity changes fast)
    writeVel(ball, state.vel[0], state.vel[1], state.vel[2]);

    // Write rotation + facing
    if (!IsBadWritePtr((void*)(ball + BALL_ROT), 4))
        *(float*)(ball + BALL_ROT) = state.rot;
    if (!IsBadWritePtr((void*)(ball + BALL_FACING_X), 8)) {
        *(float*)(ball + BALL_FACING_X) = state.facing[0];
        *(float*)(ball + BALL_FACING_Z) = state.facing[1];
    }
}

// ── Vtable callback implementations ───────────────────────────────

static void* __thiscall sc_dtor(void* thisptr, int flags) {
    g_pipeRunning = false;
    if (g_pipeThread) {
        WaitForSingleObject(g_pipeThread, 2000);
        CloseHandle(g_pipeThread);
        g_pipeThread = NULL;
    }
    DeleteCriticalSection(&g_stateLock);
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Netplay"; }
static const char* __thiscall get_author(void*) { return "rsks + Hamsterbot"; }
static const char* __thiscall get_contributors(void*) {
    return "Distributed authority + state sync. Host=P1, Guest=P2.";
}
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    InitializeCriticalSection(&g_stateLock);
    loadConfigFile();

    HBPlusAPI hb = HBAPI(api);

    // Single mode slider: 0=Disabled, 1=Host (P1), 2=Guest (P2)
    CustomSlider sMode("NETPLAY_MODE", "Netplay Mode (0=Off, 1=Host, 2=Guest)", 0.0f);
    sMode.lowerBound = 0; sMode.upperBound = 2; sMode.stepSize = 1; sMode.decimalPlaces = 0;
    hb.CreateSlider(sMode, thisptr);

    // Status slider: shows connection state (read-only display)
    CustomSlider sStatus("NETPLAY_STATUS", "Netplay Status (3=Connected)", 0.0f);
    sStatus.lowerBound = 0; sStatus.upperBound = 4; sStatus.stepSize = 1; sStatus.decimalPlaces = 0;
    hb.CreateSlider(sStatus, thisptr);

    CustomSlider sPort("NETPLAY_PORT", "Netplay Port", (float)g_port);
    sPort.lowerBound = 1024; sPort.upperBound = 65535; sPort.stepSize = 1; sPort.decimalPlaces = 0;
    hb.CreateSlider(sPort, thisptr);

    CustomSlider sIp1("NETPLAY_IP1", "Host IP: Octet 1", (float)g_ip_octet[0]);
    sIp1.lowerBound = 0; sIp1.upperBound = 255; sIp1.stepSize = 1; sIp1.decimalPlaces = 0;
    hb.CreateSlider(sIp1, thisptr);
    CustomSlider sIp2("NETPLAY_IP2", "Host IP: Octet 2", (float)g_ip_octet[1]);
    sIp2.lowerBound = 0; sIp2.upperBound = 255; sIp2.stepSize = 1; sIp2.decimalPlaces = 0;
    hb.CreateSlider(sIp2, thisptr);
    CustomSlider sIp3("NETPLAY_IP3", "Host IP: Octet 3", (float)g_ip_octet[2]);
    sIp3.lowerBound = 0; sIp3.upperBound = 255; sIp3.stepSize = 1; sIp3.decimalPlaces = 0;
    hb.CreateSlider(sIp3, thisptr);
    CustomSlider sIp4("NETPLAY_IP4", "Host IP: Octet 4", (float)g_ip_octet[3]);
    sIp4.lowerBound = 0; sIp4.upperBound = 255; sIp4.stepSize = 1; sIp4.decimalPlaces = 0;
    hb.CreateSlider(sIp4, thisptr);
}

static void __thiscall ball_update(void*, Ball*) {}

static void __thiscall render_apply(void*, void*, float*) {}

static void __thiscall button_toggle(void* thisptr, const char* buttonId, bool newState) {
    // No toggle buttons in v7 — mode is a slider
}

static void __thiscall slider_change(void* thisptr, const char* sliderId, float newValue) {
    if (strcmp(sliderId, "NETPLAY_MODE") == 0) {
        int mode = (int)newValue;
        if (mode == 1) { g_role = ROLE_HOST; g_connState = CONN_SEARCHING; }
        else if (mode == 2) { g_role = ROLE_GUEST; g_connState = CONN_SEARCHING; }
        else { g_role = ROLE_DISABLED; g_connState = CONN_OFFLINE; }
    }
    if (strcmp(sliderId, "NETPLAY_PORT") == 0) g_port = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP1") == 0) g_ip_octet[0] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP2") == 0) g_ip_octet[1] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP3") == 0) g_ip_octet[2] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP4") == 0) g_ip_octet[3] = (int)newValue;
    // NETPLAY_STATUS is read-only — ignore user changes
}

static void __thiscall game_update(void*) {
    g_frameCount++;
    if (!g_gameReady && g_frameCount > 120) g_gameReady = true;

    // Pause interception: only during races (not menus — ESC = quit/back in menus)
    DWORD board = findBoard();
    if (board && g_inRace && !IsBadReadPtr((void*)(board + BOARD_PAUSE_FLAG), 1)) {
        DWORD pauseFlag = *(DWORD*)(board + BOARD_PAUSE_FLAG);
        if (pauseFlag != g_lastPauseFlag) {
            if (pauseFlag == 1) {
                g_localPaused = !g_localPaused;
                *(DWORD*)(board + BOARD_PAUSE_FLAG) = 0;
            }
            g_lastPauseFlag = pauseFlag;
        }
    } else {
        g_lastPauseFlag = 0; // reset when not in race
    }

    // Race detection: in race if board exists and P1 ball exists
    bool wasInRace = g_inRace;
    g_inRace = false;
    if (g_gameReady && g_role != ROLE_DISABLED) {
        DWORD p1 = getBall(0);
        if (p1) {
            float py = readF(p1, BALL_POS_Y);
            if (py > -100000.0f && py < 100000.0f) {
                g_inRace = true;
                // Just entered race: suppress input for remote player
                if (!wasInRace) {
                    int remotePlayer = (g_role == ROLE_HOST) ? 1 : 0;
                    suppressLocalInput(remotePlayer);
                }
            }
        }
    }
    // Left race: restore input
    if (wasInRace && !g_inRace) {
        int remotePlayer = (g_role == ROLE_HOST) ? 1 : 0;
        restoreLocalInput(remotePlayer);
    }

    if (g_gameReady && !g_pipeThread) {
        g_pipeRunning = true;
        g_pipeThread = CreateThread(NULL, 0, pipeThreadFunc, NULL, 0, NULL);
    }

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

static void __thiscall text_render(void* thisptr) {
    if (!g_gameReady || !g_inRace) return;
    if (g_connState != CONN_CONNECTED) return;
    if (g_localPaused) return;

    // Apply remote player's state to their ball
    // This overwrites the remote player's velocity every frame.
    // Local keyboard input adds small force, but our velocity
    // overwrite dominates — remote ball follows network state.
    applyRemoteState();
}

static void __thiscall event_collide(void*, Ball*, char*) {}
static void __thiscall ball_bump(void*, Ball*, Ball*) {}
static void __thiscall scene_end(void*) {
    // Restore input when leaving a level
    if (g_inRace) {
        int remotePlayer = (g_role == ROLE_HOST) ? 1 : 0;
        restoreLocalInput(remotePlayer);
    }
    g_inRace = false;
}
static void __thiscall level_start(void*) {
    g_frameCount = 0;
    g_remoteStateFrame = 0;
    g_lastAppliedFrame = 0;
    memset(&g_remoteState, 0, sizeof(g_remoteState));
    memset(&g_prevRemoteState, 0, sizeof(g_prevRemoteState));
}

// ── 16-entry vtable matching MSVC layout ───────────────────────────

static void* g_vtable[16] = {
    (void*)sc_dtor,
    (void*)get_mod_name,
    (void*)get_author,
    (void*)get_version,
    (void*)get_contributors,
    (void*)init_impl,
    (void*)ball_update,
    (void*)render_apply,
    (void*)button_toggle,
    (void*)slider_change,
    (void*)game_update,
    (void*)event_collide,
    (void*)text_render,
    (void*)ball_bump,
    (void*)scene_end,
    (void*)level_start,
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    void* obj = operator new(8);
    *(void**)obj = g_vtable;
    *(void**)((char*)obj + 4) = NULL;
    return (HamsterballAPI*)obj;
}
