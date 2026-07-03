/*
 * ghost_saver.c — Persistent Ghost Data for Time Trial Mode
 *
 * Saves per-race ghost recordings to GHOST.txt so they persist across
 * game restarts. The vanilla game stores ghosts only in memory — they
 * vanish when you quit. This mod:
 *
 *   1. On race start: loads saved ghost data from GHOST.txt into App+0x910
 *      (playback buffer) so the ghost ball appears immediately.
 *   2. During race: records ball snapshots every frame (same as vanilla).
 *   3. On race finish: compares finish time against saved best time.
 *      If faster: overwrites the saved data. If slower: discards.
 *
 * GHOST.txt format:
 *   [RACE:Warm-Up Race]
 *   TIME=12345
 *   FRAMES=1500
 *   <10 float values per line, space-separated>
 *   ...
 *   [END]
 *
 * Build:
 *   i686-w64-mingw32-gcc -shared -o bass.dll ghost_saver.c -I../shared \
 *     -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
 *     -Wl,--add-stdcall-alias -msse2 -mfpmath=sse
 */

#include "bass_proxy.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * Game constants
 * ═══════════════════════════════════════════════════════════════════════════ */

#define APP_PTR             0x005341E0
#define ADDR_SCENE_UPDATE   0x0041B540  /* Scene_UpdateBallsAndState */
#define ADDR_START_RACE     0x00428C50  /* App_StartPracticeRace */
#define ADDR_RECORD_SNAP    0x00427810  /* BestTimeTracker_RecordSnapshot */
#define ADDR_PLAYBACK_SNAP  0x00427690  /* BestTimeTracker_PlaybackSnapshot */
#define ADDR_BTT_CTOR       0x00427660  /* BestTimeTracker_ctor */
#define ADDR_BTT_DTOR       0x00427760  /* BestTimeTracker_dtor */
#define ADDR_ALIST_APPEND   0x00453780  /* AthenaList_Append */
#define ADDR_ALIST_GETSIZE  0x004536A0  /* AthenaList_GetSize */
#define ADDR_OPERATOR_NEW   0x004BA570  /* operator_new */
#define ADDR_FREE           0x004BA580  /* free (operator delete) */

/* App offsets */
#define APP_90C_RECORDING  0x90C
#define APP_910_PLAYBACK    0x910
#define APP_5DC_BALL        0x5DC
#define APP_5E8_RACE_TIMER  0x5E8
#define APP_5D6_GOAL_FLAG   0x5D6   /* BYTE: set to 1 when player crosses goal */
#define APP_234_PARTY_MODE  0x234
#define APP_220_PROFILE     0x220

/* BestTimeTracker offsets */
#define BTT_SIZE            0x528
#define BTT_BEST_TIME       0x524   /* dword at +0x149 (0x149*4=0x524) */
#define BTT_LIST_ARRAY      0x410
#define BTT_PLAYBACK_INDEX  0x41C
#define BTT_RACE_TIME       0x420
#define BTT_NAME            0x424   /* char[...] — race name string */
#define BTT_LIST_COUNT       0x004  /* relative to BTT start */

/* BallSnapshot is 0x28 bytes (10 fields, stored as DWORD array) */
#define SNAP_SIZE           0x28
#define SNAP_FIELDS         10

/* AthenaList layout (relative to list start = BTT+4) */
#define ALIST_COUNT         0x000
#define ALIST_ITEMS         0x008   /* first item pointer */
#define ALIST_ARRAY_PTR     0x410   /* dynamic array when count > 256 */

/* Sentinel for "no time recorded" */
#define NO_TIME             9999999

/* Max snapshots we store per race in our own buffer */
#define MAX_SNAPSHOTS       5000

/* ═══════════════════════════════════════════════════════════════════════════
 * Mod state
 * ═══════════════════════════════════════════════════════════════════════════ */

static char g_ghostPath[MAX_PATH] = "";
static char g_logPath[MAX_PATH] = "";

/* Our own snapshot buffer — mirrors what the game records */
typedef struct {
    float pos_x, pos_y, pos_z;    /* 0,1,2 */
    float vel_x, vel_y;          /* 3,4 */
    float rotation;               /* 5 */
    unsigned char state_flag;     /* 6 (byte, padded to 4) */
    float rot_x, rot_y;          /* 7,8 */
    float radius;                 /* 9 */
} Snapshot;

static Snapshot g_snapshots[MAX_SNAPSHOTS];
static int g_snapshotCount = 0;
static char g_currentRaceName[128] = "";
static int g_recording = 0;       /* 1 = currently in a Time Trial race */
static int g_raceFinished = 0;   /* 1 = goal crossed, waiting for race end */
static int g_prevGoalFlag = 0;

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

/* ═══════════════════════════════════════════════════════════════════════════
 * Get current race name from the recording buffer
 * ═══════════════════════════════════════════════════════════════════════════ */

static void get_race_name(char *out, int outLen) {
    out[0] = '\0';
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;
    if (IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) return;
    DWORD btt = *(DWORD*)(app + APP_90C_RECORDING);
    if (!btt || btt < 0x10000) return;
    if (IsBadReadPtr((void*)(btt + BTT_NAME), 1)) return;
    /* Race name is stored at BTT+0x424 as a null-terminated string */
    char *name = (char*)(btt + BTT_NAME);
    if (IsBadReadPtr(name, 1)) return;
    /* Validate it looks like a real string (alphanumeric first char) */
    if (name[0] < 0x20 || name[0] > 0x7E) return;
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GHOST.txt file format
 *
 * [RACE:Warm-Up Race]
 * TIME=12345
 * FRAMES=1500
 * <pos_x> <pos_y> <pos_z> <vel_x> <vel_y> <rotation> <state> <rot_x> <rot_y> <radius>
 * ...
 * [END]
 * ═══════════════════════════════════════════════════════════════════════════ */

static int find_race_in_file(const char *raceName, int *outTime, int *outFrames) {
    HANDLE h = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;

    char line[512];
    DWORD bytesRead;
    char buf[4096];
    DWORD bufPos = 0;
    BOOL eof = FALSE;

    /* Read file in chunks and search line by line */
    while (!eof) {
        /* Fill buffer */
        if (!ReadFile(h, buf + bufPos, sizeof(buf) - bufPos - 1, &bytesRead, NULL) || bytesRead == 0) {
            eof = TRUE;
            bytesRead = 0;
        }
        bufPos += bytesRead;
        buf[bufPos] = '\0';

        /* Process lines */
        char *p = buf;
        while (p < buf + bufPos) {
            char *nl = strchr(p, '\n');
            if (!nl) {
                /* Move remaining to start */
                memmove(buf, p, buf + bufPos - p);
                bufPos = buf + bufPos - p;
                break;
            }
            int len = nl - p;
            if (len > 0 && p[len-1] == '\r') len--;
            memcpy(line, p, len);
            line[len] = '\0';
            p = nl + 1;

            /* Check for [RACE:name] */
            if (strncmp(line, "[RACE:", 6) == 0) {
                char *end = strchr(line + 6, ']');
                if (end) {
                    *end = '\0';
                    if (_stricmp(line + 6, raceName) == 0) {
                        /* Found our race — read TIME and FRAMES */
                        *outTime = NO_TIME;
                        *outFrames = 0;
                        /* Read next two lines for TIME= and FRAMES= */
                        /* We need to continue reading from file */
                        CloseHandle(h);
                        return 1; /* Found — caller will parse the data */
                    }
                }
            }
        }
        if (eof) break;
    }

    CloseHandle(h);
    return 0;
}

/* Load ghost data from GHOST.txt for a specific race name.
 * Returns snapshots in a malloc'd buffer (caller must free).
 * Sets *outCount and *outTime. Returns NULL if not found. */
static Snapshot* load_ghost_for_race(const char *raceName, int *outCount, int *outTime) {
    HANDLE h = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;

    Snapshot *snaps = NULL;
    int count = 0;
    int time = NO_TIME;
    int frames = 0;
    int inRace = 0;
    int foundRace = 0;

    /* Simple line-by-line reader */
    char line[512];
    DWORD filePos = 0;
    DWORD fileSize = GetFileSize(h, NULL);

    while (filePos < fileSize) {
        /* Read one line */
        DWORD toRead = sizeof(line) - 1;
        if (filePos + toRead > fileSize) toRead = fileSize - filePos;

        OVERLAPPED ov = {0};
        ov.Offset = filePos;
        DWORD bytesRead = 0;
        /* Use SetFilePointer + ReadFile instead */
        SetFilePointer(h, filePos, NULL, FILE_BEGIN);
        if (!ReadFile(h, line, toRead, &bytesRead, NULL) || bytesRead == 0) break;

        /* Find newline */
        char *nl = memchr(line, '\n', bytesRead);
        if (!nl) {
            filePos += bytesRead;
            continue;
        }

        int lineLen = nl - line;
        if (lineLen > 0 && line[lineLen-1] == '\r') lineLen--;
        line[lineLen] = '\0';
        filePos += (nl - line) + 1;

        /* Parse [RACE:name] */
        if (strncmp(line, "[RACE:", 6) == 0) {
            char *end = strchr(line + 6, ']');
            if (end) {
                *end = '\0';
                if (_stricmp(line + 6, raceName) == 0) {
                    inRace = 1;
                    foundRace = 1;
                } else {
                    inRace = 0;
                }
            }
            continue;
        }

        if (!inRace) continue;

        /* [END] marker */
        if (strcmp(line, "[END]") == 0) {
            inRace = 0;
            break;
        }

        /* TIME= line */
        if (strncmp(line, "TIME=", 5) == 0) {
            time = atoi(line + 5);
            continue;
        }

        /* FRAMES= line */
        if (strncmp(line, "FRAMES=", 7) == 0) {
            frames = atoi(line + 7);
            if (frames > 0 && frames <= MAX_SNAPSHOTS) {
                snaps = (Snapshot*)malloc(frames * sizeof(Snapshot));
                if (!snaps) { CloseHandle(h); return NULL; }
            }
            continue;
        }

        /* Data line: 10 space-separated floats */
        if (snaps && count < frames) {
            float v[10];
            int parsed = sscanf(line, "%f %f %f %f %f %f %f %f %f %f",
                &v[0], &v[1], &v[2], &v[3], &v[4],
                &v[5], &v[6], &v[7], &v[8], &v[9]);
            if (parsed == 10) {
                snaps[count].pos_x = v[0];
                snaps[count].pos_y = v[1];
                snaps[count].pos_z = v[2];
                snaps[count].vel_x = v[3];
                snaps[count].vel_y = v[4];
                snaps[count].rotation = v[5];
                snaps[count].state_flag = (unsigned char)v[6];
                snaps[count].rot_x = v[7];
                snaps[count].rot_y = v[8];
                snaps[count].radius = v[9];
                count++;
            }
        }
    }

    CloseHandle(h);

    if (!foundRace || !snaps || count == 0) {
        if (snaps) free(snaps);
        return NULL;
    }

    *outCount = count;
    *outTime = time;
    return snaps;
}

/* Save ghost data for a race to GHOST.txt.
 * If the race already exists, overwrite it. Otherwise append. */
static void save_ghost_for_race(const char *raceName, int time,
                                Snapshot *snaps, int count) {
    /* Read entire existing file into memory */
    char *fileBuf = NULL;
    DWORD fileSize = 0;

    HANDLE h = CreateFileA(g_ghostPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        fileSize = GetFileSize(h, NULL);
        if (fileSize > 0 && fileSize < 20*1024*1024) { /* 20MB max */
            fileBuf = (char*)malloc(fileSize + 1);
            if (fileBuf) {
                DWORD bytesRead;
                ReadFile(h, fileBuf, fileSize, &bytesRead, NULL);
                fileBuf[fileSize] = '\0';
            }
        }
        CloseHandle(h);
    }

    /* Open file for writing (truncate) */
    h = CreateFileA(g_ghostPath, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        if (fileBuf) free(fileBuf);
        log_msg("ERROR: Cannot create GHOST.txt for writing");
        return;
    }

    DWORD written;

    /* If we have existing data, copy all races except the one we're overwriting */
    if (fileBuf) {
        char *p = fileBuf;
        while (*p) {
            /* Find next [RACE: or [END] */
            char *raceStart = strstr(p, "[RACE:");
            if (!raceStart) {
                /* No more races — write remaining (shouldn't happen) */
                break;
            }

            /* Find [END] for this race */
            char *endMark = strstr(raceStart, "[END]");
            if (!endMark) break;
            char *afterEnd = endMark + 5;
            while (*afterEnd == '\r' || *afterEnd == '\n') afterEnd++;

            /* Extract race name */
            char *nameStart = raceStart + 6;
            char *nameEnd = strchr(nameStart, ']');
            if (nameEnd) {
                int nameLen = nameEnd - nameStart;
                char existingName[128];
                if (nameLen < 128) {
                    memcpy(existingName, nameStart, nameLen);
                    existingName[nameLen] = '\0';
                    if (_stricmp(existingName, raceName) != 0) {
                        /* Different race — copy it as-is */
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

    /* Write our new race data */
    char header[256];
    int headerLen = snprintf(header, sizeof(header),
        "[RACE:%s]\r\nTIME=%d\r\nFRAMES=%d\r\n",
        raceName, time, count);
    WriteFile(h, header, headerLen, &written, NULL);

    /* Write each snapshot as a line */
    char line[256];
    for (int i = 0; i < count; i++) {
        int len = snprintf(line, sizeof(line),
            "%.3f %.3f %.3f %.3f %.3f %.3f %d %.3f %.3f %.3f\r\n",
            snaps[i].pos_x, snaps[i].pos_y, snaps[i].pos_z,
            snaps[i].vel_x, snaps[i].vel_y, snaps[i].rotation,
            snaps[i].state_flag,
            snaps[i].rot_x, snaps[i].rot_y, snaps[i].radius);
        WriteFile(h, line, len, &written, NULL);
    }

    WriteFile(h, "[END]\r\n", 6, &written, NULL);
    CloseHandle(h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Inject saved ghost into the game's playback buffer (App+0x910)
 *
 * The game creates a BestTimeTracker at App+0x910 only if the previous
 * recording was promoted. If there's no playback buffer, the ghost ball
 * won't appear. We create one ourselves and fill it with saved data.
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef void* (__cdecl *operator_new_fn)(SIZE_T);
typedef void (__thiscall *btt_ctor_fn)(void *btt);
typedef void (__thiscall *alist_append_fn)(DWORD *list, void *item);
typedef int  (__thiscall *alist_getsize_fn)(DWORD *list);

static void inject_saved_ghost(const char *raceName) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;

    /* Load saved ghost data */
    int savedCount = 0;
    int savedTime = NO_TIME;
    Snapshot *snaps = load_ghost_for_race(raceName, &savedCount, &savedTime);
    if (!snaps || savedCount == 0) {
        log_fmt("No saved ghost found for '%s'", raceName);
        if (snaps) free(snaps);
        return;
    }

    log_fmt("Loading ghost for '%s': %d frames, time=%d", raceName, savedCount, savedTime);

    /* Get function pointers */
    operator_new_fn op_new = (operator_new_fn)ADDR_OPERATOR_NEW;
    btt_ctor_fn btt_ctor = (btt_ctor_fn)ADDR_BTT_CTOR;
    alist_append_fn alist_append = (alist_append_fn)ADDR_ALIST_APPEND;

    /* Create a new BestTimeTracker for playback */
    void *btt = op_new(BTT_SIZE);
    if (!btt) {
        free(snaps);
        log_msg("ERROR: Cannot allocate BestTimeTracker");
        return;
    }
    btt_ctor(btt);

    /* Set the best time */
    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;

    /* Copy race name into BTT+0x424 */
    char *bttName = (char*)((char*)btt + BTT_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    /* Create BallSnapshot objects and append to the AthenaList */
    DWORD *listPtr = (DWORD*)((char*)btt + 4); /* AthenaList starts at BTT+4 */

    for (int i = 0; i < savedCount; i++) {
        /* Allocate a 0x28-byte BallSnapshot */
        DWORD *snap = (DWORD*)op_new(SNAP_SIZE);
        if (!snap) continue;

        /* Fill it from our saved data */
        snap[0] = *(DWORD*)&snaps[i].pos_x;
        snap[1] = *(DWORD*)&snaps[i].pos_y;
        snap[2] = *(DWORD*)&snaps[i].pos_z;
        snap[3] = *(DWORD*)&snaps[i].vel_x;
        snap[4] = *(DWORD*)&snaps[i].vel_y;
        snap[5] = *(DWORD*)&snaps[i].rotation;
        *(unsigned char*)(snap + 6) = snaps[i].state_flag;
        snap[7] = *(DWORD*)&snaps[i].rot_x;
        snap[8] = *(DWORD*)&snaps[i].rot_y;
        snap[9] = *(DWORD*)&snaps[i].radius;

        /* Append to the AthenaList */
        alist_append(listPtr, snap);
    }

    /* Store in App+0x910 (playback buffer) */
    /* If there's already a playback buffer, free it first */
    DWORD existingPlayback = *(DWORD*)(app + APP_910_PLAYBACK);
    if (existingPlayback && existingPlayback > 0x10000) {
        /* Call its vtable[0] (destructor) with param 1 (free) */
        if (!IsBadReadPtr((void*)existingPlayback, 4)) {
            DWORD vtable = *(DWORD*)existingPlayback;
            if (vtable > 0x400000 && vtable < 0x500000) {
                void (__thiscall *dtor)(void*, int) =
                    *(void (__thiscall **)(void*, int))vtable;
                dtor((void*)existingPlayback, 1);
            }
        }
    }

    *(DWORD*)(app + APP_910_PLAYBACK) = (DWORD)btt;

    free(snaps);
    log_fmt("Ghost injected: %d snapshots into App+0x910", savedCount);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Per-frame hook: record snapshots + detect race finish
 *
 * We hook Scene_UpdateBallsAndState's epilogue (after all ball updates).
 * The function is __fastcall(param_1=scene). We detour at the function entry
 * to call our logic after the original returns.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Original function bytes (to restore on unload) */
static BYTE g_origBytes[8];
static int g_hookInstalled = 0;

/* Trampoline: original prologue + JMP back */
static BYTE *g_trampoline = NULL;

/* Our per-frame callback */
static void on_frame_update(DWORD scene) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;

    /* Check Time Trial mode: profile+0x11 != 0 AND app+0x234 == 0 */
    if (IsBadReadPtr((void*)(app + APP_220_PROFILE), 4)) return;
    DWORD profile = *(DWORD*)(app + APP_220_PROFILE);
    if (!profile || profile < 0x10000) return;
    if (IsBadReadPtr((void*)(profile + 0x11), 1)) return;
    if (*(BYTE*)(profile + 0x11) == 0) return;  /* not time trial */
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return;  /* party mode */

    /* Record snapshot into our buffer */
    if (g_recording && g_snapshotCount < MAX_SNAPSHOTS) {
        if (!IsBadReadPtr((void*)(app + APP_5DC_BALL), 4)) {
            DWORD ball = *(DWORD*)(app + APP_5DC_BALL);
            if (ball && ball > 0x10000 && !IsBadReadPtr((void*)(ball + 0x164), 4)) {
                Snapshot *s = &g_snapshots[g_snapshotCount];
                s->pos_x = *(float*)(ball + 0x164);
                s->pos_y = *(float*)(ball + 0x168);
                s->pos_z = *(float*)(ball + 0x16C);
                s->vel_x = *(float*)(ball + 0x190);
                s->vel_y = *(float*)(ball + 0x194);
                s->rotation = *(float*)(ball + 0x150);
                s->state_flag = *(unsigned char*)(ball + 0x748);
                s->rot_x = *(float*)(ball + 0x74C);
                s->rot_y = *(float*)(ball + 0x750);
                s->radius = *(float*)(ball + 0x284);
                g_snapshotCount++;
            }
        }
    }

    /* Detect goal crossing → race finished */
    if (g_recording && !g_raceFinished) {
        if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
            BYTE goalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            if (goalFlag && !g_prevGoalFlag) {
                g_raceFinished = 1;
                log_fmt("Goal detected! Race finished. Frames=%d", g_snapshotCount);

                /* Get finish time from recording buffer */
                int finishTime = NO_TIME;
                if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) {
                    DWORD btt = *(DWORD*)(app + APP_90C_RECORDING);
                    if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4)) {
                        finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
                    }
                }

                if (finishTime != NO_TIME && g_snapshotCount > 0) {
                    /* Compare with saved best time */
                    int savedTime = NO_TIME;
                    int savedCount = 0;
                    Snapshot *saved = load_ghost_for_race(g_currentRaceName, &savedCount, &savedTime);

                    if (saved) {
                        if (finishTime < savedTime) {
                            log_fmt("New best time! %d < %d — saving", finishTime, savedTime);
                            save_ghost_for_race(g_currentRaceName, finishTime,
                                               g_snapshots, g_snapshotCount);
                        } else {
                            log_fmt("Slower: %d >= %d — discarding", finishTime, savedTime);
                        }
                        free(saved);
                    } else {
                        /* No saved ghost — save this one */
                        log_fmt("First ghost for '%s' — saving (time=%d, frames=%d)",
                               g_currentRaceName, finishTime, g_snapshotCount);
                        save_ghost_for_race(g_currentRaceName, finishTime,
                                           g_snapshots, g_snapshotCount);
                    }
                }
            }
            g_prevGoalFlag = goalFlag;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Hook Scene_UpdateBallsAndState entry
 *
 * Original prologue (8 bytes):
 *   0041B540: 51                push ecx
 *   0041B541: 56                push esi
 *   0041B542: 8B F1             mov esi, ecx    (fastcall: param_1 in ECX)
 *   0041B544: 57                push edi
 *
 * We detour: JMP to our cave, which calls original then our callback.
 * Since this is a __fastcall function (ECX = scene), we save ECX,
 * call the original via trampoline, then call our callback with the scene.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Our cave code (written as a C function called from the detour) */
static void __fastcall scene_update_cave(DWORD scene) {
    /* We already arrived here via the trampoline, which means:
     * The trampoline executed the original prologue (4 bytes) + rest of function + RET,
     * then returned to us. But actually, for a function-entry detour, we need to:
     * 1. Call the original function (with the original ECX)
     * 2. After it returns, call our callback
     *
     * The trampoline approach: we saved the original bytes, create a trampoline
     * that has those bytes + JMP back to after the patch point.
     */

    /* This function is called by the JMP hook.
     * At this point, ECX = scene (the original parameter).
     * We need to: call original, then do our stuff.
     * But the original function already ran via the trampoline...
     *
     * Actually, simpler approach: we use a CALL-based hook.
     * The hook redirects the CALLER's call to us. We call the original
     * via trampoline, then do our post-processing.
     *
     * But there's no single caller — this function is called from
     * Scene_Update via vtable. So we hook the function entry.
     *
     * Simplest approach: hook at the RETURN of the function.
     * Scene_UpdateBallsAndState ends with a simple RET.
     */

    /* Actually, let's use a different approach: a polling thread.
     * This is much simpler and safer than function hooks for this use case.
     * We poll every 16ms (60Hz) which matches the game's frame rate.
     */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Race start detection
 *
 * Instead of hooking App_StartPracticeRace (complex __thiscall with stack params),
 * we detect race start in our polling thread by watching:
 * - App+0x90C (recording buffer) changes (new BestTimeTracker created)
 * - profile+0x11 == 1 (time trial mode)
 * - Race name from BTT+0x424
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD prevRecordingPtr = 0;

static void check_race_start(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return;

    /* Check Time Trial mode */
    if (IsBadReadPtr((void*)(app + APP_220_PROFILE), 4)) return;
    DWORD profile = *(DWORD*)(app + APP_220_PROFILE);
    if (!profile || profile < 0x10000) return;
    if (IsBadReadPtr((void*)(profile + 0x11), 1)) return;
    if (*(BYTE*)(profile + 0x11) == 0) return;
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return;

    /* Get current recording buffer pointer */
    if (IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) return;
    DWORD currRecording = *(DWORD*)(app + APP_90C_RECORDING);

    /* Detect new race: recording pointer changed */
    if (currRecording != prevRecordingPtr && currRecording && currRecording > 0x10000) {
        /* New race started! */
        prevRecordingPtr = currRecording;

        /* Get race name */
        char raceName[128];
        get_race_name(raceName, sizeof(raceName));
        if (raceName[0]) {
            strncpy(g_currentRaceName, raceName, sizeof(g_currentRaceName) - 1);
            g_currentRaceName[sizeof(g_currentRaceName) - 1] = '\0';

            /* Reset recording state */
            g_snapshotCount = 0;
            g_recording = 1;
            g_raceFinished = 0;
            g_prevGoalFlag = 0;

            log_fmt("Race started: '%s' (BTT=0x%X)", raceName, currRecording);

            /* Inject saved ghost into playback buffer */
            inject_saved_ghost(raceName);
        }
    }

    /* Detect race end (left time trial mode) */
    if (prevRecordingPtr && currRecording == 0) {
        prevRecordingPtr = 0;
        g_recording = 0;
        g_raceFinished = 0;
        g_snapshotCount = 0;
        g_currentRaceName[0] = '\0';
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Polling thread — 60Hz, no hooks needed
 * ═══════════════════════════════════════════════════════════════════════════ */

static DWORD WINAPI ghost_thread(LPVOID param) {
    /* Wait for game to fully initialize */
    Sleep(3000);

    log_msg("Ghost saver thread started");

    while (1) {
        Sleep(16); /* ~60Hz */

        /* Check for race start/end */
        check_race_start();

        /* Per-frame recording + finish detection */
        if (g_recording) {
            on_frame_update(0);
        }
    }
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Init
 * ═══════════════════════════════════════════════════════════════════════════ */

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

    log_msg("=== Ghost Saver Mod Init ===");

    /* Create empty GHOST.txt if it doesn't exist */
    if (GetFileAttributesA(g_ghostPath) == INVALID_FILE_ATTRIBUTES) {
        HANDLE h = CreateFileA(g_ghostPath, GENERIC_WRITE, 0, NULL,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
        log_msg("Created empty GHOST.txt");
    }

    /* Start polling thread */
    CreateThread(NULL, 0, ghost_thread, NULL, 0, NULL);
    log_msg("Polling thread launched");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DllMain
 * ═══════════════════════════════════════════════════════════════════════════ */

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        load_real_bass();
        init_mod(hModule);
    }
    return TRUE;
}
