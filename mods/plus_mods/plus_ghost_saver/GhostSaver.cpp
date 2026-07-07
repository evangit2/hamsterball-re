/* GhostSaver.cpp — Main mod class for Hamsterball Plus API.
 *
 * Persistent Ghost Data for Time Trial Mode.
 * Saves/loads ghost ball recordings to/from .ghost files in the Ghosts/
 * directory, so Time Trial ghosts survive game restarts.
 *
 * Converted from ghost_saver.c v25.4 (bass.dll proxy) to HB+ API.
 * Key simplification: no background thread, no CRITICAL_SECTION,
 * no inline asm — everything runs on the main thread via callbacks.
 *
 * Files:
 *   GhostSaver.h          — constants, globals, declarations
 *   GhostSaverUtils.cpp   — race name lookup, file I/O, state checks
 *   GhostSaverHook.cpp    — App_StartPracticeRace hook + ghost injection
 *   GhostSaver.cpp        — mod class (this file): Initialize, onGameUpdate
 */
#include "GhostSaver.h"

/* ── Race state monitoring (replaces background thread) ──
 *
 * Called every tick from the main thread via onGameUpdate().
 * Detects:
 *   1. New recording BTT at App+0x90C (new race started)
 *   2. Goal flag 0→1 transition (race finished)
 *   3. Left Time Trial mode (cleanup)
 *
 * When the goal is crossed, reads the game's own recording from the
 * BTT's AthenaList and saves it to a .ghost file. Only saves if the
 * finish time is better than the existing ghost (or no ghost exists).
 * Always saves the most recent run as Previous_Run.ghost. */
static void check_race_state(void) {
    DWORD app = get_app();
    if (!app) return;

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
        cleanup_dummy_btt(app);
        if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
            g_prevRecording = *(DWORD*)(app + APP_90C_RECORDING);
        return;
    }

    /* Detect new recording BTT (new race started) */
    DWORD currRecording = 0;
    if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
        currRecording = *(DWORD*)(app + APP_90C_RECORDING);

    if (currRecording != g_prevRecording && currRecording && currRecording > 0x10000) {
        g_prevRecording = currRecording;

        char raceName[128];
        /* Prefer hook name (from static table — BTT name can be partially
         * written early in the race, causing garbled names). */
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
            /* Set prevGoalFlag to CURRENT value (not 0) to prevent stale
             * goal flag false-trigger from previous race. */
            if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1))
                g_prevGoalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            else
                g_prevGoalFlag = 0;
            log_fmt("RACE START: '%s' (BTT=0x%X)", raceName, currRecording);
        } else {
            log_fmt("Race detected (BTT=0x%X) but name not ready, will retry", currRecording);
            g_prevRecording = 0;
        }
    }

    /* Monitor goal flag for 0→1 transition (race finished) */
    if (g_recording && !g_raceFinished) {
        if (!IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1)) {
            BYTE goalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
            if (goalFlag && !g_prevGoalFlag) {
                g_raceFinished = 1;

                int finishTime = NO_TIME;
                DWORD btt = 0;
                if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) {
                    btt = *(DWORD*)(app + APP_90C_RECORDING);
                    if (btt && btt > 0x10000 &&
                        !IsBadReadPtr((void*)(btt + BTT_BEST_TIME), 4))
                        finishTime = *(DWORD*)(btt + BTT_BEST_TIME);
                }
                log_fmt("GOAL! finishTime=%d", finishTime);

                if (finishTime != NO_TIME && btt && g_currentRaceName[0]) {
                    /* Read the game's own recording from App+0x90C's AthenaList.
                     * BTT layout: vtable@+0, AthenaList starts at +0x004:
                     *   BTT+0x008 = AthenaList count
                     *   BTT+0x410 = AthenaList data array pointer */
                    if (!IsBadReadPtr((void*)(btt + 8), 4)) {
                        DWORD count = *(DWORD*)(btt + 8);
                        if (!IsBadReadPtr((void*)(btt + 0x410), 4)) {
                            DWORD *data = *(DWORD**)(btt + 0x410);
                            if (count > 0 && count < 200000 && data &&
                                (DWORD)data > 0x10000 &&
                                !IsBadReadPtr(data, count * 4)) {
                                log_fmt("Reading %d frames from game recording", count);

                                snaps_reserve((int)count);
                                if (g_rawSnaps) {
                                    g_rawCount = 0;
                                    for (int i = 0; i < (int)count; i++) {
                                        DWORD *snap = (DWORD*)data[i];
                                        if (snap && (DWORD)snap > 0x10000 &&
                                            !IsBadReadPtr(snap, SNAP_BYTES)) {
                                            memcpy(g_rawSnaps[g_rawCount], snap, SNAP_BYTES);
                                            g_rawCount++;
                                        }
                                    }
                                    log_fmt("Read %d snapshots", g_rawCount);
                                }
                            } else {
                                log_fmt("ERROR: bad BTT list count=%d data=0x%X", count, (DWORD)data);
                            }
                        }
                    }

                    if (g_rawCount > 0) {
                        /* Always save most recent run as Previous_Run.ghost */
                        save_ghost_for_race("Previous_Run", finishTime,
                                           g_rawSnaps, g_rawCount);

                        /* Save per-race ghost only if better than existing */
                        int existingTime = get_saved_time(g_currentRaceName);
                        if (existingTime == NO_TIME) {
                            log_fmt("No existing ghost — saving");
                            save_ghost_for_race(g_currentRaceName, finishTime,
                                               g_rawSnaps, g_rawCount);
                        } else if (finishTime < existingTime) {
                            log_fmt("New time %d < saved %d — overwriting",
                                   finishTime, existingTime);
                            save_ghost_for_race(g_currentRaceName, finishTime,
                                               g_rawSnaps, g_rawCount);
                        } else {
                            log_fmt("New time %d >= saved %d — discarding",
                                   finishTime, existingTime);
                        }
                    } else {
                        /* 0 snapshots = stale goal flag — reset for real goal */
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
}

/* ── Mod class ── */

class GhostSaverMod : public HamsterballAPI {
private:
public:
    const char* GetModName() override { return "Ghost Saver"; }
    const char* GetAuthorName() override { return "BookwormKevin"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        g_api = modApi;

        /* Create toggle button (default ON) */
        CustomButton btn("GHOST_SAVER", "Save Ghosts");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        /* Initialize Ghosts/ directory */
        init_ghost_dir();

        /* Install hook on App_StartPracticeRace.
         * RegisterCustomHook takes ABSOLUTE address (0x428C50).
         * The hook function uses __fastcall convention. */
        modApi->RegisterCustomHook(ADDR_APP_START_PRACTICE,
            (void*)hook_AppStartPracticeRace,
            (void**)&orig_AppStartPracticeRace);

        log_msg("=== Ghost Saver Mod (HB+) Initialized ===");
        log_fmt("Ghost dir: %s", g_ghostDir);
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        if (strcmp(buttonId, "GHOST_SAVER") == 0) {
            g_enabled = newState;
            log_fmt("Ghost Saver: %s", g_enabled ? "ENABLED" : "DISABLED");
        }
    }

    void onGameUpdate() override {
        if (!g_enabled) return;
        if (!g_api) return;
        check_race_state();
    }

    void onSceneEnd() override {
        /* Reset recording state when leaving a level */
        g_recording = 0;
        g_raceFinished = 0;
        g_prevGoalFlag = 0;
        g_currentRaceName[0] = '\0';
        g_hookRaceName[0] = '\0';
        /* Don't reset g_prevRecording here — let check_race_state
         * detect the new race naturally. */
    }

    void onLevelStart() override {
        /* Reset goal flag to current value to prevent stale triggers */
        DWORD app = get_app();
        if (app && !IsBadReadPtr((void*)(app + APP_5D6_GOAL_FLAG), 1))
            g_prevGoalFlag = *(BYTE*)(app + APP_5D6_GOAL_FLAG);
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new GhostSaverMod();
}
