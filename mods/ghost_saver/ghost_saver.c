/* ghost_saver.c — Persistent Ghost Data for Time Trial Mode (v25)
 *
 * v25: Three fixes:
 *      (1) Dummy BTT ctor leak fix — if the dummy recording BTT constructor
 *          fails (vtable mismatch), the 528-byte struct was never freed.
 *          Now calls game_free (0x4BA74D) to release it, matching the
 *          existing fix in inject_saved_ghost.
 *      (2) Frame count sanity check — inject_saved_ghost now rejects .ghost
 *          files with frameCount >= 200000 to prevent huge allocations from
 *          corrupted or crafted files (200000 frames = ~55 min, well beyond
 *          any legit race).
 *      (3) Filename sanitization — race_name_to_filename now replaces invalid
 *          Windows filename characters (\ / : * ? " < > |) with underscores
 *          so custom levels with unusual names produce valid filenames.
 *      (4) Title-case conversion — every word's first letter is uppercased
 *          and all other letters lowercased. Words are delimited by spaces
 *          and hyphens, so "RIDDLES IN THE DARK" -> "Riddles In The Dark",
 *          "WARM-UP" -> "Warm-Up".
 * v24: Three fixes:
 *      (1) Dynamic snapshot buffer — replaces fixed 5000-frame static array
 *          with malloc/realloc so long races (>83s) are no longer truncated.
 *          The old g_rawSnaps[5000][10] static BSS buffer capped ghosts at
 *          ~83 seconds; longer races had their ghosts frozen mid-track.
 *      (2) BTT ctor failure leak fix — if call_btt_ctor() produces a bad
 *          vtable, the 528-byte BTT struct (allocated via game_operator_new)
 *          was never freed. Now calls game_free (0x4BA74D, the same CRT _free
 *          that operator_new's _malloc pairs with) to release it.
 *      (3) Thread synchronization — added CRITICAL_SECTION around all shared
 *          state between the detour hook (main thread) and the background
 *          monitor thread. Prevents torn reads on g_hookRaceName, stale
 *          g_recording state, and corrupted g_rawSnaps during race transitions.
 * v23: Fix memory leak — save old App+0x910 BTT before overwriting, then
 *      destroy it after the trampoline returns. Calls the BTT deleting
 *      destructor (vtable[0]=0x4278C0, __thiscall(this,flags=1), RET 0x4)
 *      which frees all snapshots via BestTimeTracker_dtor (0x427760) then
 *      frees the BTT struct via _free (0x4BA74D).
 *      Also fix ghost-not-saved bug: set g_prevGoalFlag to current value
 *      (not 0) on race start to prevent stale goal flag false-trigger, and
 *      reset g_raceFinished when 0 snapshots detected (stale flag recovery).
 * v22: Remove dead get_player_ball override, convert ghost file format from
 *      hex text to binary (2x smaller, faster load/save), update README.
 * v21: Pre-inject App+0x910 BEFORE App_StartPracticeRace so Board_ctor
 *      creates ghost ball. Read game's own BTT recording at goal time
 *      instead of 60Hz polling (fixes 2x playback speed). Per-race .ghost
 *      files in Ghosts/ directory. PreviousRun.ghost = always latest.
 * v19: Fix crash at 0x4276B6 (BestTimeTracker_PlaybackSnapshot NULL deref).
 * v18: Fix heap allocator (use game's operator_new, not malloc), add time
 *      comparison before saving, add Time Trial check in hook path.
 * v3:  Fix __thiscall calls via inline asm — MinGW __thiscall function
 *      pointers silently fail. Use raw push/mov ecx/call.
 * v1:  Initial release.
 */

#include "bass_proxy.h"

#define APP_PTR             0x005341E0
#define ADDR_BTT_CTOR       0x00427660
#define ADDR_BTT_DTOR       0x004278C0  /* vtable[0] — deleting destructor: __thiscall(this, flags), RET 0x4 */
#define ADDR_ALIST_APPEND   0x00453780
#define ADDR_OPERATOR_NEW   0x004BA57B  /* REAL operator_new — NOT 0x4BA570 (zlib) */
#define ADDR_GAME_FREE      0x004BA74D  /* CRT _free — pairs with operator_new's _malloc */
#define BTT_SIZE            0x528
#define BTT_BEST_TIME       0x524
#define BTT_NAME            0x424
#define SNAP_SIZE           0x28
#define NO_TIME             9999999
#define MAX_SNAPSHOTS       5000  /* initial capacity, grows dynamically */
#define SNAP_DWORDS         10
#define SNAP_BYTES          40

#define APP_90C_RECORDING  0x90C
#define APP_910_PLAYBACK    0x910
#define APP_5DC_BALL        0x5DC
#define APP_5D6_GOAL_FLAG   0x5D6
#define APP_234_PARTY_MODE  0x234
#define APP_220_PROFILE     0x220

/* game_operator_new is provided by bass_proxy.h (0x4BA57B — real operator_new) */

/* --- Dynamic snapshot buffer (P1 fix) ---
 * Replaces the old static g_rawSnaps[5000][10] which capped ghosts at ~83s.
 * Now grows as needed via realloc to handle arbitrarily long races. */
static DWORD (*g_rawSnaps)[SNAP_DWORDS] = NULL;
static int g_rawCount = 0;
static int g_rawCapacity = 0;

/* --- Thread synchronization (P3 fix) ---
 * Protects all shared state between the detour hook (main thread, runs
 * hook_impl) and the background monitor thread (runs check_race_state).
 * Without this, a race transition during goal processing could cause torn
 * reads on g_hookRaceName, stale g_recording state, or corrupted g_rawSnaps. */
static CRITICAL_SECTION g_cs;

static char g_currentRaceName[128] = "";
static char g_hookRaceName[128] = "";  /* set by pre-inject hook from static table */
static int g_recording = 0;
static int g_raceFinished = 0;
static int g_prevGoalFlag = 0;
static DWORD g_prevRecording = 0;
static DWORD g_savedOldPlayback = 0;   /* old App+0x910 saved for post-trampoline destruction */
static char g_ghostDir[MAX_PATH] = "";  /* .../Ghosts/ directory */
static char g_logPath[MAX_PATH] = "";

/* game_free — CRT _free that pairs with operator_new's _malloc.
 * Used to free failed BTT allocations without calling the destructor
 * (which would crash on uninitialized vtable/AthenaList state). */
typedef void (__cdecl *game_free_t)(void*);
static game_free_t game_free = (game_free_t)ADDR_GAME_FREE;

/* Logging disabled — set to 1 to re-enable */
#define LOGGING_ENABLED 0

static void log_msg(const char *msg) {
#if LOGGING_ENABLED
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
#endif
}

static void log_fmt(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log_msg(buf);
}

static DWORD get_app(void) {
    if (IsBadReadPtr((void*)APP_PTR, 4)) return 0;
    DWORD app = *(DWORD*)APP_PTR;
    if (!app || app < 0x10000) return 0;
    return app;
}

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
    for (int i = 0; i < 64 && name[i]; i++) {
        if (name[i] < 0x20 || name[i] > 0x7E) { name[i] = '\0'; break; }
    }
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

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

/* Convert race name to ghost filename.
 * "Warm-Up Race" -> "Warm-Up.ghost"
 * "BEGINNER RACE" -> "Beginner.ghost"
 * "RIDDLES IN THE DARK" -> "Riddles In The Dark.ghost"
 * Strips " RACE" suffix, converts to title-case (every word capitalized,
 * all other letters lowercased), sanitizes invalid filename chars. */
static void race_name_to_filename(const char *raceName, char *out, int outLen) {
    char base[128];
    strncpy(base, raceName, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    /* Strip " RACE" suffix (case-insensitive) */
    int len = strlen(base);
    if (len >= 5 && _stricmp(base + len - 5, " RACE") == 0) {
        base[len - 5] = '\0';
    }

    /* Title-case every word: first letter of each word uppercase,
     * all other letters lowercase. A "word" starts after a space, hyphen,
     * or at the beginning of the string. */
    int newWord = 1;
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == ' ' || c == '-')
            newWord = 1;
        else if (newWord) {
            if (c >= 'a' && c <= 'z') base[i] = c - 32;
            newWord = 0;
        } else {
            if (c >= 'A' && c <= 'Z') base[i] = c + 32;
        }
    }

    /* Sanitize characters that are invalid in Windows filenames.
     * Replace each with underscore so the file is always creatable/openable. */
    for (int i = 0; base[i]; i++) {
        char c = base[i];
        if (c == '\\' || c == '/' || c == ':' || c == '*' ||
            c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
            base[i] = '_';
    }

    snprintf(out, outLen, "%s%s.ghost", g_ghostDir, base);
}

/* Ghost binary file format:
 *   [4 bytes] magic = 0x47485347 ("GHSG")
 *   [4 bytes] version = 1
 *   [4 bytes] time (game ticks, lower = better)
 *   [4 bytes] frame_count
 *   [frame_count * 40 bytes] snapshots (10 DWORDs each, raw memory copy)
 * Total per frame: 40 bytes binary vs ~80 bytes hex text. */

#define GHOST_MAGIC    0x47485347
#define GHOST_VERSION  1

/* Get saved time for a race from its .ghost file (NO_TIME if not found) */
static int get_saved_time(const char *raceName) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NO_TIME;

    int result = NO_TIME;
    DWORD magic, version, time;

    if (ReadFile(h, &magic, 4, &(DWORD){0}, NULL) && magic == GHOST_MAGIC &&
        ReadFile(h, &version, 4, &(DWORD){0}, NULL) && version == GHOST_VERSION &&
        ReadFile(h, &time, 4, &(DWORD){0}, NULL)) {
        result = (int)time;
    }

    CloseHandle(h);
    return result;
}

static void save_ghost_for_race(const char *raceName, int time,
                                DWORD (*snaps)[10], int count) {
    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE h = CreateFileA(path, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        log_fmt("ERROR: cannot create %s", path);
        return;
    }
    DWORD written;

    DWORD magic = GHOST_MAGIC;
    DWORD version = GHOST_VERSION;
    DWORD frameCount = (DWORD)count;

    WriteFile(h, &magic, 4, &written, NULL);
    WriteFile(h, &version, 4, &written, NULL);
    WriteFile(h, (DWORD*)&time, 4, &written, NULL);
    WriteFile(h, &frameCount, 4, &written, NULL);

    /* Write all snapshots as one contiguous block — 10 DWORDs = 40 bytes each */
    if (count > 0)
        WriteFile(h, snaps, count * 40, &written, NULL);

    CloseHandle(h);
}

/* Inline asm wrappers for __thiscall game functions.
 * MinGW __thiscall function pointers silently fail — must use asm. */

static void call_btt_ctor(void *btt) {
    __asm__ volatile(
        "mov %0, %%ecx\n"
        "call *%1\n"
        : : "r"(btt), "r"((void*)ADDR_BTT_CTOR)
        : "eax", "ecx", "edx", "memory"
    );
}

/* Call BTT deleting destructor: __thiscall(btt, flags=1) with RET 0x4.
 * flags=1 calls internal dtor (frees snapshots+list) then operator delete
 * (frees the BTT struct itself) — full destroy matching operator_new. */
static void call_btt_dtor(void *btt) {
    __asm__ volatile(
        "mov %0, %%ecx\n"
        "push $1\n"               /* flags=1 (full delete) */
        "call *%1\n"
        : : "r"(btt), "r"((void*)ADDR_BTT_DTOR)
        : "eax", "ecx", "edx", "memory"
    );
}

static void call_alist_append(DWORD *list, void *item) {
    /* AthenaList_Append is __thiscall(this, item) with RET 0x4 — callee
     * cleans up the 4-byte stack param itself. Do NOT add $4 to ESP after
     * the call; that double-pops and corrupts the stack by 4 bytes per
     * call. 608 calls = 2432-byte stack corruption = crash. */
    __asm__ volatile(
        "mov %0, %%ecx\n"
        "push %1\n"
        "call *%2\n"
        : : "r"(list), "r"(item), "r"((void*)ADDR_ALIST_APPEND)
        : "eax", "ecx", "edx", "memory"
    );
}

static void inject_saved_ghost(const char *raceName) {
    DWORD app = get_app();
    if (!app) return;

    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        log_fmt("No ghost file for '%s' (%s)", raceName, path);
        return;
    }

    int savedTime = NO_TIME;
    DWORD (*savedSnaps)[10] = NULL;
    int savedCount = 0;

    DWORD magic, version, time, frameCount;

    if (ReadFile(hf, &magic, 4, &(DWORD){0}, NULL) && magic == GHOST_MAGIC &&
        ReadFile(hf, &version, 4, &(DWORD){0}, NULL) && version == GHOST_VERSION &&
        ReadFile(hf, &time, 4, &(DWORD){0}, NULL) &&
        ReadFile(hf, &frameCount, 4, &(DWORD){0}, NULL) &&
        frameCount > 0 && frameCount < 200000) {

        savedTime = (int)time;
        savedSnaps = (DWORD(*)[10])malloc(frameCount * 10 * sizeof(DWORD));
        if (savedSnaps) {
            DWORD totalBytes = frameCount * 40;
            DWORD bytesRead = 0;
            if (ReadFile(hf, savedSnaps, totalBytes, &bytesRead, NULL) &&
                bytesRead == totalBytes) {
                savedCount = (int)frameCount;
            } else {
                log_fmt("ERROR: short read — expected %d bytes, got %d", totalBytes, bytesRead);
                free(savedSnaps);
                savedSnaps = NULL;
            }
        }
    } else {
        log_fmt("ERROR: bad ghost file header for '%s'", raceName);
    }
    CloseHandle(hf);

    if (!savedSnaps || savedCount == 0) {
        log_fmt("No valid snapshots in ghost file for '%s' (snaps=%p count=%d)",
               raceName, savedSnaps, savedCount);
        if (savedSnaps) free(savedSnaps);
        return;
    }

    log_fmt("Loading ghost: '%s' time=%d frames=%d", raceName, savedTime, savedCount);

    /* Allocate BTT via game's own operator_new so the game can safely
     * free it later via matching operator_delete. Using malloc here causes
     * heap corruption when the game's destructor calls operator_delete. */
    void *btt = game_operator_new(BTT_SIZE);
    if (!btt) { free(savedSnaps); log_msg("ERROR: alloc BTT failed"); return; }
    log_fmt("BTT allocated at %p (via game operator_new)", btt);

    call_btt_ctor(btt);

    DWORD vtable = *(DWORD*)btt;
    if (vtable != 0x004D262C) {
        log_fmt("ERROR: BTT ctor failed — vtable=0x%X (expected 0x004D262C)", vtable);
        free(savedSnaps);
        /* P2 fix: btt was allocated via game_operator_new but the constructor
         * failed — the vtable is wrong, so calling the BTT destructor would
         * crash on uninitialized AthenaList state. Instead, call the game's
         * CRT _free (0x4BA74D) directly, which is the same _free that
         * operator_new's _malloc pairs with. This releases the 528-byte block
         * without touching the uninitialized internals. */
        game_free(btt);
        return;
    }
    log_msg("BTT ctor OK (vtable verified)");

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;

    char *bttName = (char*)((char*)btt + BTT_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    DWORD *alist = (DWORD*)((char*)btt + 4);
    int numToStore = savedCount;

    for (int i = 0; i < numToStore; i++) {
        /* Each snapshot also via game's operator_new for heap consistency */
        DWORD *snap = (DWORD*)game_operator_new(SNAP_SIZE);
        if (!snap) { log_fmt("ERROR: alloc snap %d failed", i); continue; }
        memcpy(snap, savedSnaps[i], 10 * sizeof(DWORD));
        call_alist_append(alist, snap);
    }
    log_fmt("Appended %d snapshots via game's AthenaList_Append", numToStore);

    *(int*)((char*)btt + 0x41C) = 0;  // playback_index = 0

    *(DWORD*)(app + APP_910_PLAYBACK) = (DWORD)btt;
    free(savedSnaps);
    log_fmt("Ghost injected: %d snapshots into App+0x910 (btt=0x%X)", savedCount, (DWORD)btt);
}

/* --- Dynamic snapshot buffer management (P1 fix) --- */

/* Ensure g_rawSnaps can hold at least 'needed' entries. Grows in MAX_SNAPSHOTS
 * increments to amortize realloc cost. Returns 1 on success, 0 on alloc failure
 * (g_rawSnaps is left unchanged — existing data is preserved). */
static int snaps_reserve(int needed) {
    if (needed <= g_rawCapacity) return 1;
    int newCap = g_rawCapacity ? g_rawCapacity : MAX_SNAPSHOTS;
    while (newCap < needed)
        newCap *= 2;
    DWORD (*newBuf)[SNAP_DWORDS] = (DWORD(*)[SNAP_DWORDS])
        realloc(g_rawSnaps, newCap * SNAP_BYTES);
    if (!newBuf) {
        log_fmt("ERROR: realloc failed for %d snaps (%d bytes)", newCap, newCap * SNAP_BYTES);
        return 0;
    }
    g_rawSnaps = newBuf;
    g_rawCapacity = newCap;
    return 1;
}

/* Reset the snapshot buffer for a new race. Frees the old buffer entirely. */
static void snaps_reset(void) {
    if (g_rawSnaps) {
        free(g_rawSnaps);
        g_rawSnaps = NULL;
    }
    g_rawCount = 0;
    g_rawCapacity = 0;
}

static void check_race_state(void) {
    DWORD app = get_app();
    if (!app) return;

    EnterCriticalSection(&g_cs);

    int tt = is_time_trial_active();
    if (!tt) {
        if (g_recording) {
            log_fmt("Left Time Trial mode (was recording %d frames)", g_rawCount);
            g_recording = 0;
            g_raceFinished = 0;
            snaps_reset();
            g_prevGoalFlag = 0;
            g_currentRaceName[0] = '\0';
            g_hookRaceName[0] = '\0';
        }
        if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
            g_prevRecording = *(DWORD*)(app + APP_90C_RECORDING);
        LeaveCriticalSection(&g_cs);
        return;
    }

    DWORD currRecording = 0;
    if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
        currRecording = *(DWORD*)(app + APP_90C_RECORDING);

    if (currRecording != g_prevRecording && currRecording && currRecording > 0x10000) {
        g_prevRecording = currRecording;

        char raceName[128];
        /* Prefer the name from the pre-inject hook (from static table) —
         * the BTT name at +0x424 can be partially written early in the
         * race, causing garbled names like 'Y' instead of 'BEGINNER RACE'. */
        if (g_hookRaceName[0]) {
            strncpy(raceName, g_hookRaceName, sizeof(raceName) - 1);
            raceName[sizeof(raceName) - 1] = '\0';
        } else {
            get_race_name(raceName, sizeof(raceName));
        }
        if (raceName[0]) {
            strncpy(g_currentRaceName, raceName, sizeof(g_currentRaceName) - 1);
            g_currentRaceName[sizeof(g_currentRaceName) - 1] = '\0';
            snaps_reset();
            g_recording = 1;
            g_raceFinished = 0;
            /* Set prevGoalFlag to CURRENT value, not 0. If the previous race's
             * goal flag (App+0x5D6) is still 1 when the new race starts, our
             * 16ms thread could see goalFlag=1 && prevGoalFlag=0 → false goal
             * detection with 0 snapshots, permanently blocking the real goal.
             * By syncing to the current value, we require a 0→1 transition
             * (the game clears it to 0 at race start, then sets it to 1 on goal). */
            if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1))
                g_prevGoalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            else
                g_prevGoalFlag = 0;
            log_fmt("RACE START: '%s' (BTT=0x%X, goalFlag=%d)", raceName, currRecording, g_prevGoalFlag);
        } else {
            log_fmt("Race detected (BTT=0x%X) but name not ready, will retry", currRecording);
            g_prevRecording = 0;
        }
    }

    /* Recording is handled by the game itself (App+0x90C BTT). We read the
     * game's own snapshots at goal time — no need to poll in our thread. */

    if (g_recording && !g_raceFinished) {
        if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
            BYTE goalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            if (goalFlag && !g_prevGoalFlag) {
                g_raceFinished = 1;

                int finishTime = NO_TIME;
                DWORD btt = 0;
                if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) {
                    btt = *(DWORD*)(app + APP_90C_RECORDING);
                    if (btt && btt > 0x10000 && !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
                        finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
                }
                log_fmt("GOAL! finishTime=%d", finishTime);

                if (finishTime != NO_TIME && btt && g_currentRaceName[0]) {
                    /* Read the game's own recording from App+0x90C's AthenaList.
                     * The game records at its internal frame rate (matching
                     * playback), so this avoids the 2x speed problem caused
                     * by our 60Hz polling thread recording at double rate.
                     *
                     * BTT layout: vtable@+0, AthenaList starts at +0x004:
                     *   BTT+0x004 = AthenaList vtable
                     *   BTT+0x008 = AthenaList count
                     *   BTT+0x410 = AthenaList data array pointer */
                    if (!IsBadReadPtr((void*)(btt + 8), 4)) {
                        DWORD count = *(DWORD*)(btt + 8);  /* AthenaList count */
                        if (!IsBadReadPtr((void*)(btt + 0x410), 4)) {
                            DWORD *data = *(DWORD**)(btt + 0x410);  /* AthenaList data ptr */
                            if (count > 0 && data &&
                                (DWORD)data > 0x10000 && !IsBadReadPtr(data, count * 4)) {
                                log_fmt("Reading %d frames from game's recording BTT", count);

                                /* P1 fix: dynamically grow buffer for long races.
                                 * Old code truncated at MAX_SNAPSHOTS(5000) = ~83s. */
                                if (!snaps_reserve((int)count)) {
                                    log_msg("ERROR: cannot allocate snapshot buffer — skipping save");
                                } else {
                                    g_rawCount = 0;
                                    for (int i = 0; i < (int)count; i++) {
                                        DWORD *snap = (DWORD*)data[i];
                                        if (snap && (DWORD)snap > 0x10000 &&
                                            !IsBadReadPtr(snap, SNAP_BYTES)) {
                                            memcpy(g_rawSnaps[g_rawCount], snap, SNAP_BYTES);
                                            g_rawCount++;
                                        }
                                    }
                                    log_fmt("Read %d snapshots from game recording", g_rawCount);
                                }
                            } else {
                                log_fmt("ERROR: bad BTT list count=%d data=0x%X", count, (DWORD)data);
                            }
                        }
                    }

                    if (g_rawCount > 0) {
                        /* Always save the most recent run as PreviousRun.ghost
                         * so it's always available regardless of best time. */
                        save_ghost_for_race("PreviousRun", finishTime,
                                           g_rawSnaps, g_rawCount);
                        log_msg("Previous run saved");

                        int existingTime = get_saved_time(g_currentRaceName);
                        if (existingTime == NO_TIME) {
                            log_fmt("No existing ghost for '%s' — saving", g_currentRaceName);
                            save_ghost_for_race(g_currentRaceName, finishTime,
                                               g_rawSnaps, g_rawCount);
                            log_msg("Ghost saved");
                        } else if (finishTime < existingTime) {
                            log_fmt("New time %d < saved time %d — overwriting",
                                   finishTime, existingTime);
                            save_ghost_for_race(g_currentRaceName, finishTime,
                                               g_rawSnaps, g_rawCount);
                            log_msg("Ghost saved");
                        } else {
                            log_fmt("New time %d >= saved time %d — discarding",
                                   finishTime, existingTime);
                        }
                    } else {
                        /* 0 snapshots means we caught a stale goal flag from
                         * the previous race. Don't treat as finished — reset
                         * so the real goal crossing is detected later. */
                        log_msg("0 snapshots — likely stale goal flag, resetting");
                        g_raceFinished = 0;
                    }
                } else {
                    log_fmt("NOT saving: finishTime=%d btt=0x%X name='%s'",
                           finishTime, btt, g_currentRaceName);
                }
            }
            g_prevGoalFlag = goalFlag;
        }
    }

    LeaveCriticalSection(&g_cs);
}

/* App_StartPracticeRace detour hook.
 * App_StartPracticeRace (0x428C50) is __thiscall(App*, DWORD race_index).
 * It takes 1 stack parameter (race_index) and ends with RET 0x4.
 * It creates the ghost ball when App+0x910 is non-NULL.
 *
 * First bytes: 6A FF 68 B6 AE 4C 00 64 A1 00 00 00 00 50 64 89
 *   push 0xFFFFFFFF           (2 bytes)
 *   push 0x004CAEB6           (5 bytes)  = 7 bytes (instruction boundary)
 * We copy 7 bytes for the trampoline, write JMP+2NOPs. */

#define ADDR_APP_START_PRACTICE  0x00428C50
#define HOOK_BYTES               7
#define TRAMPOLINE_SIZE          16

static unsigned char *g_trampoline = NULL;
static unsigned char g_origBytes[HOOK_BYTES];
static int g_hookInstalled = 0;

/* Get race name from the static tournament name table by race index.
 * Used pre-call (before PlayerProfile/BTT exist) to look up the name.
 * Table at 0x4F7080: array of DWORD pointers to race name strings. */
static int get_race_name_by_index(DWORD race_index, char *out, int outLen) {
    if (race_index >= 16) return 0;
    DWORD *nameTable = (DWORD*)0x004F7080;
    if (IsBadReadPtr(nameTable + race_index, 4)) return 0;
    char *name = (char*)nameTable[race_index];
    if (!name || (DWORD)name < 0x400000) return 0;
    if (IsBadReadPtr(name, 2)) return 0;
    if (name[0] < 0x20 || name[0] > 0x7E) return 0;
    strncpy(out, name, outLen - 1);
    out[outLen - 1] = '\0';
    return 1;
}

/* Check if this is a Time Trial race by looking at App+0x234 (party mode).
 * Pre-call, we can't check profile+0x11 (profile doesn't exist yet), so we
 * use party_mode==0 as a heuristic. The full check is done post-call. */
static int is_time_trial_precheck(void) {
    DWORD app = get_app();
    if (!app) return 0;
    if (IsBadReadPtr((void*)(app + APP_234_PARTY_MODE), 1)) return 0;
    if (*(BYTE*)(app + APP_234_PARTY_MODE) != 0) return 0;
    return 1;
}

/* hook_impl receives both App (ECX at entry) and race_index ([ESP+4] at entry).
 *
 * CRITICAL: We must set App+0x910 BEFORE calling the original function, not
 * after. The original App_StartPracticeRace calls App_StartRace → Board_ctor,
 * and Board_ctor checks App+0x910 to decide whether to create the ghost ball
 * (stored at scene+0x361C). If App+0x910 is NULL when Board_ctor runs, no
 * ghost ball is created. Then Level_UpdateAndRender sees App+0x910 != NULL
 * (set by our post-hook injection) and calls BestTimeTracker_PlaybackSnapshot
 * with a NULL ghost_ball_ptr → crash at 0x4276B6 (deref NULL+0x16C).
 *
 * But there's a catch: App_StartPracticeRace's BTT management code will
 * DESTROY our injected App+0x910 if App+0x90C (recording) is NULL — it
 * enters the "one is NULL" branch and frees the playback BTT. To survive
 * this, we must ALSO pre-set App+0x90C with a dummy BTT so the game's
 * BTT management enters the "both exist" branch and keeps our playback.
 *
 * After the original returns, we destroy the old App+0x910 BTT that we
 * replaced. At that point:
 *   ✅ The old scene has been torn down (step 1 complete)
 *   ✅ The game's BTT management has run (step 2 complete — it managed our
 *      new BTT, not the old one)
 *   ✅ No live references to the old BTT remain — the old ghost ball is gone
 *   ✅ Safe to call vtable[0](1) on the saved pointer (full delete) */
void hook_impl(DWORD app, DWORD race_index) {
    g_savedOldPlayback = 0;

    EnterCriticalSection(&g_cs);

    /* Pre-inject: set App+0x910 before calling original so Board_ctor creates
     * the ghost ball. We need the race name, looked up by race index. */
    if (is_time_trial_precheck()) {
        char raceName[128] = "";
        if (get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
            log_fmt("HOOK: pre-inject for race '%s' (index=%d)", raceName, race_index);
            /* Store the authoritative race name from the static table so
             * check_race_state() uses this instead of reading from the BTT
             * (which can have a partially-written name early in the race). */
            strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
            g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\0';

            /* Check if we have a saved ghost for this race */
            int savedTime = get_saved_time(raceName);
            if (savedTime != NO_TIME) {
                /* Save old App+0x910 before overwriting so we can destroy it
                 * after the trampoline returns. The game never sees this old
                 * pointer (we replaced it before the trampoline runs), so it
                 * can't destroy it — that's the leak we're fixing. */
                if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_910_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                        log_fmt("Saved old App+0x910 (0x%X) for post-hook destruction", existing);
                    }
                    /* inject_saved_ghost sets App+0x910 to the new BTT,
                     * overwriting whatever was there */
                    inject_saved_ghost(raceName);
                }

                /* If App+0x90C (recording) is NULL, the game's BTT management
                 * will destroy our App+0x910. Create a dummy recording BTT with
                 * NO_TIME so the game enters the "both exist" branch and keeps
                 * our playback (since NO_TIME will not beat savedTime).
                 *
                 * Also: if App+0x90C already has a recording from a PREVIOUS
                 * race (e.g. Warm-Up time=373), its time could beat our
                 * injected playback (e.g. Beginner time=1414), causing the
                 * game to destroy our ghost and promote the wrong-race
                 * recording. Fix: set the existing recording's time to
                 * NO_TIME so it loses the comparison. */
                if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) {
                    DWORD recording = *(DWORD*)(app + APP_90C_RECORDING);
                    if (recording && recording > 0x10000 &&
                        !IsBadReadPtr((void*)(recording + BTT_BEST_TIME), 4)) {
                        int oldTime = *(int*)((char*)recording + BTT_BEST_TIME);
                        if (oldTime != NO_TIME) {
                            log_fmt("Neutralizing old recording time %d -> NO_TIME", oldTime);
                            *(int*)((char*)recording + BTT_BEST_TIME) = NO_TIME;
                        }
                    }
                    if (!recording || recording < 0x10000) {
                        log_msg("Pre-creating dummy recording BTT to protect playback");
                        void *dummyRec = game_operator_new(BTT_SIZE);
                        if (dummyRec) {
                            call_btt_ctor(dummyRec);
                            DWORD vt = *(DWORD*)dummyRec;
                            if (vt == 0x004D262C) {
                                *(DWORD*)((char*)dummyRec + BTT_BEST_TIME) = NO_TIME;
                                *(DWORD*)(app + APP_90C_RECORDING) = (DWORD)dummyRec;
                                log_fmt("Dummy recording BTT at 0x%X (NO_TIME)", (DWORD)dummyRec);
                            } else {
                                log_fmt("ERROR: dummy BTT ctor vtable=0x%X", vt);
                                /* Free failed ctor allocation via CRT _free — can't call
                                 * destructor on uninitialized vtable/AthenaList state. */
                                game_free(dummyRec);
                            }
                        }
                    }
                }
            } else {
                log_fmt("No saved ghost for '%s', clearing stale playback", raceName);
                /* No saved ghost for this race — clear App+0x910 so the
                 * ghost ball from a previous race doesn't show up.
                 * Save old pointer for post-hook destruction (same leak fix). */
                if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_910_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                        log_fmt("Saved old App+0x910 (0x%X) for post-hook destruction", existing);
                    }
                    *(DWORD*)(app + APP_910_PLAYBACK) = 0;
                }
            }
        } else {
            log_fmt("HOOK: could not resolve race name for index %d", race_index);
        }
    }

    /* Release the lock during the trampoline call — the original
     * App_StartPracticeRace runs game code (Board_ctor, scene setup, etc.)
     * for potentially hundreds of milliseconds. Holding the lock would
     * stall the background thread unnecessarily. The post-trampoline
     * cleanup re-acquires the lock. */
    LeaveCriticalSection(&g_cs);

    /* Call original App_StartPracticeRace via trampoline. Board_ctor will see
     * App+0x910 (if we set it) and create the ghost ball at scene+0x361C. */
    __asm__ volatile(
        "mov %0, %%ecx\n"
        "push %2\n"
        "call *%1\n"
        : : "r"(app), "r"((void*)g_trampoline), "r"(race_index)
        : "eax", "ecx", "edx", "memory"
    );

    log_msg("HOOK: App_StartPracticeRace returned");

    EnterCriticalSection(&g_cs);

    /* Destroy the old App+0x910 BTT that we replaced before the trampoline.
     * At this point the old scene is torn down, the old ghost ball is gone,
     * and the game's BTT management has already run (on our new BTT, not the
     * old one). No live references to the old BTT remain — safe to destroy. */
    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {
        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {
            DWORD vt = *(DWORD*)g_savedOldPlayback;
            if (vt == 0x004D262C) {
                log_fmt("Destroying old playback BTT at 0x%X", g_savedOldPlayback);
                call_btt_dtor((void*)g_savedOldPlayback);
                log_msg("Old playback BTT destroyed");
            } else {
                log_fmt("WARNING: old playback BTT vtable=0x%X (expected 0x4D262C) — skipping destroy",
                        vt);
            }
        }
        g_savedOldPlayback = 0;
    }

    LeaveCriticalSection(&g_cs);
}

/* Naked stub: extracts ECX (App) and [ESP+4] (race_index) from the
 * __thiscall entry, passes them to hook_impl, then returns with RET 4
 * to clean up the caller's stack parameter. */
__attribute__((naked, used)) static void hook_App_StartPracticeRace(void) {
    __asm__ volatile(
        "pushl %%eax\n"
        "pushl %%ecx\n"
        "pushl %%edx\n"
        /* Stack now: [0]=edx [4]=ecx [8]=eax [12]=ret_addr [16]=race_index */
        "movl 16(%%esp), %%eax\n"   /* race_index */
        "pushl %%eax\n"             /* 2nd param: race_index */
        "pushl %%ecx\n"             /* 1st param: App */
        "call _hook_impl\n"
        "addl $8, %%esp\n"          /* clean up 2 params */
        "popl %%edx\n"
        "popl %%ecx\n"
        "popl %%eax\n"
        "ret $4\n"                  /* clean up caller's race_index */
        : : : "memory"
    );
}

static void install_hook(void) {
    unsigned char *target = (unsigned char*)ADDR_APP_START_PRACTICE;

    memcpy(g_origBytes, target, HOOK_BYTES);
    log_fmt("Original bytes: %02X %02X %02X %02X %02X %02X %02X",
            g_origBytes[0], g_origBytes[1], g_origBytes[2], g_origBytes[3],
            g_origBytes[4], g_origBytes[5], g_origBytes[6]);

    g_trampoline = (unsigned char*)VirtualAlloc(NULL, TRAMPOLINE_SIZE,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!g_trampoline) {
        log_msg("ERROR: VirtualAlloc for trampoline failed");
        return;
    }

    memcpy(g_trampoline, g_origBytes, HOOK_BYTES);

    DWORD jmp_src = (DWORD)(g_trampoline + HOOK_BYTES + 5);
    DWORD jmp_dst = ADDR_APP_START_PRACTICE + HOOK_BYTES;
    g_trampoline[HOOK_BYTES + 0] = 0xE9;
    *(DWORD*)(g_trampoline + HOOK_BYTES + 1) = jmp_dst - jmp_src;

    log_fmt("Trampoline at 0x%X, JMP target 0x%X", (DWORD)g_trampoline, jmp_dst);

    DWORD oldProtect;
    if (!VirtualProtect(target, HOOK_BYTES, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        log_msg("ERROR: VirtualProtect failed");
        return;
    }

    DWORD hookAddr = (DWORD)&hook_App_StartPracticeRace;
    target[0] = 0xE9;
    *(DWORD*)(target + 1) = hookAddr - (DWORD)target - 5;
    target[5] = 0x90;
    target[6] = 0x90;

    VirtualProtect(target, HOOK_BYTES, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, HOOK_BYTES);

    g_hookInstalled = 1;
    log_fmt("Hook installed: JMP 0x%X -> 0x%X", ADDR_APP_START_PRACTICE, hookAddr);
}

static DWORD WINAPI ghost_thread(LPVOID param) {
    Sleep(3000);
    log_msg("Ghost thread v24 started");
    while (1) {
        Sleep(16);
        check_race_state();
    }
    return 0;
}

static void init_paths(HMODULE hInst) {
    char path[MAX_PATH];
    if (GetModuleFileNameA(hInst, path, MAX_PATH)) {
        char *p = strrchr(path, '\\');
        if (p) {
            /* Create Ghosts/ directory next to bass.dll */
            strcpy(p + 1, "Ghosts\\");
            strncpy(g_ghostDir, path, MAX_PATH - 1);
            g_ghostDir[MAX_PATH - 1] = '\0';
            CreateDirectoryA(g_ghostDir, NULL);
            /* Ignore error if dir already exists */

            p = strrchr(path, '\\');
            if (p) {
                strcpy(p + 1, "ghost_saver_log.txt");
                strncpy(g_logPath, path, MAX_PATH - 1);
                g_logPath[MAX_PATH - 1] = '\0';
            }
        }
    }
}

static void init_mod(HMODULE hInst) {
    init_paths(hInst);
    InitializeCriticalSection(&g_cs);
    log_msg("=== Ghost Saver Mod v24 Init ===");
    log_fmt("Ghost dir: %s", g_ghostDir);
    log_fmt("Log path: %s", g_logPath);

    install_hook();
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
