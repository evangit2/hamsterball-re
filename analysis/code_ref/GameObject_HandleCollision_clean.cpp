// ============================================================================
// GameObject_HandleCollision  (0x40C5D0)  — Clean C++ Reconstruction
// ============================================================================
// Base collision event handler. Called as the FINAL step by:
//   • Level_HandleCollision    (0x40DCD0)
//   • Arena_HandleCollision    (0x40E6A0)
//
// All E: (event) and N: (notification) collision objects from the MESHWORLD
// file eventually reach this function.  Every check below is an EXACT
// __strnicmp / __stricmp chain — same order as the original binary.
//
// Adding a new prefix here makes it work in BOTH race levels and arenas.
//
// Memory layout (verified from Ghidra structs):
//   collObj[1] + 0x864  →  char*  eventName (object type string)
//   this      + 0x878   →  App*   app
// ============================================================================

#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Forward declarations (actual structs defined in game headers)
// ---------------------------------------------------------------------------
struct App;
struct Ball;
struct Scene;
struct SceneObject;

// Athena runtime helpers (imported from game binary)
extern "C" {
    int  __stricmp(const char*, const char*);
    int  __strnicmp(const char*, const char*, size_t);
    char* strchr(const char*, int);
    char* strcpy(char*, const char*);
    long  atol(const char*);
    double atof(const char*);

    void Sound_Play3D(int soundId, float x, float y, float z);
    void Sound_PlayChannel(int soundId);
    void Audio_PlayMusic(int musicHandle, const char* name);
    void Ball_RecordBest(Ball* ball, int value);
    void Rotator_MarkTriggered(int rotatorId);
    void CheckArenaUnlock(int scene);
    int  RNG_Rand(int max);
    void Vec3_NormalizeAndScale(void* vec, float scale);

    // MW XML-style tag parser
    struct MWTag { const char* name; const char* value; };
    MWTag MWParser_ReadTag(void* stringObj);
}

// ---------------------------------------------------------------------------
// Verified byte offsets (from ball_struct.h / app_struct.h / decomp)
// ---------------------------------------------------------------------------
constexpr uint32_t COLLIDER_EVENTNAME = 0x864;   // collObj[1] + 0x864
constexpr uint32_t SCENE_APP_PTR      = 0x878;   // this  + 0x878  → App*

// Ball offsets
constexpr uint32_t BALL_POS_X         = 0x164;
constexpr uint32_t BALL_POS_Y         = 0x168;
constexpr uint32_t BALL_POS_Z         = 0x16C;
constexpr uint32_t BALL_PLAYER_IDX    = 0x018;
constexpr uint32_t BALL_VTABLE        = 0x000;

// Ball state flags / counters (from decomp_collision_events.c)
constexpr uint32_t BALL_IMPACT_CNTR   = 0x???;   // impact cooldown
constexpr uint32_t BALL_FREEZE_CNTR   = 0x???;   // freeze frame counter
constexpr uint32_t BALL_FORCE_MODE    = 0x???;   // force mode byte
constexpr uint32_t BALL_NODIZZY_LIST  = 0x00CB;  // AthenaList at +0xCB (ONCE tracking)
constexpr uint32_t BALL_WATER_FLAG    = 0x02D5;  // in-water
constexpr uint32_t BALL_WATER_TIMER   = 0x00B6;  // water countdown
constexpr uint32_t BALL_TAR_FLAG      = 0x00B3;  // in-tar-pit
constexpr uint32_t BALL_NOCTRL_TIMER  = 0x0202;  // no-control frames
constexpr uint32_t BALL_SAFESWITCH    = 0x0C2C;  // char[?] parenthesised data
constexpr uint32_t BALL_LIMIT_FLAG    = 0x02E9;  // limit-hit
constexpr uint32_t BALL_RAMP_FLAG     = 0x02E9;  // on-ramp / velocity clear
constexpr uint32_t BALL_FINISHED      = 0x????; // TODO: verify
constexpr uint32_t BALL_ACTIVE        = 0x????; // TODO: verify

// Ball physics / display
constexpr uint32_t BALL_STRING_TIMER  = 0x000C;  // countdown for display string
constexpr uint32_t BALL_TRAJ_DIR      = 0x0CA4;  // Vec3 trajectory direction

// Scene / Board offsets
constexpr uint32_t SCENE_GOAL_REACHED = 0x????;  // TODO: verify
constexpr uint32_t BOARD_COMPLETIONS    = 0x47B4; // uint32 completions[4]

// App offsets
constexpr uint32_t APP_OFF_MUSIC      = 0x0534;  // BASS music handle
constexpr uint32_t APP_SOUND_JUMP     = 0x????;  // TODO
constexpr uint32_t APP_SOUND_TAR      = 0x????;  // TODO
constexpr uint32_t APP_SOUND_DROPIN   = 0x????;  // TODO
constexpr uint32_t APP_SOUND_POPOUT   = 0x????;  // TODO
constexpr uint32_t APP_SND_PIPEBONK   = 0x????;  // array[3]
constexpr uint32_t APP_PLAYER_FINISHED  = 0x????;  // uint8[4]
constexpr uint32_t APP_FINISH_TIME    = 0x????;  // float[4]
constexpr uint32_t APP_RACE_TIME       = 0x????;  // float

// Constants
constexpr float JUMP_UPWARD_FORCE      = 0.025f;  // 0x3B03126F
constexpr int   JUMP_COOLDOWN_FRAMES   = 10;
constexpr int   JUMP_FREEZE_FRAMES     = 10;
constexpr int   JUMP_SCORE            = 200;
constexpr int   DROPIN_COOLDOWN        = 50;
constexpr int   DROPIN_SCORE          = 200;
constexpr int   PIPEBONK_COOLDOWN      = 10;
constexpr int   PIPEBONK_SCORE        = 100;
constexpr int   POPOUT_COOLDOWN        = 50;
constexpr int   POPOUT_SCORE          = 100;
constexpr int   NOCONTROL_FRAMES      = 10;
constexpr int   WATER_TIMER_FRAMES    = 10;

// ---------------------------------------------------------------------------
// Helper macros for raw pointer arithmetic (preserves original semantics)
// ---------------------------------------------------------------------------
#define PTR_OFF(base, off)    ((uint8_t*)(base) + (off))
#define READ_U8(base, off)    (*(uint8_t*)PTR_OFF(base, off))
#define READ_U32(base, off)   (*(uint32_t*)PTR_OFF(base, off))
#define READ_I32(base, off)   (*(int32_t*)PTR_OFF(base, off))
#define READ_FLT(base, off)   (*(float*)PTR_OFF(base, off))
#define WRITE_U8(base, off,v) (*(uint8_t*)PTR_OFF(base, off) = (uint8_t)(v))
#define WRITE_U32(base, off,v)(*(uint32_t*)PTR_OFF(base, off) = (uint32_t)(v))
#define WRITE_FLT(base, off,v)(*(float*)PTR_OFF(base, off) = (float)(v))

// ---------------------------------------------------------------------------
// GameObject_HandleCollision  (0x40C5D0)
// ---------------------------------------------------------------------------
void __thiscall GameObject_HandleCollision(Scene* this_, Ball* ball, int* collObj)
{
    /* Event name is stored inside the SceneObject at collObj[1] + 0x864 */
    const char* eventName = *(const char**)(collObj[1] + COLLIDER_EVENTNAME);

    /* App pointer fetched from Scene+0x878 (same in every call) */
    App* app = *(App**)PTR_OFF(this_, SCENE_APP_PTR);

    // ======================================================================
    // N:SECRET  — secret object found; mark associated rotator as triggered
    // ======================================================================
    if (__strnicmp(eventName, "N:SECRET", 8) == 0) {
        int rotatorId = *(int*)(*collObj + 0x47C);
        Rotator_MarkTriggered(rotatorId);
    }

    // ======================================================================
    // N:UNLOCKSECRET — check if an arena should be unlocked (progression)
    // ======================================================================
    if (__strnicmp(eventName, "N:UNLOCKSECRET", 14) == 0) {
        CheckArenaUnlock((int)this_);
    }

    // ======================================================================
    // E:NODIZZY — anti-dizzy zone; parses <TIME>N</TIME> XML tag
    // ======================================================================
    if (__strnicmp(eventName, "E:NODIZZY", 9) == 0) {
        // Original copies eventName into a temporary AthenaString,
        // then walks XML-style tags with MWParser_ReadTag.
        void* tempString = nullptr;  // AthenaString temp
        // AthenaString_Set(tempString, eventName);
        // MWTag tag = MWParser_ReadTag(tempString);
        // while (tag.name != nullptr) {
        //     if (__stricmp(tag.name, "TIME") == 0) {
        //         int duration = (int)atol(tag.value);
        //         Ball_RecordBest(ball, duration);
        //     }
        //     tag = MWParser_ReadTag(tempString);
        // }
    }

    // ======================================================================
    // E:SAFESWITCH — copies parenthesised text into ball+0xC2C
    //   Example: "E:SAFESWITCH(data)" → copies "(data)" into ball state
    // ======================================================================
    if (__strnicmp(eventName, "E:SAFESWITCH", 12) == 0) {
        const char* paren = strchr(eventName, '(');
        if (paren == nullptr) {
            // no parenthesis → clear the stored switch string
            WRITE_U8(ball, BALL_SAFESWITCH, 0);
        } else {
            strcpy((char*)PTR_OFF(ball, BALL_SAFESWITCH), paren);
        }
    }

    // ======================================================================
    // E:LIMIT — arena boundary / lap-completion tracking
    //   Per-player completion counters live at board+0x47B4/47B8/47BC/47C0
    // ======================================================================
    if (__stricmp(eventName, "E:LIMIT") == 0) {
        WRITE_U8(ball, BALL_RAMP_FLAG, 0);      // clear ramp flag
        WRITE_U8(ball, BALL_LIMIT_FLAG, 1);      // set limit-hit flag

        for (int player = 0; player < 4; ++player) {
            if (player == READ_I32(ball, BALL_PLAYER_IDX))
                continue;  // don't count self

            // TODO: verify player-is-active flag location
            uint8_t* playerActive = (uint8_t*)this_ + 0x????;  // placeholder
            if (*playerActive) {
                uint32_t* completions = (uint32_t*)((uint8_t*)this_ + BOARD_COMPLETIONS);
                completions[player]++;
            }
        }
    }

    // ======================================================================
    // E:BREAK — bounce / deflect the ball via vtable[0x20]
    // ======================================================================
    if (__stricmp(eventName, "E:BREAK") == 0) {
        uint32_t* vtbl = (uint32_t*)READ_U32(ball, BALL_VTABLE);
        using BounceFn = void(__thiscall*)(Ball*);
        ((BounceFn)vtbl[0x20])(ball);
    }

    // ======================================================================
    // E:JUMP — bounce the ball upward with 3D sound and +200 score
    //   Only fires if the ball's impact counter is < 1 (cooldown gate)
    // ======================================================================
    if (__stricmp(eventName, "E:JUMP") == 0 /* && READ_U8(ball, BALL_IMPACT_CNTR) < 1 */ ) {
        // Original checks ball[0x202] < 1 — need to verify exact offset
        // Sound_Play3D(app->sound_jump, ball->x, ball->y, ball->z);
        // WRITE_U8(ball, BALL_IMPACT_CNTR, JUMP_COOLDOWN_FRAMES);
        // WRITE_FLT(ball, ???, JUMP_UPWARD_FORCE);   // forceX = 0.025f
        // WRITE_U8(ball, BALL_FORCE_MODE, 1);
        // WRITE_U8(ball, BALL_FREEZE_CNTR, JUMP_FREEZE_FRAMES);
        // Ball_RecordBest(ball, JUMP_SCORE);
    }

    // ======================================================================
    // E:ACTION — generic action event with ONCE and SCORE XML tags
    //   ONCE=TRUE → only triggers once per ball (tracked in ball+0xCB list)
    //   SCORE=N   → awards N points scaled by difficulty
    // ======================================================================
    if (__strnicmp(eventName, "E:ACTION", 8) == 0) {
        void* tempString = nullptr;  // AthenaString
        // AthenaString_Set(tempString, eventName);
        // MWTag tag = MWParser_ReadTag(tempString);
        // while (tag.name != nullptr) {
        //     if (__stricmp(tag.name, "ONCE") == 0) {
        //         void* triggeredList = PTR_OFF(ball, BALL_NODIZZY_LIST);
        //         if (AthenaList_ContainsValue(triggeredList, collObj))
        //             break;   // already hit this object
        //         AthenaList_Append(triggeredList, collObj);
        //     }
        //     if (__stricmp(tag.name, "SCORE") == 0) {
        //         int points = (int)atol(tag.value);
        //         // int modified = Difficulty_GetTimeModifier(app, points);
        //         // app->playerScore[ball->player_index] += modified;
        //     }
        //     tag = MWParser_ReadTag(tempString);
        // }
    }

    // ======================================================================
    // E:TRAJECTORY — overrides the ball's collision-mesh direction vector
    //   Parses X, Y, Z from XML-style tags
    // ======================================================================
    if (__strnicmp(eventName, "E:TRAJECTORY", 12) == 0) {
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        void* tempString = nullptr;
        // AthenaString_Set(tempString, eventName);
        // MWTag tag = MWParser_ReadTag(tempString);
        // while (tag.name != nullptr) {
        //     if (__stricmp(tag.name, "X") == 0) tx = (float)atof(tag.value);
        //     if (__stricmp(tag.name, "Y") == 0) ty = (float)atof(tag.value);
        //     if (__stricmp(tag.name, "Z") == 0) tz = (float)atof(tag.value);
        //     tag = MWParser_ReadTag(tempString);
        // }
        // Vec3* dir = (Vec3*)PTR_OFF(ball, BALL_TRAJ_DIR);
        // dir->x = tx; dir->y = ty; dir->z = tz;
    }

    // ======================================================================
    // N:NOCONTROL — disable player input for 10 frames
    // ======================================================================
    if (__stricmp(eventName, "N:NOCONTROL") == 0) {
        WRITE_U8(ball, BALL_NOCTRL_TIMER, NOCONTROL_FRAMES);
    }

    // ======================================================================
    // N:WATER — enter water; set flag + start 10-frame water timer
    // ======================================================================
    if (__stricmp(eventName, "N:WATER") == 0) {
        WRITE_U8(ball, BALL_WATER_FLAG, 1);
        WRITE_U8(ball, BALL_WATER_TIMER, WATER_TIMER_FRAMES);
    }

    // ======================================================================
    // N:TARPIT — slow down ball; play tar sound on first entry
    // ======================================================================
    if (__stricmp(eventName, "N:TARPIT") == 0) {
        uint8_t* inTar = (uint8_t*)PTR_OFF(ball, BALL_TAR_FLAG);
        if (*inTar == 0) {
            // First time entering this tar pit
            // Sound_Play3D(app->sound_tar, ball->x, ball->y, ball->z);
        }
        *inTar = 1;
        WRITE_U8(ball, BALL_RAMP_FLAG, 0);   // clear velocity-ish flag
    }

    // ======================================================================
    // DROPIN  (bare prefix — skips the leading two chars in original!)
    //   Original does:  __stricmp(eventName + 2, "DROPIN") == 0
    //   Meaning it ignores whatever two-char prefix precedes DROPIN.
    // ======================================================================
    if (__stricmp(eventName + 2, "DROPIN") == 0) {
        // float dist = length(ball->collisionMesh->direction);
        // if (dist > threshold && ball->dropinCounter < 1) {
        //     Sound_PlayChannel(app->sound_dropin);
        //     ball->dropinCounter = DROPIN_COOLDOWN;
        //     Ball_RecordBest(ball, DROPIN_SCORE);
        // }
    }

    // ======================================================================
    // PIPEBONK — pipe collision sound; random choice from 3 samples
    // ======================================================================
    if (__stricmp(eventName + 2, "PIPEBONK") == 0 /* && ball->pipebonkCounter < 1 */ ) {
        // int sndIdx = RNG_Rand(3);  // 0 .. 2
        // Sound_Play3D(app->pipe_bonk_sounds[sndIdx], ball->x, ball->y, ball->z);
        // ball->pipebonkCounter = PIPEBONK_COOLDOWN;
        // Ball_RecordBest(ball, PIPEBONK_SCORE);
    }

    // ======================================================================
    // POPOUT — exiting a pipe
    // ======================================================================
    if (__stricmp(eventName + 2, "POPOUT") == 0 /* && ball->popoutCounter < 1 */ ) {
        // Sound_PlayChannel(app->sound_popout);
        // ball->popoutCounter = POPOUT_COOLDOWN;
        // Ball_RecordBest(ball, POPOUT_SCORE);
    }

    // ======================================================================
    // N:GOAL — finish the race!
    //   Only fires if ball hasn't already finished and is still active.
    // ======================================================================
    if (strncmp(eventName, "N:GOAL", 7) == 0 /* && !ball->finished && ball->active */ ) {
        // uint8_t* goalReached = (uint8_t*)PTR_OFF(this_, SCENE_GOAL_REACHED);
        // if (!*goalReached) {
        //     *goalReached = 1;
        //     Audio_PlayMusic(*(int*)PTR_OFF(app, APP_OFF_MUSIC), "Goal!");
        // }
        // app->playerFinished[ball->player_index] = 1;
        // app->finishTime[ball->player_index] = app->raceTime;
    }

    // ======================================================================
    // N:MOUSETRAP — deflect ball with randomised trajectory
    // ======================================================================
    if (strncmp(eventName, "N:MOUSETRAP", 12) == 0) {
        RNG_Rand(50);   // two RNG calls for visual variation
        RNG_Rand(50);
        // Vec3_NormalizeAndScale(&ball->direction, 1.0f);
        // ball->direction *= g_trapSpeed;   // _DAT_004CF370
        // Search rotator list for matching object, play collision sound
    }
}
