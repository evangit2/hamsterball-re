#define _CRT_SECURE_NO_WARNINGS
#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static const char* CONFIG_FILE = "discord_rpc.txt";
static char g_appId[64] = "";
static char g_largeImage[64] = "hamsterball";

static HANDLE g_pipe = INVALID_HANDLE_VALUE;
static bool g_connected = false;

static const char* RACE_NAMES[16] = {
    "Warm-Up Race", "Beginner Race", "Intermediate Race", "Dizzy Race",
    "Tower Race", "Up Race", "Neon Race", "Expert Race",
    "Odd Race", "Toob Race", "Wobbly Race", "Glass Race",
    "Sky Race", "Master Race", "Impossible Race", "Bug Race"
};

struct GameState {
    bool inLevel;
    int raceIdx;
    int playerCount;
    time_t levelStartTime;
    char levelName[128];
};

static GameState g_currentState = {};
static GameState g_lastSentState = {};
static time_t g_lastSendTime = 0;
static bool g_forceUpdate = false;
static bool g_rpcEnabled = true;
static bool g_running = true;

static bool SendFrame(DWORD opcode, const char* json) {
    if (g_pipe == INVALID_HANDLE_VALUE) return false;
    DWORD len = (DWORD)strlen(json);
    size_t totalLen = 8 + len;
    char* buf = (char*)malloc(totalLen);
    if (!buf) return false;
    memcpy(buf, &opcode, 4);
    memcpy(buf + 4, &len, 4);
    memcpy(buf + 8, json, len);
    DWORD written;
    BOOL ok = WriteFile(g_pipe, buf, (DWORD)totalLen, &written, NULL);
    free(buf);
    return ok && written == totalLen;
}

static bool Connect() {
    for (int i = 0; i < 10; i++) {
        char path[64];
        snprintf(path, sizeof(path), "\\\\.\\pipe\\discord-ipc-%d", i);
        g_pipe = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (g_pipe != INVALID_HANDLE_VALUE) {
            char json[256];
            snprintf(json, sizeof(json), "{\"v\":1,\"client_id\":\"%s\"}", g_appId);
            if (SendFrame(1, json)) {
                g_connected = true;
                return true;
            }
            CloseHandle(g_pipe);
            g_pipe = INVALID_HANDLE_VALUE;
        }
    }
    return false;
}

static void Disconnect() {
    if (g_pipe != INVALID_HANDLE_VALUE) {
        CloseHandle(g_pipe);
        g_pipe = INVALID_HANDLE_VALUE;
    }
    g_connected = false;
}

static bool SendActivity(const char* state, const char* details, time_t startTime) {
    if (!g_connected) return false;
    char json[1024];
    if (startTime > 0) {
        snprintf(json, sizeof(json),
            "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%d,\"activity\":{"
            "\"state\":\"%s\",\"details\":\"%s\","
            "\"timestamps\":{\"start\":%lld},"
            "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"Hamsterball\"}"
            "}},\"nonce\":\"hb-%lld\"}",
            (int)GetCurrentProcessId(), state, details,
            (long long)startTime, g_largeImage, (long long)time(NULL));
    } else {
        snprintf(json, sizeof(json),
            "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%d,\"activity\":{"
            "\"state\":\"%s\",\"details\":\"%s\","
            "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"Hamsterball\"}"
            "}},\"nonce\":\"hb-%lld\"}",
            (int)GetCurrentProcessId(), state, details,
            g_largeImage, (long long)time(NULL));
    }
    return SendFrame(2, json);
}

static bool ClearActivity() {
    if (!g_connected) return false;
    char json[256];
    snprintf(json, sizeof(json),
        "{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":%d},\"nonce\":\"hb-%lld\"}",
        (int)GetCurrentProcessId(), (long long)time(NULL));
    return SendFrame(2, json);
}

static void LoadConfig() {
    char dir[512];
    GetCurrentDirectoryA(512, dir);
    char path[768];
    snprintf(path, sizeof(path), "%s\\%s", dir, CONFIG_FILE);
    FILE* f = NULL;
    if (fopen_s(&f, path, "r") == 0 && f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            char key[128], val[128];
            if (sscanf(line, "%127[^=]=%127[^\n]", key, val) == 2) {
                char* k = key;
                while (*k == ' ' || *k == '\t') k++;
                char* end = k + strlen(k) - 1;
                while (end > k && (*end == ' ' || *end == '\t')) *end-- = 0;
                char* v = val;
                while (*v == ' ' || *v == '\t') v++;
                if (_stricmp(k, "app_id") == 0) {
                    strncpy_s(g_appId, sizeof(g_appId), v, _TRUNCATE);
                } else if (_stricmp(k, "large_image") == 0) {
                    strncpy_s(g_largeImage, sizeof(g_largeImage), v, _TRUNCATE);
                }
            }
        }
        fclose(f);
        return;
    }
    if (fopen_s(&f, path, "w") == 0 && f) {
        fprintf(f, "# Discord Rich Presence Configuration\n");
        fprintf(f, "# Get your Application ID from https://discord.com/developers/applications\n");
        fprintf(f, "app_id=YOUR_APP_ID_HERE\n");
        fprintf(f, "# Art asset key name uploaded to Discord Developer Portal\n");
        fprintf(f, "large_image=hamsterball\n");
        fclose(f);
    }
}

static void ReadGameState(IModAPI* api) {
    memset(&g_currentState, 0, sizeof(g_currentState));

    Scene* scene = api->GetScene();
    if (!scene) return;

    if (scene->name && !IsBadReadPtr(scene->name, 1)) {
        strncpy_s(g_currentState.levelName, sizeof(g_currentState.levelName), scene->name, _TRUNCATE);
    }

    char* appBase = (char*)api->GetApp();
    if (appBase && !IsBadReadPtr(appBase + 0x220, 4)) {
        DWORD profile = *(DWORD*)(appBase + 0x220);
        if (profile && profile > 0x10000 && !IsBadReadPtr((char*)profile + 0x08, 4)) {
            g_currentState.raceIdx = *(int*)((char*)profile + 0x08);
        }
    }

    int pc = 1;
    if (api->GetPlayer2()) pc++;
    if (api->GetPlayer3()) pc++;
    if (api->GetPlayer4()) pc++;
    g_currentState.playerCount = pc;

    g_currentState.inLevel = true;
}

static bool StateChanged() {
    if (g_forceUpdate) return true;
    if (g_currentState.inLevel != g_lastSentState.inLevel) return true;
    if (g_currentState.raceIdx != g_lastSentState.raceIdx) return true;
    if (g_currentState.playerCount != g_lastSentState.playerCount) return true;
    if (strcmp(g_currentState.levelName, g_lastSentState.levelName) != 0) return true;
    return false;
}

static void BuildPresence(char* outState, int stateLen, char* outDetails, int detailsLen, time_t* outStartTime) {
    *outStartTime = 0;

    if (!g_currentState.inLevel) {
        strncpy_s(outState, stateLen, "In Menu", _TRUNCATE);
        strncpy_s(outDetails, detailsLen, "Hamsterball", _TRUNCATE);
        return;
    }

    *outStartTime = g_currentState.levelStartTime;

    if (g_currentState.playerCount > 1) {
        snprintf(outState, stateLen, "Arena - %dP", g_currentState.playerCount);
    } else {
        int raceNum = g_currentState.raceIdx + 1;
        if (raceNum >= 1 && raceNum <= 16) {
            snprintf(outState, stateLen, "Race %d/16", raceNum);
        } else {
            strncpy_s(outState, stateLen, "Time Trial", _TRUNCATE);
        }
    }

    const char* raceName = "Custom Level";
    if (g_currentState.raceIdx >= 0 && g_currentState.raceIdx < 16) {
        raceName = RACE_NAMES[g_currentState.raceIdx];
    }

    if (g_currentState.levelName[0] != '\0' &&
        strncmp(g_currentState.levelName, "Level", 5) != 0 &&
        strncmp(g_currentState.levelName, "Arena", 5) != 0) {
        snprintf(outDetails, detailsLen, "%s", g_currentState.levelName);
    } else {
        snprintf(outDetails, detailsLen, "%s", raceName);
    }
}

static DWORD WINAPI DiscordThread(LPVOID param) {
    IModAPI* api = (IModAPI*)param;

    LoadConfig();

    if (strlen(g_appId) == 0 || strcmp(g_appId, "YOUR_APP_ID_HERE") == 0) {
        return 0;
    }

    time_t lastReconnectAttempt = 0;

    while (g_running) {
        if (!g_rpcEnabled) {
            if (g_connected) {
                ClearActivity();
                Disconnect();
            }
            Sleep(2000);
            continue;
        }

        if (!g_connected) {
            time_t now = time(NULL);
            if (now - lastReconnectAttempt >= 15) {
                lastReconnectAttempt = now;
                Connect();
            }
            Sleep(2000);
            continue;
        }

        ReadGameState(api);

        time_t now = time(NULL);
        bool shouldUpdate = StateChanged();

        if (shouldUpdate) {
            if (g_currentState.inLevel && !g_lastSentState.inLevel) {
                if (g_currentState.levelStartTime == 0) {
                    g_currentState.levelStartTime = now;
                }
            }

            if (!g_currentState.inLevel && g_lastSentState.inLevel) {
                ClearActivity();
                g_lastSentState = g_currentState;
                g_lastSendTime = now;
                g_forceUpdate = false;
                Sleep(2000);
                continue;
            }
        }

        if (shouldUpdate && (now - g_lastSendTime >= 15 || g_lastSendTime == 0)) {
            char stateStr[128], detailsStr[128];
            time_t startTime = 0;
            BuildPresence(stateStr, sizeof(stateStr), detailsStr, sizeof(detailsStr), &startTime);

            SendActivity(stateStr, detailsStr, startTime);

            g_lastSentState = g_currentState;
            g_lastSendTime = now;
            g_forceUpdate = false;
        }

        DWORD avail = 0;
        if (PeekNamedPipe(g_pipe, NULL, 0, NULL, &avail, NULL)) {
            if (avail > 0) {
                char readBuf[4096];
                DWORD read = 0;
                if (!ReadFile(g_pipe, readBuf, sizeof(readBuf), &read, NULL)) {
                    Disconnect();
                }
            }
        } else {
            Disconnect();
        }

        Sleep(1000);
    }

    if (g_connected) {
        ClearActivity();
    }
    Disconnect();
    return 0;
}

class DiscordRPCMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    HANDLE thread = NULL;
public:
    const char* GetModName() override    { return "Discord Rich Presence"; }
    const char* GetAuthorName() override { return "Hamsterbot"; }
    int GetApiVersion() override         { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;
        CustomButton btn("discord_rpc", "Discord Rich Presence");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);
        g_running = true;
        thread = CreateThread(NULL, 0, DiscordThread, modApi, 0, NULL);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "discord_rpc") == 0) {
            g_rpcEnabled = newState;
            g_forceUpdate = true;
        }
    }

    void onLevelStart() override {
        g_currentState.levelStartTime = time(NULL);
        g_forceUpdate = true;
    }

    void onSceneEnd() override {
        g_forceUpdate = true;
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new DiscordRPCMod();
}
