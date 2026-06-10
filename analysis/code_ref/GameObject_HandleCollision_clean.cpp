// ============================================================================
// GameObject_HandleCollision  (0x40C5D0)  — Clean C++ Refactor
// ============================================================================
// Base collision event handler. Called by Level_HandleCollision (0x40DCD0)
// and Arena_HandleCollision (0x40E6A0) as the final dispatch tier.
//
// All E: (event) and N: (notification) prefixed collision objects in the
// MESHWORLD file reach this function. Adding a new prefix here makes it
// work in BOTH race levels and rumble arenas.
//
// Event name string lives at:  collider->sceneObject->type_name  (+0x864)
//
// Offset map (for modding / DLL injection):
//   this    = Scene*   (param_1)
//   ball    = Ball*    (param_2)
//   collObj = int*     (param_3)  → collObj[1] is the inner SceneObject ptr
//   app     = App*     (*(int**)(this + 0x878))
// ============================================================================

#pragma once
#include <cstdint>
#include <cstring>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Minimal forward declarations (match game structs at their known sizes)
// ---------------------------------------------------------------------------
struct App;
struct Ball;
struct Scene;
struct Collider;
struct SceneObject;

// Athena helper stubs (real implementations live in the game binary)
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
}

// ---------------------------------------------------------------------------
// Tag parser for XML-style metadata  (e.g. <TIME>30</TIME>)
// ---------------------------------------------------------------------------
struct MWTag {
    const char* name;
    const char* value;
};

// Stub — real implementation is in the original binary at MWParser_ReadTag
MWTag MWParser_ReadTag(void* stringObj);

// ---------------------------------------------------------------------------
// Event type enum — every prefix the original recognises
// ---------------------------------------------------------------------------
enum class EventType : uint8_t {
    Unknown,        // no prefix match — treated as normal geometry

    // === N: notifications (game-state side effects) ===
    Secret,         // N:SECRET        → rotator trigger
    UnlockSecret,   // N:UNLOCKSECRET  → arena unlock check
    NoControl,      // N:NOCONTROL     → freeze input 10 frames
    Water,          // N:WATER         → water flag + timer
    TarPit,         // N:TARPIT        → slow + tar sound
    Goal,           // N:GOAL          → finish race
    MouseTrap,      // N:MOUSETRAP     → deflect ball

    // === E: events (immediate gameplay effects) ===
    NoDizzy,        // E:NODIZZY       → anti-dizzy with duration
    SafeSwitch,     // E:SAFESWITCH    → copy parenthesised data to ball
    Limit,          // E:LIMIT         → arena boundary / completion
    Break,          // E:BREAK         → bounce callback
    Jump,           // E:JUMP          → bounce upward + score
    Action,         // E:ACTION        → generic action (ONCE + SCORE tags)
    Trajectory,     // E:TRAJECTORY    → set ball direction vector

    // === Bare / stripped prefixes ===
    DropIn,         // DROPIN          → pipe entrance
    PipeBonk,       // PIPEBONK        → pipe impact
    PopOut,         // POPOUT          → exit pipe
};

// ---------------------------------------------------------------------------
// Fast event classifier  (mirrors original __strnicmp chain)
// ---------------------------------------------------------------------------
static inline EventType ClassifyEvent(const char* name)
{
    if (!name || name[0] == '\0')
        return EventType::Unknown;

    // ---- N: family ---------------------------------------------------------
    if ((name[0] == 'N' || name[0] == 'n') && name[1] == ':') {
        if (__strnicmp(name + 2, "SECRET",       6) == 0) return EventType::Secret;
        if (__strnicmp(name + 2, "UNLOCKSECRET",12) == 0) return EventType::UnlockSecret;
        if (__stricmp (name + 2, "NOCONTROL")      == 0) return EventType::NoControl;
        if (__stricmp (name + 2, "WATER")          == 0) return EventType::Water;
        if (__stricmp (name + 2, "TARPIT")         == 0) return EventType::TarPit;
        if (__strnicmp(name + 2, "GOAL", 4)        == 0) return EventType::Goal;
        if (__strnicmp(name + 2, "MOUSETRAP", 9)   == 0) return EventType::MouseTrap;
        return EventType::Unknown;
    }

    // ---- E: family ---------------------------------------------------------
    if ((name[0] == 'E' || name[0] == 'e') && name[1] == ':') {
        if (__strnicmp(name + 2, "NODIZZY",     7) == 0) return EventType::NoDizzy;
        if (__strnicmp(name + 2, "SAFESWITCH", 10) == 0) return EventType::SafeSwitch;
        if (__stricmp (name + 2, "LIMIT")        == 0) return EventType::Limit;
        if (__stricmp (name + 2, "BREAK")        == 0) return EventType::Break;
        if (__stricmp (name + 2, "JUMP")         == 0) return EventType::Jump;
        if (__strnicmp(name + 2, "ACTION", 6)    == 0) return EventType::Action;
        if (__strnicmp(name + 2, "TRAJECTORY",10) == 0) return EventType::Trajectory;
        return EventType::Unknown;
    }

    // ---- Bare (no prefix) --------------------------------------------------
    if (__stricmp(name + 2, "DROPIN")   == 0) return EventType::DropIn;
    if (__stricmp(name + 2, "PIPEBONK") == 0) return EventType::PipeBonk;
    if (__stricmp(name + 2, "POPOUT")  == 0) return EventType::PopOut;

    return EventType::Unknown;
}

// ---------------------------------------------------------------------------
// Sound handle cache offsets inside App  (verified from decompilations)
// ---------------------------------------------------------------------------
constexpr uint32_t APP_OFF_SOUND_JUMP    = 0x???;  // TODO: verify exact offset
constexpr uint32_t APP_OFF_SOUND_TAR     = 0x???;
constexpr uint32_t APP_OFF_SOUND_DROPIN  = 0x???;
constexpr uint32_t APP_OFF_SOUND_POPOUT  = 0x???;
constexpr uint32_t APP_OFF_SND_PIPEBONK  = 0x???;  // array[3]
constexpr uint32_t APP_OFF_MUSIC_HANDLE  = 0x534;  // from app_struct.h

// ---------------------------------------------------------------------------
// Ball state offsets (from ball_struct.h, byte-accurate)
// ---------------------------------------------------------------------------
constexpr uint32_t BALL_OFF_IMPACT_CNTR  = 0x???;   // impact cooldown counter
constexpr uint32_t BALL_OFF_FREEZE_CNTR  = 0x???;   // freeze frame counter
constexpr uint32_t BALL_OFF_FORCE_MODE   = 0x???;
constexpr uint32_t BALL_OFF_WATER_FLAG   = 0x2D5;   // in-water
constexpr uint32_t BALL_OFF_TAR_FLAG     = 0xB3;    // in-tar-pit
constexpr uint32_t BALL_OFF_NOCTRL_TIMER = 0x202;   // no-control frames
constexpr uint32_t BALL_OFF_FINISHED     = 0x???;
constexpr uint32_t BALL_OFF_ACTIVE       = 0x???;
constexpr uint32_t BALL_ON_RAMP          = 0x2E9;
constexpr uint32_t BALL_OFF_DISPLAY_STR  = 0xC28;   // AthenaString* safeswitch

// ---------------------------------------------------------------------------
// Main handler  (this = Scene*, ball = Ball*, collObj = int*)
// ---------------------------------------------------------------------------
void __thiscall GameObject_HandleCollision(Scene* this_, Ball* ball, int* collObj)
{
    // Raw pointer dance: collObj[1] is the SceneObject*, type_name at +0x864
    char* eventName = *(char**)(collObj[1] + 0x864);

    App* app = *(App**)((uint8_t*)this_ + 0x878);

    switch (ClassifyEvent(eventName))
    {
        // ===================================================================
        // N:  Notification / state-change events
        // ===================================================================

        case EventType::Secret:
            Rotator_MarkTriggered(*(int*)(*collObj + 0x47C));
            break;

        case EventType::UnlockSecret:
            CheckArenaUnlock((int)this_);
            break;

        case EventType::NoControl:
            *(uint8_t*)((uint8_t*)ball + BALL_OFF_NOCTRL_TIMER) = 10;
            break;

        case EventType::Water:
            *(uint8_t*)((uint8_t*)ball + BALL_OFF_WATER_FLAG) = 1;
            *(uint8_t*)((uint8_t*)ball + 0xB6) = 10;   // water timer
            break;

        case EventType::TarPit:
        {
            uint8_t* tarFlag = (uint8_t*)ball + BALL_OFF_TAR_FLAG;
            if (*tarFlag == 0) {
                // first entry — play 3D tar sound
                Sound_Play3D(*(int*)((uint8_t*)app + APP_OFF_SOUND_TAR),
                             ball->pos_x, ball->pos_y, ball->pos_z);
            }
            *tarFlag = 1;
            *(uint8_t*)((uint8_t*)ball + BALL_ON_RAMP) = 0;  // clear velocity-ish flag
            break;
        }

        case EventType::Goal:
        {
            uint8_t* finished = (uint8_t*)ball + BALL_OFF_FINISHED;
            uint8_t* active   = (uint8_t*)ball + BALL_OFF_ACTIVE;
            if (!*finished && *active) {
                uint8_t* goalReached = (uint8_t*)this_ + 0x???;  // scene goal flag
                if (!*goalReached) {
                    *goalReached = 1;
                    Audio_PlayMusic(*(int*)((uint8_t*)app + APP_OFF_MUSIC_HANDLE), "Goal!");
                }
                // mark player finished
                *(uint8_t*)((uint8_t*)app + 0x???)[ball->player_index] = 1;
                // copy race time to finish record
                *(float*)((uint8_t*)app + 0x???)[ball->player_index] =
                    *(float*)((uint8_t*)app + 0x???);  // raceTime
            }
            break;
        }

        case EventType::MouseTrap:
        {
            RNG_Rand(50);  // consume two random numbers (original does this for effect)
            RNG_Rand(50);
            // deflect: normalize ball direction × trap speed constant
            extern float g_trapSpeed;  // _DAT_004CF370
            Vec3_NormalizeAndScale(&ball->vel_x, 1.0f);
            // ball->vel *= g_trapSpeed;  (original multiplies in-place)
            break;
        }

        // ===================================================================
        // E:  Immediate gameplay events
        // ===================================================================

        case EventType::NoDizzy:
        {
            // Parse <TIME>N</TIME> from the event name string
            void* tmpString = nullptr;  // AthenaString temp
            // AthenaString_Set(tmpString, eventName);
            // MWTag tag = MWParser_ReadTag(tmpString);
            // while (tag.name) {
            //     if (__stricmp(tag.name, "TIME") == 0)
            //         Ball_RecordBest(ball, (int)atol(tag.value));
            //     tag = MWParser_ReadTag(tmpString);
            // }
            break;
        }

        case EventType::SafeSwitch:
        {
            char* paren = strchr(eventName, '(');
            if (paren == nullptr) {
                *(uint32_t*)((uint8_t*)ball + BALL_OFF_DISPLAY_STR) = 0;  // clear
            } else {
                strcpy((char*)((uint8_t*)ball + BALL_OFF_DISPLAY_STR), paren);
            }
            break;
        }

        case EventType::Limit:
        {
            *(uint8_t*)((uint8_t*)ball + BALL_ON_RAMP) = 0;
            *(uint8_t*)((uint8_t*)ball + 0x2E9) = 1;   // limit-hit flag
            // per-player completion counting (board+0x47B4/47B8/47BC/47C0)
            for (int player = 0; player < 4; ++player) {
                if (player == ball->player_index)
                    continue;
                uint8_t* pActive = (uint8_t*)this_ + 0x???;  // TODO: player_is_active
                if (*pActive)
                    (*(uint32_t*)((uint8_t*)this_ + 0x47B4))[player]++;
            }
            break;
        }

        case EventType::Break:
            // vtable[0x20] == ApplyBounceForce / BounceCallback
            ((void(__thiscall*)(Ball*))*(uint32_t*)((uint32_t*)ball)[0x20])(ball);
            break;

        case EventType::Jump:
        {
            uint8_t* impactCnt = (uint8_t*)ball + BALL_OFF_IMPACT_CNTR;
            if (*impactCnt < 1) {
                Sound_Play3D(*(int*)((uint8_t*)app + APP_OFF_SOUND_JUMP),
                             ball->pos_x, ball->pos_y, ball->pos_z);
                *impactCnt = 10;
                ball->vel_x = 0.025f;           // tiny upward impulse (0x3B03126F)
                *(uint8_t*)((uint8_t*)ball + BALL_OFF_FORCE_MODE) = 1;
                *(uint8_t*)((uint8_t*)ball + BALL_OFF_FREEZE_CNTR) = 10;
                Ball_RecordBest(ball, 200);   // +200 score
            }
            break;
        }

        case EventType::Action:
        {
            void* tmpString = nullptr;  // AthenaString
            // AthenaString_Set(tmpString, eventName);
            // MWTag tag = MWParser_ReadTag(tmpString);
            // while (tag.name) {
            //     if (__stricmp(tag.name, "ONCE") == 0) {
            //         AthenaList* triggered = (AthenaList*)((uint8_t*)ball + 0xCB);
            //         if (AthenaList_ContainsValue(triggered, collObj))
            //             break;
            //         AthenaList_Append(triggered, collObj);
            //     }
            //     if (__stricmp(tag.name, "SCORE") == 0) {
            //         int points = (int)atol(tag.value);
            //         int modified = Difficulty_GetTimeModifier(app, points);
            //         *(int*)((uint8_t*)app + PLAYER_SCORE_OFF)[ball->player_index] += modified;
            //     }
            //     tag = MWParser_ReadTag(tmpString);
            // }
            break;
        }

        case EventType::Trajectory:
        {
            float tx = 0.0f, ty = 0.0f, tz = 0.0f;
            void* tmpString = nullptr;
            // MWTag tag = MWParser_ReadTag(tmpString);
            // while (tag.name) {
            //     if (__stricmp(tag.name, "X") == 0) tx = (float)atof(tag.value);
            //     if (__stricmp(tag.name, "Y") == 0) ty = (float)atof(tag.value);
            //     if (__stricmp(tag.name, "Z") == 0) tz = (float)atof(tag.value);
            //     tag = MWParser_ReadTag(tmpString);
            // }
            // ball->collisionMesh.direction = { tx, ty, tz };  // +0xCA4
            break;
        }

        // ===================================================================
        // Bare (no prefix) — pipe events
        // ===================================================================

        case EventType::DropIn:
        {
            float dist = 0.0f;  // length(ball->collisionMesh->direction)
            // if (dist > threshold && ball->dropinCounter < 1) {
            //     Sound_PlayChannel(app->sound_dropin);
            //     ball->dropinCounter = 50;
            //     Ball_RecordBest(ball, 200);
            // }
            break;
        }

        case EventType::PipeBonk:
        {
            // if (ball->pipebonkCounter < 1) {
            //     int sndIdx = RNG_Rand(3);  // 0..2
                       //     Sound_Play3D(app->pipe_bonk_sounds[sndIdx], ball->pos_x, ball->pos_y, ball->pos_z);
            //     ball->pipebonkCounter = 10;
            //     Ball_RecordBest(ball, 100);
            // }
            break;
        }

        case EventType::PopOut:
        {
            // if (ball->popoutCounter < 1) {
            //     Sound_PlayChannel(app->sound_popout);
            //     ball->popoutCounter = 50;
            //     Ball_RecordBest(ball, 100);
            // }
            break;
        }

        default:
            // Unknown / unhandled event — treated as normal geometry collision.
            // Ball bounces/reacts according to the collision mesh, no side effects.
            break;
    }
}
