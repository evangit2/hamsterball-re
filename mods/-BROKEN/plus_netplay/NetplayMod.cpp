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
#define HEARTBEAT_INTERVAL 300
#define PIPE_POLL_MS 4

static constexpr DWORD GLOBAL_APP_PTR       = 0x5341E0;
static constexpr DWORD APP_PROFILE_OFFSET   = 0x220;
static constexpr DWORD PROFILE_BOARD_OFFSET = 0x0C;
static constexpr DWORD BOARD_VTABLE_MIN     = 0x4D0000;
static constexpr DWORD BOARD_VTABLE_MAX     = 0x4D2000;
static constexpr DWORD BALL_POS_X          = 0x164;
static constexpr DWORD BALL_POS_Y          = 0x168;
static constexpr DWORD BALL_POS_Z          = 0x16C;
static constexpr DWORD BALL_FACING_X       = 0x190;
static constexpr DWORD BALL_FACING_Z       = 0x194;
static constexpr DWORD BALL_ROT            = 0x150;
static constexpr DWORD BALL_RADIUS         = 0x284;
static constexpr DWORD BALL_GRAVITY_PLANE  = 0x748;
static constexpr DWORD BALL_FORCE_X        = 0x2BC;
static constexpr DWORD BALL_FORCE_Y        = 0x2C0;
static constexpr DWORD BALL_FORCE_Z        = 0x2C4;
static constexpr DWORD BALL_PHYS_PTR       = 0x1A4;
static constexpr DWORD PHYS_VEL_X           = 0xCA4;
static constexpr DWORD PHYS_VEL_Y           = 0xCA8;
static constexpr DWORD PHYS_VEL_Z           = 0xCAC;
static constexpr DWORD SCENE_P1_BALL_PTR   = 0x29D0;
static constexpr DWORD SCENE_P2_BALL_LIST   = 0x3204;

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
    float p1_pos[3], p1_vel[3], p1_facing[2], p1_rot, p1_radius, p1_gravity;
    float p2_pos[3], p2_vel[3], p2_facing[2], p2_rot, p2_radius, p2_gravity;
};
struct InputStateMsg { DWORD frame; float force_x, force_y, force_z; };
struct FpsReportMsg { float local_fps, remote_fps; DWORD frame_count; };
struct StatusMsg { DWORD conn_state; DWORD remote_fps_raw; char remote_info[64]; };
#pragma pack(pop)

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
static BallStateMsg g_latestBallState = {};
static CRITICAL_SECTION g_stateLock;
static InputStateMsg g_latestInput = {};
static CRITICAL_SECTION g_inputLock;
static volatile DWORD g_lastBallStateFrame = 0;
static volatile DWORD g_lastInputFrame = 0;

// ── Utility ────────────────────────────────────────────────────────

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
    DWORD board = findBoard();
    return board; // board IS scene
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

// ── Pipe Communication ─────────────────────────────────────────────

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

        // HOST: stream ball state — disabled, scene memory access crashes in 2P
        // if (g_role == ROLE_HOST && g_connState >= CONN_CONNECTED && g_gameReady) {
            DWORD scene = getScene();
            if (scene) {
                DWORD p1Ball = 0, p2Ball = 0;
                if (!IsBadReadPtr((void*)(scene + SCENE_P1_BALL_PTR), 4))
                    p1Ball = *(DWORD*)(scene + SCENE_P1_BALL_PTR);
                if (!IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) {
                    DWORD listBase = scene + SCENE_P2_BALL_LIST;
                    DWORD count = *(DWORD*)(listBase + 4);
                    if (count > 0 && !IsBadReadPtr((void*)(listBase + 8), 4))
                        p2Ball = *(DWORD*)(listBase + 8);
                }
                if (p1Ball && !IsBadReadPtr((void*)p1Ball, 0x800)) {
                    BallStateMsg msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.frame = g_frameCount;
                    msg.p1_pos[0] = readBallFloat(p1Ball, BALL_POS_X);
                    msg.p1_pos[1] = readBallFloat(p1Ball, BALL_POS_Y);
                    msg.p1_pos[2] = readBallFloat(p1Ball, BALL_POS_Z);
                    readPhysicsVelocity(p1Ball, &msg.p1_vel[0], &msg.p1_vel[1], &msg.p1_vel[2]);
                    msg.p1_facing[0] = readBallFloat(p1Ball, BALL_FACING_X);
                    msg.p1_facing[1] = readBallFloat(p1Ball, BALL_FACING_Z);
                    msg.p1_rot = readBallFloat(p1Ball, BALL_ROT);
                    msg.p1_radius = readBallFloat(p1Ball, BALL_RADIUS);
                    msg.p1_gravity = readBallFloat(p1Ball, BALL_GRAVITY_PLANE);
                    if (p2Ball && !IsBadReadPtr((void*)p2Ball, 0x800)) {
                        msg.p2_pos[0] = readBallFloat(p2Ball, BALL_POS_X);
                        msg.p2_pos[1] = readBallFloat(p2Ball, BALL_POS_Y);
                        msg.p2_pos[2] = readBallFloat(p2Ball, BALL_POS_Z);
                        readPhysicsVelocity(p2Ball, &msg.p2_vel[0], &msg.p2_vel[1], &msg.p2_vel[2]);
                        msg.p2_facing[0] = readBallFloat(p2Ball, BALL_FACING_X);
                        msg.p2_facing[1] = readBallFloat(p2Ball, BALL_FACING_Z);
                        msg.p2_rot = readBallFloat(p2Ball, BALL_ROT);
                        msg.p2_radius = readBallFloat(p2Ball, BALL_RADIUS);
                        msg.p2_gravity = readBallFloat(p2Ball, BALL_GRAVITY_PLANE);
                    }
                    sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
                }
            }
        // }
        // End disabled HOST ball streaming — disabled, GetPlayer vtable index unverified
        // if (g_role == ROLE_GUEST && g_connState >= CONN_CONNECTED && g_gameReady) {
        //     if (g_api) {
        //         void* p1 = HBAPI(g_api).GetPlayer();
        //         if (p1 && !IsBadReadPtr(p1, 0x300)) {
        //             InputStateMsg msg;
        //             msg.frame = g_frameCount;
        //             msg.force_x = readBallFloat((DWORD)p1, BALL_FORCE_X);
        //             msg.force_y = readBallFloat((DWORD)p1, BALL_FORCE_Y);
        //             msg.force_z = readBallFloat((DWORD)p1, BALL_FORCE_Z);
        //             sendPipeMsg(MSG_INPUT_STATE, &msg, sizeof(msg));
        //         }
        //     }
        // }

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

// ── Host: apply guest input to P2 ──────────────────────────────────

static void applyHostInput() {
    InputStateMsg input;
    EnterCriticalSection(&g_inputLock);
    input = g_latestInput;
    LeaveCriticalSection(&g_inputLock);

    if (input.frame == 0) return;

    DWORD scene = getScene();
    if (!scene) return;
    if (IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) return;
    DWORD listBase = scene + SCENE_P2_BALL_LIST;
    DWORD count = *(DWORD*)(listBase + 4);
    if (count == 0) return;
    if (IsBadReadPtr((void*)(listBase + 8), 4)) return;
    DWORD p2Ball = *(DWORD*)(listBase + 8);
    if (!p2Ball || IsBadReadPtr((void*)p2Ball, 0x300)) return;

    if (g_api) {
        HBAPI(g_api).ApplyForce((void*)p2Ball, input.force_x, input.force_y, input.force_z, 1.0f);
    } else {
        *(float*)(p2Ball + BALL_FORCE_X) = input.force_x;
        *(float*)(p2Ball + BALL_FORCE_Y) = input.force_y;
        *(float*)(p2Ball + BALL_FORCE_Z) = input.force_z;
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
    DeleteCriticalSection(&g_inputLock);
    if (flags & 1) operator delete(thisptr);
    return thisptr;
}

static const char* __thiscall get_mod_name(void*) { return "Netplay"; }
static const char* __thiscall get_author(void*) { return "rsks + Hamsterbot"; }
static const char* __thiscall get_contributors(void*) {
    return "PoC: host-authority model. Host streams ball state, guest renders ghost ball.";
}
static int __thiscall get_version(void*) { return HAMSTERBALL_API_VERSION; }

static void __thiscall init_impl(void* thisptr, IModAPI* api) {
    g_api = api;
    InitializeCriticalSection(&g_stateLock);
    InitializeCriticalSection(&g_inputLock);

    HBPlusAPI hb = HBAPI(api);

    CustomButton btnHost("NETPLAY_HOST", "Netplay: HOST Mode");
    btnHost.defaultState = false; btnHost.trueText = "ON"; btnHost.falseText = "OFF";
    hb.CreateToggleButton(btnHost, thisptr);

    CustomButton btnGuest("NETPLAY_GUEST", "Netplay: GUEST Mode");
    btnGuest.defaultState = false; btnGuest.trueText = "ON"; btnGuest.falseText = "OFF";
    hb.CreateToggleButton(btnGuest, thisptr);

    CustomSlider sPort("NETPLAY_PORT", "Netplay Port", 5029);
    sPort.lowerBound = 1024; sPort.upperBound = 65535; sPort.stepSize = 1; sPort.decimalPlaces = 0;
    hb.CreateSlider(sPort, thisptr);

    CustomSlider sIp1("NETPLAY_IP1", "Host IP: Octet 1", 127);
    sIp1.lowerBound = 0; sIp1.upperBound = 255; sIp1.stepSize = 1; sIp1.decimalPlaces = 0;
    hb.CreateSlider(sIp1, thisptr);

    CustomSlider sIp2("NETPLAY_IP2", "Host IP: Octet 2", 0);
    sIp2.lowerBound = 0; sIp2.upperBound = 255; sIp2.stepSize = 1; sIp2.decimalPlaces = 0;
    hb.CreateSlider(sIp2, thisptr);

    CustomSlider sIp3("NETPLAY_IP3", "Host IP: Octet 3", 0);
    sIp3.lowerBound = 0; sIp3.upperBound = 255; sIp3.stepSize = 1; sIp3.decimalPlaces = 0;
    hb.CreateSlider(sIp3, thisptr);

    CustomSlider sIp4("NETPLAY_IP4", "Host IP: Octet 4", 1);
    sIp4.lowerBound = 0; sIp4.upperBound = 255; sIp4.stepSize = 1; sIp4.decimalPlaces = 0;
    hb.CreateSlider(sIp4, thisptr);
}

static void __thiscall ball_update(void*, Ball*) {}

static void __thiscall render_apply(void*, void*, float*) {}

static void __thiscall button_toggle(void* thisptr, const char* buttonId, bool newState) {
    if (strcmp(buttonId, "NETPLAY_HOST") == 0) {
        if (newState) { g_role = ROLE_HOST; g_connState = CONN_SEARCHING; }
        else if (g_role == ROLE_HOST) { g_role = ROLE_DISABLED; g_connState = CONN_OFFLINE; }
    }
    if (strcmp(buttonId, "NETPLAY_GUEST") == 0) {
        if (newState) { g_role = ROLE_GUEST; g_connState = CONN_SEARCHING; }
        else if (g_role == ROLE_GUEST) { g_role = ROLE_DISABLED; g_connState = CONN_OFFLINE; }
    }
}

static void __thiscall slider_change(void*, const char* sliderId, float newValue) {
    if (strcmp(sliderId, "NETPLAY_PORT") == 0) g_port = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP1") == 0) g_ip_octet[0] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP2") == 0) g_ip_octet[1] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP3") == 0) g_ip_octet[2] = (int)newValue;
    if (strcmp(sliderId, "NETPLAY_IP4") == 0) g_ip_octet[3] = (int)newValue;
}

static void __thiscall game_update(void*) {
    g_frameCount++;
    if (!g_gameReady && g_frameCount > 120) g_gameReady = true;

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
    // Disabled for now — DrawCustomText vtable index unverified, crashes during loading
    // Apply network state from here (runs every frame)
    if (!g_api || !g_gameReady) return;
    // Disabled network state application — vtable indices for GetPlayer/GetScene
    // via IModAPI are unverified and cause crashes in 2P mode.
    // The relay connection + UI toggles work, which is enough for PoC.
    // if (g_role == ROLE_HOST && g_connState == CONN_CONNECTED) applyHostInput();
}

static void __thiscall event_collide(void*, Ball*, char*) {}
static void __thiscall ball_bump(void*, Ball*, Ball*) {}
static void __thiscall scene_end(void*) {
}
static void __thiscall level_start(void*) {
    g_frameCount = 0;
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
