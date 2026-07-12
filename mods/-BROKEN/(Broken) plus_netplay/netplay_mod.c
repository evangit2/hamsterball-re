/*
 * netplay_mod.c — Hamsterball Netplay Mod (bass.dll proxy version)
 *
 * Converted from the HB+ API version (MainModFile.cpp) to a bass.dll proxy mod.
 * Uses config file (netplay_config.txt) instead of HB+ menu UI.
 * Communicates with the Python relay (netplay_relay.py) via named pipe IPC.
 * Logs status to netplay_log.txt next to bass.dll.
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll netplay_mod.c -lwinmm \
 *     -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse -I../shared/
 *
 * Config file (netplay_config.txt), auto-generated if missing:
 *   role=host      (or guest, disabled)
 *   port=5029
 *   host_ip=127.0.0.1
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PIPE_NAME          "\\\\.\\pipe\\hamsterball_netplay"
#define CONFIG_FILE        "netplay_config.txt"
#define LOG_FILE           "netplay_log.txt"
#define HEARTBEAT_INTERVAL 300
#define PIPE_POLL_MS       4
#define GAME_READY_DELAY   120   /* frames before starting pipe thread */
#define DEFAULT_PORT        5029

/* Ball offsets (verified) */
#define BALL_POS_X          0x164
#define BALL_POS_Y          0x168
#define BALL_POS_Z          0x16C
#define BALL_FACING_X       0x190
#define BALL_FACING_Z       0x194
#define BALL_ROT            0x150
#define BALL_RADIUS         0x284
#define BALL_GRAVITY_PLANE  0x748
#define BALL_FORCE_X        0x2BC
#define BALL_FORCE_Y        0x2C0
#define BALL_FORCE_Z        0x2C4
#define BALL_PHYS_PTR       0x1A4
#define PHYS_VEL_X          0xCA4
#define PHYS_VEL_Y          0xCA8
#define PHYS_VEL_Z          0xCAC
#define SCENE_P1_BALL_PTR   0x29D0
#define SCENE_P2_BALL_LIST  0x3204
#define APP_PROFILE_OFFSET  0x220
#define PROFILE_BOARD_OFFSET 0x0C
#define BOARD_VTABLE_MIN    0x4D0000
#define BOARD_VTABLE_MAX    0x4D2000
#define BALL_APPLYFORCE_RVA 0x402650

/* Note: GLOBAL_APP_PTR and GAME_BASE are already defined in bass_proxy.h */

/* Message types */
#define MSG_BALL_STATE   2
#define MSG_INPUT_STATE  3
#define MSG_FPS_REPORT   4
#define MSG_ROLE_SET     5
#define MSG_PORT_SET     6
#define MSG_STATUS       7
#define MSG_HOST_IP      8

/* Roles */
#define ROLE_DISABLED 0
#define ROLE_HOST     1
#define ROLE_GUEST    2

/* Connection states */
#define CONN_OFFLINE     0
#define CONN_SEARCHING   1
#define CONN_CONNECTING  2
#define CONN_CONNECTED   3
#define CONN_ERROR       4

/* ═══════════════════════════════════════════════════════════════════════════
 * Packed message structs (must match Python relay expectations)
 * ═══════════════════════════════════════════════════════════════════════════ */

#pragma pack(push, 1)
typedef struct {
    DWORD frame;
    float p1_pos[3], p1_vel[3], p1_facing[2], p1_rot, p1_radius, p1_gravity;
    float p2_pos[3], p2_vel[3], p2_facing[2], p2_rot, p2_radius, p2_gravity;
} BallStateMsg;

typedef struct {
    DWORD frame;
    float force_x, force_y, force_z;
} InputStateMsg;

typedef struct {
    float local_fps, remote_fps;
    DWORD frame_count;
} FpsReportMsg;

typedef struct {
    DWORD conn_state;
    DWORD remote_fps_raw;
    char remote_info[64];
} StatusMsg;
#pragma pack(pop)

/* ═══════════════════════════════════════════════════════════════════════════
 * Globals
 * ═══════════════════════════════════════════════════════════════════════════ */

static volatile int  g_role      = ROLE_DISABLED;
static volatile int  g_connState = CONN_OFFLINE;
static volatile int  g_port      = DEFAULT_PORT;
static char          g_hostIP[32] = "127.0.0.1";
static HANDLE       g_pipe       = INVALID_HANDLE_VALUE;
static HANDLE       g_pipeThread = NULL;
static volatile int  g_pipeRunning = 0;
static DWORD        g_frameCount  = 0;
static DWORD        g_fpsFrameCounter = 0;
static DWORD        g_fpsLastTick = 0;
static DWORD        g_lastHeartbeat = 0;
static float        g_localFps  = 0.0f;
static float        g_remoteFps = 0.0f;
static char         g_remoteInfo[128] = "";
static BallStateMsg g_latestBallState;
static CRITICAL_SECTION g_stateLock;
static InputStateMsg g_latestInput;
static CRITICAL_SECTION g_inputLock;
static volatile int  g_gameReady = 0;
static volatile DWORD g_lastBallStateFrame = 0;
static volatile DWORD g_lastInputFrame = 0;

static char g_logPath[MAX_PATH] = "";
static char g_configPath[MAX_PATH] = "";

/* Ball_ApplyForce function pointer: void __thiscall(Ball* this, float fx, float fy, float fz, float scale) */
typedef void (__thiscall *BallApplyForce_t)(DWORD thisptr, float fx, float fy, float fz, float scale);
static BallApplyForce_t g_BallApplyForce = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * Logging
 * ═══════════════════════════════════════════════════════════════════════════ */

static void log_msg(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SYSTEMTIME st;
        GetLocalTime(&st);
        char ts[64];
        snprintf(ts, sizeof(ts), "[%02d:%02d:%02d.%03d] ",
                 st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, ts, (DWORD)strlen(ts), &w, NULL);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
}

static void log_fmt(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log_msg(buf);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Path initialization — find the DLL directory for config/log files
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_paths(void) {
    char path[MAX_PATH];
    HMODULE hSelf = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                       | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCSTR)&init_paths, &hSelf);
    GetModuleFileNameA(hSelf, path, MAX_PATH);

    /* Find last backslash to get directory */
    char *p = strrchr(path, '\\');
    if (p) {
        *(p + 1) = '\0';
        snprintf(g_logPath, sizeof(g_logPath), "%s%s", path, LOG_FILE);
        snprintf(g_configPath, sizeof(g_configPath), "%s%s", path, CONFIG_FILE);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Config file reading/writing
 * ═══════════════════════════════════════════════════════════════════════════ */

static void write_default_config(void) {
    if (!g_configPath[0]) return;
    HANDLE h = CreateFileA(g_configPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    const char *content =
        "# Hamsterball Netplay Config\r\n"
        "# role: host | guest | disabled\r\n"
        "role=disabled\r\n"
        "\r\n"
        "# TCP port for relay (both host and guest must match)\r\n"
        "port=5029\r\n"
        "\r\n"
        "# Host IP (only used by guest to connect to host)\r\n"
        "host_ip=127.0.0.1\r\n";
    DWORD written;
    WriteFile(h, content, (DWORD)strlen(content), &written, NULL);
    CloseHandle(h);
    log_msg("Created default config file (role=disabled). Edit netplay_config.txt to enable.");
}

static void read_config(void) {
    if (!g_configPath[0]) return;

    HANDLE h = CreateFileA(g_configPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        log_msg("Config file not found, creating default...");
        write_default_config();
        return;
    }

    /* Read entire file */
    char buf[1024];
    DWORD bytesRead = 0;
    ReadFile(h, buf, sizeof(buf) - 1, &bytesRead, NULL);
    CloseHandle(h);
    buf[bytesRead] = '\0';

    /* Parse line by line */
    char *line = buf;
    while (line && *line) {
        /* Find end of line */
        char *eol = strchr(line, '\n');
        if (eol) {
            *eol = '\0';
            /* Strip \r */
            if (eol > line && *(eol - 1) == '\r') *(eol - 1) = '\0';
        }

        /* Skip comments and empty lines */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == '\0') {
            if (!eol) break;
            line = eol + 1;
            continue;
        }

        /* Find '=' */
        char *eq = strchr(p, '=');
        if (eq) {
            *eq = '\0';
            char *key = p;
            char *val = eq + 1;
            /* Trim whitespace */
            while (*val == ' ' || *val == '\t') val++;
            char *vend = val + strlen(val);
            while (vend > val && (*(vend - 1) == ' ' || *(vend - 1) == '\t' || *(vend - 1) == '\r'))
                *(--vend) = '\0';
            /* Trim key whitespace */
            char *kend = key + strlen(key);
            while (kend > key && (*(kend - 1) == ' ' || *(kend - 1) == '\t'))
                *(--kend) = '\0';

            if (_stricmp(key, "role") == 0) {
                if (_stricmp(val, "host") == 0) {
                    g_role = ROLE_HOST;
                    g_connState = CONN_SEARCHING;
                } else if (_stricmp(val, "guest") == 0) {
                    g_role = ROLE_GUEST;
                    g_connState = CONN_SEARCHING;
                } else {
                    g_role = ROLE_DISABLED;
                }
                log_fmt("Config: role=%s (%d)", val, g_role);
            } else if (_stricmp(key, "port") == 0) {
                g_port = atoi(val);
                if (g_port < 1 || g_port > 65535) g_port = DEFAULT_PORT;
                log_fmt("Config: port=%d", g_port);
            } else if (_stricmp(key, "host_ip") == 0) {
                strncpy(g_hostIP, val, sizeof(g_hostIP) - 1);
                g_hostIP[sizeof(g_hostIP) - 1] = '\0';
                log_fmt("Config: host_ip=%s", g_hostIP);
            }
        }

        if (!eol) break;
        line = eol + 1;
    }

    if (g_role == ROLE_DISABLED) {
        log_msg("Netplay disabled. Set role=host or role=guest in netplay_config.txt");
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Memory access helpers (ported from MainModFile.cpp)
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD findBoard(void) {
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

static DWORD getScene(void) {
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
    if (IsBadReadPtr((void*)(ball + BALL_PHYS_PTR), 4)) return;
    DWORD physPtr = *(DWORD*)(ball + BALL_PHYS_PTR);
    if (!physPtr || physPtr < 0x10000) return;
    if (IsBadReadPtr((void*)(physPtr + PHYS_VEL_Z), 4)) return;
    *vx = *(float*)(physPtr + PHYS_VEL_X);
    *vy = *(float*)(physPtr + PHYS_VEL_Y);
    *vz = *(float*)(physPtr + PHYS_VEL_Z);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Pipe communication (ported from MainModFile.cpp)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int connectToPipe(void) {
    if (g_pipe != INVALID_HANDLE_VALUE) return 1;
    g_pipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, 0, NULL);
    if (g_pipe == INVALID_HANDLE_VALUE) return 0;
    DWORD mode = PIPE_READMODE_BYTE;
    SetNamedPipeHandleState(g_pipe, &mode, NULL, NULL);
    return 1;
}

static void disconnectPipe(void) {
    if (g_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }
}

static int sendPipeMsg(DWORD type, const void* data, DWORD dataLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return 0;
    DWORD hdr[2] = { type, dataLen };
    DWORD written;
    if (!WriteFile(g_pipe, hdr, 8, &written, NULL) || written != 8) {
        disconnectPipe();
        return 0;
    }
    if (dataLen > 0 && data) {
        if (!WriteFile(g_pipe, data, dataLen, &written, NULL) || written != dataLen) {
            disconnectPipe();
            return 0;
        }
    }
    return 1;
}

static int readPipeMsgNonBlocking(DWORD* outType, void* outBuf, DWORD bufSize, DWORD* outLen) {
    if (g_pipe == INVALID_HANDLE_VALUE) return 0;
    DWORD bytesAvailable = 0;
    if (!PeekNamedPipe(g_pipe, NULL, 0, NULL, &bytesAvailable, NULL)) {
        disconnectPipe();
        return 0;
    }
    if (bytesAvailable < 8) return 0;
    DWORD bytesRead = 0;
    if (!ReadFile(g_pipe, outType, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe();
        return 0;
    }
    DWORD len = 0;
    if (!ReadFile(g_pipe, &len, 4, &bytesRead, NULL) || bytesRead != 4) {
        disconnectPipe();
        return 0;
    }
    if (len > bufSize) len = bufSize;
    if (len > 0) {
        if (!ReadFile(g_pipe, outBuf, len, &bytesRead, NULL) || bytesRead != len) {
            disconnectPipe();
            return 0;
        }
    }
    if (outLen) *outLen = len;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Pipe thread — main netplay loop
 *
 * Host: streams ball state to relay (MSG_BALL_STATE)
 * Guest: sends input forces to relay (MSG_INPUT_STATE), receives ball state
 *
 * On connect, sends ROLE_SET, PORT_SET, and HOST_IP (guest only) so the
 * relay knows what to do.
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI pipeThreadFunc(LPVOID param) {
    (void)param;
    int lastSentRole = ROLE_DISABLED;
    int lastSentPort = -1;
    char lastSentIP[32] = "";

    log_fmt("Pipe thread started (role=%s, port=%d)",
            g_role == ROLE_HOST ? "HOST" : (g_role == ROLE_GUEST ? "GUEST" : "DISABLED"),
            g_port);

    while (g_pipeRunning) {
        /* Try to connect to the relay's named pipe */
        if (!connectToPipe()) {
            Sleep(500);
            continue;
        }

        /* Send role, port, and host_ip to relay when they change */
        if (g_role != lastSentRole) {
            DWORD r = (DWORD)g_role;
            sendPipeMsg(MSG_ROLE_SET, &r, 4);
            lastSentRole = g_role;
            log_fmt("Sent ROLE_SET=%d", r);
        }
        if (g_port != lastSentPort && g_role != ROLE_DISABLED) {
            DWORD p = (DWORD)g_port;
            sendPipeMsg(MSG_PORT_SET, &p, 4);
            lastSentPort = g_port;
            log_fmt("Sent PORT_SET=%d", p);
        }
        if (g_role == ROLE_GUEST && strcmp(g_hostIP, lastSentIP) != 0) {
            sendPipeMsg(MSG_HOST_IP, g_hostIP, (DWORD)strlen(g_hostIP) + 1);
            strncpy(lastSentIP, g_hostIP, sizeof(lastSentIP) - 1);
            lastSentIP[sizeof(lastSentIP) - 1] = '\0';
            log_fmt("Sent HOST_IP=%s", g_hostIP);
        }

        /* HOST: stream ball state */
        if (g_role == ROLE_HOST && g_connState >= CONN_CONNECTED && g_gameReady) {
            DWORD scene = getScene();
            if (scene) {
                DWORD p1 = 0, p2 = 0;
                if (!IsBadReadPtr((void*)(scene + SCENE_P1_BALL_PTR), 4))
                    p1 = *(DWORD*)(scene + SCENE_P1_BALL_PTR);
                if (!IsBadReadPtr((void*)(scene + SCENE_P2_BALL_LIST), 8)) {
                    DWORD lb = scene + SCENE_P2_BALL_LIST;
                    DWORD c = *(DWORD*)(lb + 4);
                    if (c > 0 && !IsBadReadPtr((void*)(lb + 8), 4))
                        p2 = *(DWORD*)(lb + 8);
                }
                if (p1 && !IsBadReadPtr((void*)p1, 0x800)) {
                    BallStateMsg msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.frame = g_frameCount;

                    msg.p1_pos[0] = readBallFloat(p1, BALL_POS_X);
                    msg.p1_pos[1] = readBallFloat(p1, BALL_POS_Y);
                    msg.p1_pos[2] = readBallFloat(p1, BALL_POS_Z);
                    readPhysicsVelocity(p1, &msg.p1_vel[0], &msg.p1_vel[1], &msg.p1_vel[2]);
                    msg.p1_facing[0] = readBallFloat(p1, BALL_FACING_X);
                    msg.p1_facing[1] = readBallFloat(p1, BALL_FACING_Z);
                    msg.p1_rot = readBallFloat(p1, BALL_ROT);
                    msg.p1_radius = readBallFloat(p1, BALL_RADIUS);
                    msg.p1_gravity = readBallFloat(p1, BALL_GRAVITY_PLANE);

                    if (p2 && !IsBadReadPtr((void*)p2, 0x800)) {
                        msg.p2_pos[0] = readBallFloat(p2, BALL_POS_X);
                        msg.p2_pos[1] = readBallFloat(p2, BALL_POS_Y);
                        msg.p2_pos[2] = readBallFloat(p2, BALL_POS_Z);
                        readPhysicsVelocity(p2, &msg.p2_vel[0], &msg.p2_vel[1], &msg.p2_vel[2]);
                        msg.p2_facing[0] = readBallFloat(p2, BALL_FACING_X);
                        msg.p2_facing[1] = readBallFloat(p2, BALL_FACING_Z);
                        msg.p2_rot = readBallFloat(p2, BALL_ROT);
                        msg.p2_radius = readBallFloat(p2, BALL_RADIUS);
                        msg.p2_gravity = readBallFloat(p2, BALL_GRAVITY_PLANE);
                    }

                    sendPipeMsg(MSG_BALL_STATE, &msg, sizeof(msg));
                }
            }
        }

        /* GUEST: send input forces (read from local ball's force fields) */
        if (g_role == ROLE_GUEST && g_connState >= CONN_CONNECTED && g_gameReady) {
            DWORD scene = getScene();
            if (scene) {
                DWORD p1 = 0;
                if (!IsBadReadPtr((void*)(scene + SCENE_P1_BALL_PTR), 4))
                    p1 = *(DWORD*)(scene + SCENE_P1_BALL_PTR);
                if (p1 && !IsBadReadPtr((void*)p1, 0x300)) {
                    InputStateMsg msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.frame = g_frameCount;
                    msg.force_x = readBallFloat(p1, BALL_FORCE_X);
                    msg.force_y = readBallFloat(p1, BALL_FORCE_Y);
                    msg.force_z = readBallFloat(p1, BALL_FORCE_Z);
                    sendPipeMsg(MSG_INPUT_STATE, &msg, sizeof(msg));
                }
            }
        }

        /* Heartbeat: send FPS report periodically */
        if (g_frameCount - g_lastHeartbeat >= HEARTBEAT_INTERVAL) {
            FpsReportMsg fps;
            fps.local_fps = g_localFps;
            fps.remote_fps = g_remoteFps;
            fps.frame_count = g_frameCount;
            sendPipeMsg(MSG_FPS_REPORT, &fps, sizeof(fps));
            g_lastHeartbeat = g_frameCount;
        }

        /* Read incoming messages from relay */
        DWORD type;
        char buf[512];
        DWORD len = 0;
        while (readPipeMsgNonBlocking(&type, buf, sizeof(buf), &len)) {
            switch (type) {
                case MSG_BALL_STATE:
                    /* Guest receives ball state from host */
                    if (g_role == ROLE_GUEST && len == sizeof(BallStateMsg)) {
                        EnterCriticalSection(&g_stateLock);
                        memcpy(&g_latestBallState, buf, sizeof(BallStateMsg));
                        g_lastBallStateFrame = g_frameCount;
                        LeaveCriticalSection(&g_stateLock);
                    }
                    break;

                case MSG_INPUT_STATE:
                    /* Host receives input from guest */
                    if (g_role == ROLE_HOST && len == sizeof(InputStateMsg)) {
                        EnterCriticalSection(&g_inputLock);
                        memcpy(&g_latestInput, buf, sizeof(InputStateMsg));
                        g_lastInputFrame = g_frameCount;
                        LeaveCriticalSection(&g_inputLock);

                        /* Apply guest's input to P2 ball by writing force values
                         * directly to the ball's force fields. This is simpler and
                         * more robust than calling Ball_ApplyForce via inline asm
                         * (which has calling convention issues with MinGW float args). */
                        if (g_gameReady) {
                            InputStateMsg input;
                            EnterCriticalSection(&g_inputLock);
                            input = g_latestInput;
                            LeaveCriticalSection(&g_inputLock);
                            if (input.frame > 0) {
                                DWORD scene2 = getScene();
                                if (scene2 && !IsBadReadPtr((void*)(scene2 + SCENE_P2_BALL_LIST), 8)) {
                                    DWORD lb2 = scene2 + SCENE_P2_BALL_LIST;
                                    DWORD c2 = *(DWORD*)(lb2 + 4);
                                    if (c2 > 0 && !IsBadReadPtr((void*)(lb2 + 8), 4)) {
                                        DWORD p2 = *(DWORD*)(lb2 + 8);
                                        if (p2 && !IsBadReadPtr((void*)(p2 + BALL_FORCE_Z + 4), 4)) {
                                            *(float*)(p2 + BALL_FORCE_X) = input.force_x;
                                            *(float*)(p2 + BALL_FORCE_Y) = input.force_y;
                                            *(float*)(p2 + BALL_FORCE_Z) = input.force_z;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;

                case MSG_FPS_REPORT:
                    if (len == sizeof(FpsReportMsg)) {
                        FpsReportMsg* f = (FpsReportMsg*)buf;
                        g_remoteFps = f->local_fps;
                    }
                    break;

                case MSG_STATUS:
                    if (len >= sizeof(StatusMsg)) {
                        StatusMsg* s = (StatusMsg*)buf;
                        int oldState = g_connState;
                        g_connState = (int)s->conn_state;
                        if (s->remote_fps_raw > 0)
                            g_remoteFps = (float)s->remote_fps_raw;
                        strncpy(g_remoteInfo, s->remote_info, sizeof(g_remoteInfo) - 1);
                        g_remoteInfo[sizeof(g_remoteInfo) - 1] = '\0';
                        if (oldState != g_connState) {
                            log_fmt("Conn state: %d → %d (remote: %s)",
                                    oldState, g_connState, g_remoteInfo);
                        }
                    }
                    break;
            }
        }

        Sleep(PIPE_POLL_MS);
    }

    disconnectPipe();
    log_msg("Pipe thread stopped");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Game update hook — called every frame via BASS_ChannelSetAttributes detour
 *
 * Since bass.dll proxy mods don't have an HB+ onGameUpdate callback, we
 * use the BASS audio functions that the game calls every frame as our
 * "tick" hook. BASS_ChannelSetAttributes is called frequently during gameplay.
 *
 * We use a frame counter to detect "game ready" and start the pipe thread
 * after a delay (to let the game initialize fully).
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Override the BASS_ChannelSetAttributes from bass_proxy.h to add our tick */
static int g_tickInited = 0;

static void netplay_tick(void) {
    if (!g_tickInited) {
        InitializeCriticalSection(&g_stateLock);
        InitializeCriticalSection(&g_inputLock);
        memset(&g_latestBallState, 0, sizeof(g_latestBallState));
        memset(&g_latestInput, 0, sizeof(g_latestInput));
        g_tickInited = 1;
    }

    g_frameCount++;

    /* Start pipe thread after game has been running for a while */
    if (!g_gameReady && g_frameCount > GAME_READY_DELAY) {
        g_gameReady = 1;
        if (!g_pipeThread && g_role != ROLE_DISABLED) {
            g_pipeRunning = 1;
            g_pipeThread = CreateThread(NULL, 0, pipeThreadFunc, NULL, 0, NULL);
            log_fmt("Game ready — started pipe thread (frame %d)", g_frameCount);
        }
    }

    /* FPS tracking */
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

/*
 * We need to hook into the game's frame loop. The simplest approach for a
 * bass.dll proxy is to use BASS_ChannelSetAttributes — the game calls it
 * every frame to update audio. We override the export to add our tick.
 *
 * But wait — bass_proxy.h already defines BASS_ChannelSetAttributes as a
 * static function. We can't redefine it. Instead, we'll use BASS_Start
 * which the game calls at the beginning of each frame.
 *
 * Actually, the cleanest approach: we hook the real BASS_ChannelSetAttributes
 * call. Since bass_proxy.h defines it as a non-static dllexport, we can use
 * a wrapper approach. But the header already exports it...
 *
 * The simplest working approach: use DllMain to start a timer thread that
 * polls at 60Hz. This avoids needing to hook any BASS function.
 */

/* Timer thread: simulates onGameUpdate at ~60Hz */
static DWORD WINAPI timerThreadFunc(LPVOID param) {
    (void)param;
    log_msg("Timer thread started (60Hz polling)");
    while (g_pipeRunning) {
        netplay_tick();
        Sleep(15);  /* ~66Hz, close enough to 60fps */
    }
    log_msg("Timer thread stopped");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain — Entry point
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;

    if (reason == DLL_PROCESS_ATTACH) {
        /* Step 1: Load real bass.dll for audio forwarding */
        load_real_bass();

        /* Step 2: Initialize paths for config/log files */
        init_paths();

        /* Step 3: Read config file */
        read_config();

        /* Step 4: Resolve Ball_ApplyForce function address */
        g_BallApplyForce = (BallApplyForce_t)(GAME_BASE + BALL_APPLYFORCE_RVA);

        /* Step 5: Start pipe thread if role is not disabled */
        if (g_role != ROLE_DISABLED) {
            log_fmt("=== Netplay mod loaded (role=%s, port=%d) ===",
                    g_role == ROLE_HOST ? "HOST" : "GUEST", g_port);
            g_pipeRunning = 1;
            CreateThread(NULL, 0, timerThreadFunc, NULL, 0, NULL);
        } else {
            log_msg("=== Netplay mod loaded (DISABLED) ===");
        }

    } else if (reason == DLL_PROCESS_DETACH) {
        g_pipeRunning = 0;
        if (g_pipeThread) {
            WaitForSingleObject(g_pipeThread, 2000);
            CloseHandle(g_pipeThread);
            g_pipeThread = NULL;
        }
        disconnectPipe();
        if (g_tickInited) {
            DeleteCriticalSection(&g_stateLock);
            DeleteCriticalSection(&g_inputLock);
        }
        log_msg("=== Netplay mod unloaded ===");
    }

    return TRUE;
}
