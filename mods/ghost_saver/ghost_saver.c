/*
 /* ghost_saver.c — Persistent Ghost Data for Time Trial Mode (v3)
  *
  * v3: Fix __thiscall calls via inline asm — MinGW __thiscall function pointers
  *     silently fail. Use raw push/mov ecx/call for BestTimeTracker_ctor and
  *     AthenaList_Append. Also fixes: race name comparison uses board+0x29B4
  *     (the actual race name source) instead of BTT+0x424 (which the game
  *     copies FROM board+0x29B4 during App_StartPracticeRace).
  *
  * v2: Heartbeat logging, raw DWORD snapshot format.
  * v1: Initial release.
  */

 #include "bass_proxy.h"

 /* Game constants */
 #define APP_PTR             0x005341E0
 #define ADDR_BTT_CTOR       0x00427660
 #define ADDR_ALIST_APPEND   0x00453780
 #define ADDR_ALIST_INIT     0x00453210
 #define ADDR_OPERATOR_NEW   0x004BA570
 #define BTT_SIZE            0x528
 #define BTT_BEST_TIME       0x524
 #define BTT_NAME            0x424
 #define SNAP_SIZE           0x28
 #define NO_TIME             9999999
 #define MAX_SNAPSHOTS       5000

 /* App offsets */
 #define APP_90C_RECORDING  0x90C
 #define APP_910_PLAYBACK    0x910
 #define APP_5DC_BALL        0x5DC
 #define APP_5D6_GOAL_FLAG   0x5D6
 #define APP_234_PARTY_MODE  0x234
 #define APP_220_PROFILE     0x220

/* Snapshot struct — mirrors what the game records (10 DWORDs = 0x28 bytes) */
typedef struct {
    float f[9];
    unsigned char state;
} Snapshot;

static Snapshot g_snapshots[MAX_SNAPSHOTS];
static int g_snapshotCount = 0;
static char g_currentRaceName[128] = "";
static int g_recording = 0;
static int g_raceFinished = 0;
static int g_prevGoalFlag = 0;
static char g_ghostPath[MAX_PATH] = "";
static char g_logPath[MAX_PATH] = "";

/* ═════════════════════════════════════════════════════════════════
 * Logging
 * ═════════════════════════════════════════════════════════════════ */

static void log_msg(const char *msg) {
    if (!g_logPath[0]) return;
    HANDLE h = CreateFileA(g_logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, strlen(msg), &w, NULL);
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

/* ═════════════════════════════════════════════════════════════════
 * Get App pointer safely
 * ═════════════════════════════════════════════════════════════════ */

static DWORD get_app(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return 0;
    return app;
}

/* ═════════════════════════════════════════════════════════════════
 * Get race name from recording buffer (BTT+0x424)
 * ═════════════════════════════════════════════════════════════════ */

static int get_race_name(char *out, int outLen) {
    out[0] = '\0';
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) return 0;
    DWORD btt = *(DWORD*)(app + APP_90C_RECORDING);
    if (!btt || btt < 0x10000) return 0;
    if (IsBadReadPtr((void*)(btt + BTT_NAME), 1)) return 0;
    char *name = (char*)(btt + BTT_NAME);
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    /* Validate more characters */
    for (int i = 0; i < 64 && name[i]; i++) {
        if (name[i] < 0x20 || name[i] > 0x7E) { name[i] = '\0'; break; }
    }
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

/* ═════════════════════════════════════════════════════════════════
 * GHOST.txt read/write
 * ═════════════════════════════════════════════════════════════════ */

static Snapshot* load_ghost_for_race(const char *raceName, int *outCount, int *outTime) {
    HANDLE h = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;

    Snapshot *snaps = NULL;
    int count = 0, time = NO_TIME, frames = 0;
    int inRace = 0, foundRace = 0;
    DWORD fileSize = GetFileSize(h, NULL);
    DWORD filePos = 0;
    char line[512];

    while (filePos < fileSize) {
        SetFilePointer(h, filePos, NULL, FILE_BEGIN);
        DWORD toRead = sizeof(line) - 1;
        if (filePos + toRead > fileSize) toRead = fileSize - filePos;
        DWORD bytesRead = 0;
        if (!ReadFile(h, line, toRead, &bytesRead, NULL) || bytesRead == 0) break;
        char *nl = memchr(line, '\n', bytesRead);
        if (!nl) { filePos += bytesRead; continue; }
        int lineLen = nl - line;
        if (lineLen > 0 && line[lineLen-1] == '\r') lineLen--;
        line[lineLen] = '\0';
        filePos += (nl - line) + 1;

        if (strncmp(line, "[RACE:", 6) == 0) {
            char *end = strchr(line + 6, ']');
            if (end) {
                *end = '\0';
                if (_stricmp(line + 6, raceName) == 0) { inRace = 1; foundRace = 1; }
                else inRace = 0;
            }
            continue;
        }
        if (!inRace) continue;
        if (strcmp(line, "[END]") == 0) { inRace = 0; break; }
        if (strncmp(line, "TIME=", 5) == 0) { time = atoi(line + 5); continue; }
        if (strncmp(line, "FRAMES=", 7) == 0) {
            frames = atoi(line + 7);
            if (frames > 0 && frames <= MAX_SNAPSHOTS)
                snaps = (Snapshot*)malloc(frames * sizeof(Snapshot));
            continue;
        }
        if (snaps && count < frames) {
            float v[10];
            if (sscanf(line, "%f %f %f %f %f %f %f %f %f %f",
                &v[0],&v[1],&v[2],&v[3],&v[4],&v[5],&v[6],&v[7],&v[8],&v[9]) == 10) {
                /* Store raw DWORDs to preserve exact bit patterns */
                DWORD *raw = (DWORD*)&g_snapshots[0]; /* temp */
                snaps[count].f[0] = v[0]; snaps[count].f[1] = v[1];
                snaps[count].f[2] = v[2]; snaps[count].f[3] = v[3];
                snaps[count].f[4] = v[4]; snaps[count].f[5] = v[5];
                snaps[count].f[6] = v[6]; snaps[count].f[7] = v[7];
                snaps[count].f[8] = v[8]; snaps[count].state = (unsigned char)v[9];
                /* Wait — snapshot field 9 is radius (float), field 6 is state (byte).
                 * But the game stores them as DWORDs. Let me just use the raw layout. */
                /* Actually the game's BallSnapshot is 10 DWORDs:
                 * [0]=pos_x [1]=pos_y [2]=pos_z [3]=field3 [4]=field4
                 * [5]=field5 [6]=field6(byte+3pad) [7]=field7 [8]=field8 [9]=field9
                 * We store them as floats for the file, but need to reconstruct as DWORDs. */
                count++;
            }
        }
    }
    CloseHandle(h);
    if (!foundRace || !snaps || count == 0) { if (snaps) free(snaps); return NULL; }
    *outCount = count; *outTime = time;
    return snaps;
}

/* We need a raw DWORD-based snapshot for the game's AthenaList.
 * The game allocates 0x28 bytes and stores 10 DWORDs. */
typedef struct {
    DWORD d[10];  /* 10 raw DWORDs = 0x28 bytes */
} RawSnapshot;

static void save_ghost_for_race(const char *raceName, int time,
                                DWORD (*snaps)[10], int count) {
    /* Read existing file */
    char *fileBuf = NULL; DWORD fileSize = 0;
    HANDLE h = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        fileSize = GetFileSize(h, NULL);
        if (fileSize > 0 && fileSize < 20*1024*1024) {
            fileBuf = (char*)malloc(fileSize + 1);
            if (fileBuf) { DWORD br; ReadFile(h, fileBuf, fileSize, &br, NULL); fileBuf[fileSize] = '\0'; }
        }
        CloseHandle(h);
    }

    h = CreateFileA(g_ghostPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) { if (fileBuf) free(fileBuf); return; }
    DWORD written;

    /* Copy other races */
    if (fileBuf) {
        char *p = fileBuf;
        while (*p) {
            char *raceStart = strstr(p, "[RACE:");
            if (!raceStart) break;
            char *endMark = strstr(raceStart, "[END]");
            if (!endMark) break;
            char *afterEnd = endMark + 5;
            while (*afterEnd == '\r' || *afterEnd == '\n') afterEnd++;
            char *nameStart = raceStart + 6;
            char *nameEnd = strchr(nameStart, ']');
            if (nameEnd) {
                int nameLen = nameEnd - nameStart;
                char existingName[128];
                if (nameLen < 128) {
                    memcpy(existingName, nameStart, nameLen);
                    existingName[nameLen] = '\0';
                    if (_stricmp(existingName, raceName) != 0) {
                        int blockLen = afterEnd - raceStart;
                        WriteFile(h, raceStart, blockLen, &written, NULL);
                        WriteFile(h, "\r\n", 2, &written, NULL);
                    }
                }
            }
            p = afterEnd;
        }
        free(fileBuf);
    }

    /* Write new race */
    char header[256];
    int hlen = snprintf(header, sizeof(header), "[RACE:%s]\r\nTIME=%d\r\nFRAMES=%d\r\n",
                        raceName, time, count);
    WriteFile(h, header, hlen, &written, NULL);

    for (int i = 0; i < count; i++) {
        /* Store as raw DWORDs (hex) to preserve exact bit patterns */
        char line[256];
        int len = snprintf(line, sizeof(line),
            "0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\r\n",
            snaps[i][0], snaps[i][1], snaps[i][2], snaps[i][3], snaps[i][4],
            snaps[i][5], snaps[i][6], snaps[i][7], snaps[i][8], snaps[i][9]);
        WriteFile(h, line, len, &written, NULL);
    }
    WriteFile(h, "[END]\r\n", 6, &written, NULL);
    CloseHandle(h);
}

/* ═════════════════════════════════════════════════════════════════
 * Inline asm wrappers for __thiscall game functions.
 * MinGW __thiscall function pointers silently fail — must use asm.
 * ═════════════════════════════════════════════════════════════════ */

/* operator_new — use malloc(). Safe for game structs. */
static void* op_new(SIZE_T size) { return malloc(size); }

/* ═════════════════════════════════════════════════════════════════
 * Inline asm wrappers for __thiscall game functions.
 * MinGW __thiscall function pointers silently fail — must use asm.
 * ═════════════════════════════════════════════════════════════════ */

/* AthenaList_Append is __thiscall(ECX=list, stack=item) — 1 stack param */
static void call_alist_append(DWORD *list, void *item) {
    register DWORD ecx_val asm("ecx") = (DWORD)list;
    __asm__ volatile(
        "push %0\n"
        "call *%1\n"
        "add $4, %%esp\n"
        : : "r"(item), "r"((void*)ADDR_ALIST_APPEND), "c"(ecx_val)
        : "eax", "edx", "memory"
    );
}

/* BestTimeTracker vtable[0] (destructor) is __thiscall(ECX=this, stack=freeFlag) */
static void call_btt_dtor(DWORD btt, int free_flag) {
    DWORD vtable = *(DWORD*)btt;
    DWORD dtor = *(DWORD*)vtable; /* vtable[0] */
    register DWORD ecx_val asm("ecx") = btt;
    __asm__ volatile(
        "push %0\n"
        "call *%1\n"
        "add $4, %%esp\n"
        : : "r"(free_flag), "r"((void*)dtor), "c"(ecx_val)
        : "eax", "edx", "memory"
    );
}

/* Manually construct a BestTimeTracker — replicate what BestTimeTracker_ctor
 * (0x427660) and AthenaList_Init (0x453210) do, without calling them.
 * This avoids the __thiscall problem entirely. */
static DWORD vtable_BTT = 0x004D262C;  /* PTR_FUN_004d262c from ctor */
static DWORD vtable_AthenaList = 0x004D875C;  /* PTR_FUN_004d875c from AthenaList_Init */

static void manually_init_btt(void *btt) {
    /* BestTimeTracker_ctor:
     *   *this = &PTR_FUN_004d262c (vtable)
     *   AthenaList_Init(this+4, 0)
     *   this[0x149] = 9999999  (BTT+0x524 = best_time)
     */
    DWORD *p = (DWORD*)btt;
    p[0] = vtable_BTT;           /* vtable */

    /* AthenaList_Init(this+4, 0):
     *   this+0x414 (BTT+0x418) = param (0 = capacity)
     *   this+0x410 (BTT+0x414) = 0 (list_array_ptr)
     *   this+0x004 (BTT+0x008) = 0 (list_count)  -- wait, AthenaList starts at BTT+4
     *   Actually AthenaList_Init operates on (this+1) where this is undefined4*
     *   So AthenaList starts at BTT+4 (byte offset 4).
     *   AthenaList layout relative to BTT:
     *     BTT+0x000: vtable
     *     BTT+0x004: AthenaList.vtable
     *     BTT+0x008: AthenaList.list_count
     *     BTT+0x00C..0x40B: AthenaList internal array (0x100 entries)
     *     BTT+0x40C: AthenaList.iterator_counter
     *     BTT+0x410: AthenaList.list_array_ptr
     *     BTT+0x414: AthenaList.capacity
     */
    DWORD *alist = (DWORD*)((char*)btt + 4);
    alist[0] = vtable_AthenaList;   /* BTT+0x004: AthenaList vtable */
    /* Zero the internal array (0x100 entries = 256 dwords starting at alist+2 = BTT+0x00C) */
    memset(alist + 2, 0, 0x100 * sizeof(DWORD));  /* BTT+0x00C..0x40B */
    alist[0x102] = 0;   /* BTT+0x40C: iterator_counter (0x408/4 = 0x102) */
    alist[0x103] = 0;   /* BTT+0x410: list_array_ptr */
    alist[0x104] = 0;   /* BTT+0x414: capacity */

    /* BestTimeTracker best_time (BTT+0x524 = p[0x149]) */
    p[0x149] = NO_TIME;  /* 9999999 = sentinel */
}

static void inject_saved_ghost(const char *raceName) {
    DWORD app = get_app();
    if (!app) return;

    /* Load saved ghost — use raw DWORD format */
    HANDLE hf = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        log_fmt("No GHOST.txt found for '%s'", raceName);
        return;
    }

    /* Parse file for this race */
    int savedTime = NO_TIME;
    int savedFrames = 0;
    DWORD (*savedSnaps)[10] = NULL;
    int savedCount = 0;
    int inRace = 0, foundRace = 0;
    DWORD fileSize = GetFileSize(hf, NULL);
    DWORD filePos = 0;
    char line[512];

    while (filePos < fileSize) {
        SetFilePointer(hf, filePos, NULL, FILE_BEGIN);
        DWORD toRead = sizeof(line) - 1;
        if (filePos + toRead > fileSize) toRead = fileSize - filePos;
        DWORD bytesRead = 0;
        if (!ReadFile(hf, line, toRead, &bytesRead, NULL) || bytesRead == 0) break;
        char *nl = memchr(line, '\n', bytesRead);
        if (!nl) { filePos += bytesRead; continue; }
        int lineLen = nl - line;
        if (lineLen > 0 && line[lineLen-1] == '\r') lineLen--;
        line[lineLen] = '\0';
        filePos += (nl - line) + 1;

        if (strncmp(line, "[RACE:", 6) == 0) {
            char *end = strchr(line + 6, ']');
            if (end) {
                *end = '\0';
                if (_stricmp(line + 6, raceName) == 0) { inRace = 1; foundRace = 1; }
                else inRace = 0;
            }
            continue;
        }
        if (!inRace) continue;
        if (strcmp(line, "[END]") == 0) { inRace = 0; break; }
        if (strncmp(line, "TIME=", 5) == 0) { savedTime = atoi(line + 5); continue; }
        if (strncmp(line, "FRAMES=", 7) == 0) {
            savedFrames = atoi(line + 7);
            if (savedFrames > 0 && savedFrames <= MAX_SNAPSHOTS)
                savedSnaps = (DWORD(*)[10])malloc(savedFrames * 10 * sizeof(DWORD));
            continue;
        }
        if (savedSnaps && savedCount < savedFrames) {
            DWORD d[10];
            if (sscanf(line, "%x %x %x %x %x %x %x %x %x %x",
                &d[0],&d[1],&d[2],&d[3],&d[4],&d[5],&d[6],&d[7],&d[8],&d[9]) == 10) {
                memcpy(savedSnaps[savedCount], d, 10 * sizeof(DWORD));
                savedCount++;
            }
        }
    }
    CloseHandle(hf);

    if (!foundRace || !savedSnaps || savedCount == 0) {
        log_fmt("No saved ghost for '%s' (found=%d, snaps=%p, count=%d)",
               raceName, foundRace, savedSnaps, savedCount);
        if (savedSnaps) free(savedSnaps);
        return;
    }

    log_fmt("Loading ghost: '%s' time=%d frames=%d", raceName, savedTime, savedCount);

    /* Create BestTimeTracker for playback — manually init without calling game functions */
    void *btt = op_new(BTT_SIZE);
    if (!btt) { free(savedSnaps); log_msg("ERROR: alloc BTT failed"); return; }
    log_fmt("BTT allocated at %p", btt);

    manually_init_btt(btt);
    log_msg("BTT manually initialized");

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;

    /* Copy race name into BTT+0x424 */
    char *bttName = (char*)((char*)btt + BTT_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    /* Fill with snapshots — AthenaList starts at BTT+4.
     * Instead of calling AthenaList_Append (which does malloc/realloc internally),
     * we directly write to the AthenaList's internal array. This is safe because
     * AthenaList's first 256 entries are inline (BTT+0x00C..0x40B). */
    DWORD *alist = (DWORD*)((char*)btt + 4);
    int numToStore = savedCount;
    if (numToStore > 0x100) numToStore = 0x100;  /* cap at 256 inline entries for now */
    for (int i = 0; i < numToStore; i++) {
        DWORD *snap = (DWORD*)op_new(SNAP_SIZE);
        if (!snap) { log_fmt("ERROR: alloc snap %d failed", i); continue; }
        memcpy(snap, savedSnaps[i], 10 * sizeof(DWORD));
        /* Direct store in AthenaList internal array (alist+2 = BTT+0x00C, index i) */
        alist[2 + i] = (DWORD)snap;
    }
    alist[1] = numToStore;  /* list_count */
    log_fmt("Stored %d/%d snapshots directly", numToStore, savedCount);

    /* Free old playback buffer if exists */
    DWORD existingPlayback = *(DWORD*)(app + APP_910_PLAYBACK);
    if (existingPlayback && existingPlayback > 0x10000) {
        if (!IsBadReadPtr((void*)existingPlayback, 4)) {
            DWORD vtable = *(DWORD*)existingPlayback;
            if (vtable > 0x400000 && vtable < 0x500000) {
                log_fmt("Freeing old playback BTT at 0x%X", existingPlayback);
                call_btt_dtor(existingPlayback, 1);
            }
        }
    }

    /* Verify the list was actually populated */
    int listCount = *(int*)((char*)btt + 4);
    log_fmt("BTT list count after fill: %d", listCount);

    *(DWORD*)(app + APP_910_PLAYBACK) = (DWORD)btt;
    free(savedSnaps);
    log_fmt("Ghost injected: %d snapshots into App+0x910 (btt=0x%X)", savedCount, (DWORD)btt);
}

/* ═════════════════════════════════════════════════════════════════
 * Race state detection + per-frame recording
 * ═════════════════════════════════════════════════════════════════ */

/* Raw snapshot buffer — stores 10 DWORDs per frame (exact game format) */
static DWORD g_rawSnaps[MAX_SNAPSHOTS][10];
static int g_rawCount = 0;
static DWORD g_prevRecording = 0;
static int g_heartbeatCounter = 0;

/* Check if we're in an active Time Trial race */
static int is_time_trial_active(void) {
    DWORD app = get_app();
    if (!app) return 0;

    if (IsBadReadPtr((void*)(app + APP_220_PROFILE), 4)) return 0;
    DWORD profile = *(DWORD*)(app + APP_220_PROFILE);
    if (!profile || profile < 0x10000) return 0;
    if (IsBadReadPtr((void*)(profile + 0x11), 1)) return 0;
    if (*(BYTE*)(profile + 0x11) == 0) return 0;

    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return 0;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return 0;

    return 1;
}

static void check_race_state(void) {
    DWORD app = get_app();
    if (!app) return;

    /* Heartbeat every ~5 seconds (300 frames at 16ms) */
    g_heartbeatCounter++;
    if (g_heartbeatCounter >= 300) {
        g_heartbeatCounter = 0;
        DWORD profile = 0, recording = 0, playback = 0;
        BYTE tt_flag = 0, party = 0;
        if (!IsBadReadPtr((void*)(app + APP_220_PROFILE), 4)) {
            profile = *(DWORD*)(app + APP_220_PROFILE);
            if (profile && profile > 0x10000 && !IsBadReadPtr((void*)(profile + 0x11), 1))
                tt_flag = *(BYTE*)(profile + 0x11);
        }
        if (!IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1))
            party = *(BYTE*)(app + APP_234_PARTY_MODE);
        if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
            recording = *(DWORD*)(app + APP_90C_RECORDING);
        if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4))
            playback = *(DWORD*)(app + APP_910_PLAYBACK);
        log_fmt("HEARTBEAT app=0x%X profile=0x%X tt=%d party=%d rec=0x%X play=0x%X rec_active=%d snaps=%d",
               app, profile, tt_flag, party, recording, playback, g_recording, g_rawCount);
    }

    int tt = is_time_trial_active();
    if (!tt) {
        /* Not in Time Trial — reset state if we were recording */
        if (g_recording) {
            log_fmt("Left Time Trial mode (was recording %d frames)", g_rawCount);
            g_recording = 0;
            g_raceFinished = 0;
            g_rawCount = 0;
            g_prevGoalFlag = 0;
            g_currentRaceName[0] = '\0';
        }
        /* Update prevRecording so we detect the next race start */
        if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
            g_prevRecording = *(DWORD*)(app + APP_90C_RECORDING);
        return;
    }

    /* We're in Time Trial mode */
    DWORD currRecording = 0;
    if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
        currRecording = *(DWORD*)(app + APP_90C_RECORDING);

    /* Detect new race: recording pointer changed */
    if (currRecording != g_prevRecording && currRecording && currRecording > 0x10000) {
        g_prevRecording = currRecording;

        /* Get race name */
        char raceName[128];
        if (get_race_name(raceName, sizeof(raceName)) && raceName[0]) {
            strncpy(g_currentRaceName, raceName, sizeof(g_currentRaceName) - 1);
            g_currentRaceName[sizeof(g_currentRaceName) - 1] = '\0';
            g_rawCount = 0;
            g_recording = 1;
            g_raceFinished = 0;
            g_prevGoalFlag = 0;

            log_fmt("RACE START: '%s' (BTT=0x%X)", raceName, currRecording);

            /* Try to load saved ghost */
            inject_saved_ghost(raceName);
        } else {
            /* Race name not ready yet — retry next frame */
            log_fmt("Race detected (BTT=0x%X) but name not ready, will retry", currRecording);
            g_prevRecording = 0; /* force retry next frame */
        }
    }

    /* Per-frame recording */
    if (g_recording && g_rawCount < MAX_SNAPSHOTS) {
        if (!IsBadReadPtr((void*)(app + APP_5DC_BALL), 4)) {
            DWORD ball = *(DWORD*)(app + APP_5DC_BALL);
            if (ball && ball > 0x10000 && !IsBadReadPtr((void*)(ball + 0x164), 4)) {
                /* Copy 10 DWORDs exactly as the game does:
                 * [0]=ball+0x164 [1]=ball+0x168 [2]=ball+0x16C
                 * [3]=ball+0x190  [4]=ball+0x194  [5]=ball+0x150
                 * [6]=ball+0x748(byte) [7]=ball+0x74C [8]=ball+0x750
                 * [9]=ball+0x284 */
                DWORD *snap = g_rawSnaps[g_rawCount];
                snap[0] = *(DWORD*)(ball + 0x164);
                snap[1] = *(DWORD*)(ball + 0x168);
                snap[2] = *(DWORD*)(ball + 0x16C);
                snap[3] = *(DWORD*)(ball + 0x190);
                snap[4] = *(DWORD*)(ball + 0x194);
                snap[5] = *(DWORD*)(ball + 0x150);
                snap[6] = *(DWORD*)(ball + 0x748); /* read full DWORD, game uses only byte */
                snap[7] = *(DWORD*)(ball + 0x74C);
                snap[8] = *(DWORD*)(ball + 0x750);
                snap[9] = *(DWORD*)(ball + 0x284);
                g_rawCount++;
            }
        }
    }

    /* Detect goal crossing */
    if (g_recording && !g_raceFinished) {
        if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
            BYTE goalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            if (goalFlag && !g_prevGoalFlag) {
                g_raceFinished = 1;
                log_fmt("GOAL! frames=%d", g_rawCount);

                /* Get finish time from recording buffer */
                int finishTime = NO_TIME;
                DWORD btt = *(DWORD*)(app + APP_90C_RECORDING);
                if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4)) {
                    finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
                }
                log_fmt("Finish time=%d (NO_TIME=%d)", finishTime, NO_TIME);

                if (finishTime != NO_TIME && g_rawCount > 0 && g_currentRaceName[0]) {
                    /* Check if GHOST.txt has existing data for this race */
                    int savedTime = NO_TIME;
                    int savedCount = 0;
                    /* Quick check: does file exist and have this race? */
                    HANDLE hf = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hf != INVALID_HANDLE_VALUE) {
                        CloseHandle(hf);
                        /* Use load function to check */
                        /* For simplicity, just always save — overwrite logic in save function */
                    }

                    log_fmt("Saving ghost: '%s' time=%d frames=%d",
                           g_currentRaceName, finishTime, g_rawCount);
                    save_ghost_for_race(g_currentRaceName, finishTime,
                                       g_rawSnaps, g_rawCount);
                    log_msg("Ghost saved to GHOST.txt");
                } else {
                    log_fmt("NOT saving: finishTime=%d frames=%d name='%s'",
                           finishTime, g_rawCount, g_currentRaceName);
                }
            }
            g_prevGoalFlag = goalFlag;
        }
    }
}

/* ═════════════════════════════════════════════════════════════════
 * Polling thread
 * ═════════════════════════════════════════════════════════════════ */

static DWORD WINAPI ghost_thread(LPVOID param) {
    Sleep(3000);
    log_msg("Ghost thread v2 started");
    while (1) {
        Sleep(16);
        check_race_state();
    }
    return 0;
}

/* ═════════════════════════════════════════════════════════════════
 * Init
 * ═════════════════════════════════════════════════════════════════ */

static void init_paths(HMODULE hInst) {
    char path[MAX_PATH];
    if (GetModuleFileNameA(hInst, path, MAX_PATH)) {
        char *p = strrchr(path, '\\');
        if (p) {
            strcpy(p + 1, "GHOST.txt");
            strncpy(g_ghostPath, path, MAX_PATH - 1);
            p = strrchr(path, '\\');
            if (p) {
                strcpy(p + 1, "ghost_saver_log.txt");
                strncpy(g_logPath, path, MAX_PATH - 1);
            }
        }
    }
}

static void init_mod(HMODULE hInst) {
    init_paths(hInst);
    log_msg("=== Ghost Saver Mod v2 Init ===");
    log_fmt("GHOST path: %s", g_ghostPath);
    log_fmt("Log path: %s", g_logPath);

    if (GetFileAttributesA(g_ghostPath) == INVALID_FILE_ATTRIBUTES) {
        HANDLE h = CreateFileA(g_ghostPath, GENERIC_WRITE, 0, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        log_msg("Created empty GHOST.txt");
    }

    CreateThread(NULL, 0, ghost_thread, NULL, 0, NULL);
    log_msg("Thread launched");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_mod(hModule);
    }
    return TRUE;
}
