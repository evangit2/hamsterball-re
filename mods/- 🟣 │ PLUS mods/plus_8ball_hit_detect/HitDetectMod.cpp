#include "HamsterballAPI.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>

/* 8-Ball Hit Detector for HB+ v2.1
 *
 * Port of 8ball_hit_detect v6 (bass.dll proxy) to HB+ API.
 *
 * Simplifications vs bass.dll:
 * - Hook into Ball_Update asm code cave → onBallBump callback
 * - No BASS proxy shell
 * - Per-frame state tracked in mod, hitlog.txt written from callback
 *
 * Features preserved:
 * - Detects player -> 8-ball collisions (either ball can be player/8ball)
 * - Writes to hitlog.txt in game directory
 * - Counts each collision once via pointer ordering
 */

class HitDetectMod : public HamsterballAPI {
private:
    IModAPI* api = nullptr;
    FILE* logFile = nullptr;
    volatile DWORD hitCount = 0;

    void logHit(int playerIndex) {
        DWORD count = ++hitCount;
        if (logFile) {
            fprintf(logFile, "[Hit %lu] Player %d struck an 8-ball\r\n",
                    (unsigned long)count, playerIndex + 1);
            fflush(logFile);
        }
    }

public:
    const char* GetModName() override      { return "8-Ball Hit Detector"; }
    const char* GetAuthorName() override   { return "RodentRacer"; }
    const char* GetContributors() override { return "Hamsterbot"; }
    int GetApiVersion() override           { return HAMSTERBALL_API_VERSION; }

    void Initialize(IModAPI* modApi) override {
        api = modApi;

        CustomButton btn("8BALL_HIT_DETECT", "Log 8-Ball Hits");
        btn.defaultState = true;
        modApi->CreateToggleButton(btn, this);

        /* Open hitlog.txt in game directory (same folder as Hamsterball.exe).
           We intentionally do this at Initialize, not in DllMain. */
        char path[MAX_PATH];
        GetModuleFileNameA(GetModuleHandleA(NULL), path, MAX_PATH);
        char* slash = strrchr(path, '\\');
        if (slash) strcpy(slash + 1, "hitlog.txt");
        else strcpy(path, "hitlog.txt");
        logFile = fopen(path, "a");
        if (logFile) {
            fprintf(logFile, "[MOD] 8-Ball Hit Detector (HB+) loaded\r\n");
            fflush(logFile);
        }
    }

    void onButtonToggle(const char* buttonId, bool newState) override {
        /* If button is toggled off, stop logging; when on, resume. */
        if (strcmp(buttonId, "8BALL_HIT_DETECT") == 0) {
            if (!newState) hitCount = 0;
        }
    }

    void onBallBump(Ball* ball1, Ball* ball2) override {
        if (!ball1 || !ball2) return;

        /* Player ID: 0-3 = player, -1 = 8-ball / NPC */
        int id1 = ball1->playerID;
        int id2 = ball2->playerID;

        bool p1 = (id1 >= 0 && id1 <= 3);
        bool p2 = (id2 >= 0 && id2 <= 3);
        bool npc1 = (id1 == -1);
        bool npc2 = (id2 == -1);

        /* Count only player -> 8-ball collisions */
        if ((p1 && npc2) || (p2 && npc1)) {
            int playerIndex = p1 ? id1 : id2;
            logHit(playerIndex);
        }
    }

    void onSceneEnd() override {
        if (logFile) {
            fflush(logFile);
        }
    }

    ~HitDetectMod() {
        if (logFile) {
            fprintf(logFile, "[MOD] Shutting down\r\n");
            fclose(logFile);
            logFile = nullptr;
        }
    }
};

extern "C" __declspec(dllexport) HamsterballAPI* CreateModInstance() {
    return new HitDetectMod();
}
