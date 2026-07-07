/* GhostSaver.h — Constants, globals, and declarations
 * for the HB+ API version of the ghost saver mod.
 *
 * Converted from ghost_saver.c v25.4 (bass.dll proxy) to HB+ API.
 * Key simplification: no background thread, no CRITICAL_SECTION,
 * no inline asm — everything runs on the main thread via callbacks.
 */
#pragma once
#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Game memory constants ── */

#define GLOBAL_APP_PTR     0x005341E0
#define BTT_SIZE           0x528
#define BTT_BEST_TIME      0x524
#define BTT_NAME           0x424
#define SNAP_SIZE          0x28
#define NO_TIME            9999999
#define SNAP_DWORDS        10
#define SNAP_BYTES         40
#define BTT_VTABLE         0x004D262C

#define APP_90C_RECORDING  0x90C
#define APP_910_PLAYBACK    0x910
#define APP_5D6_GOAL_FLAG   0x5D6
#define APP_234_PARTY_MODE  0x234
#define APP_220_PROFILE     0x220

/* RVA offsets for Call<>/CallMethod<> templates.
 * These are absolute address - 0x400000 (game base). */
#define RVA_BTT_CTOR       0x27660
#define RVA_BTT_DTOR       0x278C0
#define RVA_ALIST_APPEND   0x53780
#define RVA_OPERATOR_NEW   0xBA57B
#define RVA_GAME_FREE      0xBA74D

/* Absolute addresses for RegisterCustomHook */
#define ADDR_APP_START_PRACTICE  0x00428C50
#define RACE_NAME_TABLE          0x004F7080

/* Ghost binary file format:
 *   [4] magic = 0x47485347 ("GHSG")
 *   [4] version = 1
 *   [4] time (game ticks, lower = better)
 *   [4] frame_count
 *   [frame_count * 40] snapshots (10 DWORDs each) */
#define GHOST_MAGIC    0x47485347
#define GHOST_VERSION  1

/* ── Global state ── */

static IModAPI* g_api = nullptr;
static char g_currentRaceName[128] = "";
static char g_hookRaceName[128] = "";
static int g_recording = 0;
static int g_raceFinished = 0;
static int g_prevGoalFlag = 0;
static DWORD g_prevRecording = 0;
static DWORD g_savedOldPlayback = 0;
static DWORD g_dummyRecording = 0;
static char g_ghostDir[MAX_PATH] = "";
static bool g_enabled = true;

/* Dynamic snapshot buffer */
static DWORD (*g_rawSnaps)[SNAP_DWORDS] = NULL;
static int g_rawCount = 0;
static int g_rawCapacity = 0;

/* Hook typedef */
typedef void (__fastcall *AppStartPracticeRace_t)(void* app, void* edx, DWORD race_index);
static AppStartPracticeRace_t orig_AppStartPracticeRace = nullptr;

/* ── Logging (disabled by default) ── */
#define LOGGING_ENABLED 0

static void log_msg(const char *msg) {
#if LOGGING_ENABLED
    if (!g_ghostDir[0]) return;
    char logPath[MAX_PATH];
    snprintf(logPath, sizeof(logPath), "%s..\\ghost_saver_log.txt", g_ghostDir);
    HANDLE h = CreateFileA(logPath, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD w;
        SetFilePointer(h, 0, NULL, FILE_END);
        WriteFile(h, msg, (DWORD)strlen(msg), &w, NULL);
        WriteFile(h, "\r\n", 2, &w, NULL);
        CloseHandle(h);
    }
#endif
}

static void log_fmt(const char *fmt, ...) {
#if LOGGING_ENABLED
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    log_msg(buf);
#endif
}

/* ── Function declarations ── */

static DWORD get_app(void);
static int is_time_trial_active(void);
static int is_time_trial_precheck(void);
static int get_race_name(char *out, int outLen);
static int get_race_name_table_count(void);
static int get_race_name_by_index(DWORD race_index, char *out, int outLen);
static void race_name_to_filename(const char *raceName, char *out, int outLen);
static int get_saved_time(const char *raceName);
static void save_ghost_for_race(const char *raceName, int time,
                                DWORD (*snaps)[10], int count);
static void inject_saved_ghost(const char *raceName);
static void check_race_state(void);
static void snaps_reserve(int needed);
static void snaps_reset(void);
static void init_ghost_dir(void);
static void cleanup_dummy_btt(DWORD app);
