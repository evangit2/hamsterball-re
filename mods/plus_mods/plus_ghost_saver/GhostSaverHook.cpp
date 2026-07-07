/* GhostSaverHook.cpp — Hook for App_StartPracticeRace and ghost injection.
 *
 * This file contains the detour hook (installed via RegisterCustomHook) and
 * the inject_saved_ghost function. The hook pre-injects the ghost BTT at
 * App+0x910 BEFORE calling the original App_StartPracticeRace, so that
 * Board_ctor (called inside it) creates the ghost ball at scene+0x361C.
 *
 * After the original returns, we destroy the old App+0x910 BTT that we replaced
 * (no live references remain — old scene is torn down, game's BTT management
 * has already run on our new BTT).
 */
#include "GhostSaver.h"

/* Load a saved ghost from disk and inject it into App+0x910.
 * Creates a BTT via game's operator_new, calls the BTT constructor,
 * copies all snapshots into the BTT's AthenaList, then stores the
 * BTT pointer at App+0x910 so Board_ctor sees it and creates a ghost ball. */
static void inject_saved_ghost(const char *raceName) {
    DWORD app = get_app();
    if (!app) return;

    char path[MAX_PATH];
    race_name_to_filename(raceName, path, sizeof(path));

    HANDLE hf = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        log_fmt("No ghost file for '%s'", raceName);
        return;
    }

    int savedTime = NO_TIME;
    DWORD (*savedSnaps)[10] = NULL;
    int savedCount = 0;

    DWORD magic, version, time, frameCount;
    DWORD bytesRead;

    if (ReadFile(hf, &magic, 4, &bytesRead, NULL) && magic == GHOST_MAGIC &&
        ReadFile(hf, &version, 4, &bytesRead, NULL) && version == GHOST_VERSION &&
        ReadFile(hf, &time, 4, &bytesRead, NULL) &&
        ReadFile(hf, &frameCount, 4, &bytesRead, NULL) &&
        frameCount > 0 && frameCount < 200000) {

        savedTime = (int)time;
        savedSnaps = (DWORD(*)[10])malloc(frameCount * 10 * sizeof(DWORD));
        if (savedSnaps) {
            DWORD totalBytes = frameCount * SNAP_BYTES;
            if (ReadFile(hf, savedSnaps, totalBytes, &bytesRead, NULL) &&
                bytesRead == totalBytes) {
                savedCount = (int)frameCount;
            } else {
                log_fmt("ERROR: short read for ghost");
                free(savedSnaps);
                savedSnaps = NULL;
            }
        }
    } else {
        log_fmt("ERROR: bad ghost file header for '%s'", raceName);
    }
    CloseHandle(hf);

    if (!savedSnaps || savedCount == 0) {
        if (savedSnaps) free(savedSnaps);
        return;
    }

    log_fmt("Loading ghost: '%s' time=%d frames=%d", raceName, savedTime, savedCount);

    /* Allocate BTT via game's operator_new so the game can safely free it */
    void *btt = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
    if (!btt) { free(savedSnaps); return; }
    log_fmt("BTT allocated at %p", btt);

    /* Call BTT constructor */
    CallMethod<void>(RVA_BTT_CTOR, btt);

    DWORD vtable = *(DWORD*)btt;
    if (vtable != BTT_VTABLE) {
        log_fmt("ERROR: BTT ctor failed — vtable=0x%X", vtable);
        free(savedSnaps);
        Call<void>(RVA_GAME_FREE, btt);
        return;
    }

    *(DWORD*)((char*)btt + BTT_BEST_TIME) = savedTime;

    char *bttName = (char*)((char*)btt + BTT_NAME);
    strncpy(bttName, raceName, 127);
    bttName[127] = '\0';

    /* Append each snapshot to the BTT's AthenaList.
     * BTT+0x004 = embedded AthenaList (its "this" pointer for Append). */
    DWORD *alist = (DWORD*)((char*)btt + 4);
    for (int i = 0; i < savedCount; i++) {
        DWORD *snap = (DWORD*)Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)SNAP_SIZE);
        if (!snap) continue;
        memcpy(snap, savedSnaps[i], SNAP_BYTES);
        CallMethod<void>(RVA_ALIST_APPEND, alist, snap);
    }
    log_fmt("Appended %d snapshots", savedCount);

    *(int*)((char*)btt + 0x41C) = 0;  /* playback_index = 0 */

    *(DWORD*)(app + APP_910_PLAYBACK) = (DWORD)btt;
    free(savedSnaps);
    log_fmt("Ghost injected into App+0x910 (btt=%p)", btt);
}

/* The hook for App_StartPracticeRace.
 * __fastcall: ECX=App (this), [ESP+4]=race_index.
 *
 * Pre-inject: set App+0x910 with saved ghost BTT BEFORE calling original,
 * so Board_ctor creates the ghost ball. Also set App+0x90C with a dummy
 * recording BTT (NO_TIME) so the game's BTT management keeps our playback.
 *
 * Post-call: destroy the old App+0x910 BTT we replaced (safe — old scene
 * is torn down, no live references remain). */
static void __fastcall hook_AppStartPracticeRace(void* app_ptr, void* edx, DWORD race_index) {
    DWORD app = (DWORD)app_ptr;
    log_fmt("HOOK: App_StartPracticeRace(app=0x%X, race_index=%d)", app, race_index);

    /* Clean up stale dummy from previous race if still present */
    if (g_dummyRecording && g_dummyRecording > 0x10000) {
        DWORD curr90C = 0;
        if (!IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4))
            curr90C = *(DWORD*)(app + APP_90C_RECORDING);
        if (curr90C == g_dummyRecording) {
            if (!IsBadReadPtr((void*)g_dummyRecording, 4)) {
                DWORD vt = *(DWORD*)g_dummyRecording;
                if (vt == BTT_VTABLE)
                    CallMethod<void>(RVA_BTT_DTOR, (void*)g_dummyRecording, (DWORD)1);
                else
                    Call<void>(RVA_GAME_FREE, (void*)g_dummyRecording);
            }
            *(DWORD*)(app + APP_90C_RECORDING) = 0;
        }
        g_dummyRecording = 0;
    }

    g_savedOldPlayback = 0;

    if (is_time_trial_precheck()) {
        char raceName[128] = "";
        if (get_race_name_by_index(race_index, raceName, sizeof(raceName)) && raceName[0]) {
            log_fmt("HOOK: pre-inject for race '%s'", raceName);
            strncpy(g_hookRaceName, raceName, sizeof(g_hookRaceName) - 1);
            g_hookRaceName[sizeof(g_hookRaceName) - 1] = '\0';

            int savedTime = get_saved_time(raceName);
            if (savedTime != NO_TIME) {
                /* Save old App+0x910 for post-hook destruction */
                if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_910_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                        log_fmt("Saved old App+0x910 (0x%X) for post-hook destruction", existing);
                    }
                    /* Inject ghost — sets App+0x910 to new BTT */
                    inject_saved_ghost(raceName);

                    /* Check if injection succeeded */
                    DWORD newPlayback = 0;
                    if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4))
                        newPlayback = *(DWORD*)(app + APP_910_PLAYBACK);

                    int injectFailed = 0;
                    if (g_savedOldPlayback && newPlayback == g_savedOldPlayback) {
                        /* inject_saved_ghost failed — App+0x910 unchanged */
                        log_msg("inject failed — leaving App+0x910 unchanged");
                        g_savedOldPlayback = 0;
                        injectFailed = 1;
                    }

                    /* Create dummy recording BTT at App+0x90C to protect
                     * playback from game's BTT management. The dummy has
                     * NO_TIME so it loses every comparison and gets freed
                     * by the game normally. We only create it if injection
                     * actually changed App+0x910. */
                    if (!injectFailed && newPlayback && newPlayback > 0x10000 &&
                        !IsBadReadPtr((void*)(app + APP_90C_RECORDING), 4)) {
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
                            log_msg("Pre-creating dummy recording BTT");
                            void *dummyRec = Call<void*>(RVA_OPERATOR_NEW, (SIZE_T)BTT_SIZE);
                            if (dummyRec) {
                                CallMethod<void>(RVA_BTT_CTOR, dummyRec);
                                DWORD vt = *(DWORD*)dummyRec;
                                if (vt == BTT_VTABLE) {
                                    *(DWORD*)((char*)dummyRec + BTT_BEST_TIME) = NO_TIME;
                                    *(DWORD*)(app + APP_90C_RECORDING) = (DWORD)dummyRec;
                                    g_dummyRecording = (DWORD)dummyRec;
                                    log_fmt("Dummy recording BTT at 0x%X", (DWORD)dummyRec);
                                } else {
                                    log_fmt("ERROR: dummy BTT ctor vtable=0x%X", vt);
                                    Call<void>(RVA_GAME_FREE, dummyRec);
                                }
                            }
                        }
                    }
                }
            } else {
                /* No saved ghost — clear App+0x910 so old ghost doesn't show */
                log_fmt("No saved ghost for '%s', clearing playback", raceName);
                if (!IsBadReadPtr((void*)(app + APP_910_PLAYBACK), 4)) {
                    DWORD existing = *(DWORD*)(app + APP_910_PLAYBACK);
                    if (existing && existing > 0x10000) {
                        g_savedOldPlayback = existing;
                    }
                    *(DWORD*)(app + APP_910_PLAYBACK) = 0;
                }
            }
        } else {
            log_fmt("HOOK: could not resolve race name for index %d", race_index);
        }
    }

    /* Call original App_StartPracticeRace via the saved trampoline.
     * HB+ RegisterCustomHook provides the original function pointer. */
    orig_AppStartPracticeRace(app_ptr, edx, race_index);

    log_msg("HOOK: App_StartPracticeRace returned");

    /* Destroy old App+0x910 BTT that we replaced before the trampoline.
     * Safe now: old scene torn down, game's BTT management already ran. */
    if (g_savedOldPlayback && g_savedOldPlayback > 0x10000) {
        if (!IsBadReadPtr((void*)g_savedOldPlayback, 4)) {
            DWORD vt = *(DWORD*)g_savedOldPlayback;
            if (vt == BTT_VTABLE) {
                log_fmt("Destroying old playback BTT at 0x%X", g_savedOldPlayback);
                CallMethod<void>(RVA_BTT_DTOR, (void*)g_savedOldPlayback, (DWORD)1);
                log_msg("Old playback BTT destroyed");
            } else {
                log_fmt("WARNING: old playback BTT vtable=0x%X — skipping", vt);
            }
        }
        g_savedOldPlayback = 0;
    }
}
