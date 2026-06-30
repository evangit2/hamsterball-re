// CreateBadBall (0x40BCA0) — Clean C++ reconstruction
// Scans MESHWORLD section 3 objects for names starting with "BADBALL"
// and spawns AI 8-ball opponents with configurable behavior tags.

#include <cstdlib>
#include <cstring>

// Forward declarations — addresses from Ghidra
// Ball_ctor            0x4087A0
// AthenaList_NextIndex  runtime
// AthenaList_Append     runtime
// AthenaString_Set      runtime
// MWParser_ReadTag       runtime
// StreamReader_dtor      runtime

struct Ball;          // 0xC98 bytes for 8-ball
struct Scene;        // active scene
struct MeshWorld;    // level data
struct AthenaString;

// MESHWORLD section 3 object entry (object array element)
// *puVar7[0] = name string pointer
// puVar7[1,2,3] = position x,y,z (as float bits in int slots)
// puVar7[5] = rotation/offset used for Y adjustment
struct MWObject {
    char*  name;       // +0x00  object name string (e.g. "BADBALL<CHASE>25</CHASE>...")
    float  x;         // +0x04  position X
    float  y;         // +0x08  position Y
    float  z;         // +0x0C  position Z
    int    _pad10;    // +0x10
    float  rot_y;     // +0x14  rotation Y (used for spawn offset)
};

// MWParser tag result — returned by MWParser_ReadTag
// puVar7[0] = vtable/release functor
// puVar7[1] = tag name string  (e.g. "CHASE", "HOME", "SIZE")
// puVar7[2] = tag value string (e.g. "25.0")
struct MWTag {
    void*  release;   // +0x00  vtable with release()(1) to free
    char*  tag_name;  // +0x04  tag key
    char*  tag_value; // +0x08  tag value as string
};

// Ball struct offsets (verified from decompilation)
namespace BallOffsets {
    constexpr int DISPLAY_POS_X   = 0x164;  // piVar5[0x59] — current display position
    constexpr int DISPLAY_POS_Y   = 0x168;  // piVar5[0x5a]
    constexpr int DISPLAY_POS_Z   = 0x16C;  // piVar5[0x5b]
    constexpr int HOME_POS_X      = 0xC60;  // piVar5[0x318] — spawn/return position
    constexpr int HOME_POS_Y      = 0xC64;  // piVar5[0x319]
    constexpr int HOME_POS_Z      = 0xC68;  // piVar5[0x31a]
    constexpr int CHASE_DISTANCE  = 0xC6C;  // piVar5[0x31b] — AI chase activation range
    constexpr int HOME_RADIUS     = 0xC70;  // piVar5[0x31c] — AI patrol radius
    constexpr int SPIN_DISTANCE   = 0xC7C;  // piVar5[799]  — AI spin orbit distance
    constexpr int BASE_RADIUS     = 0x188;  // piVar5[0x62] — ball collision radius (3.0f for giant)
    constexpr int RADIUS_VALUE    = 0x284;  // piVar5[0xa1] — SIZE tag parsed value
    constexpr int RADIUS_OVERRIDE  = 0x27C;  // piVar5[0x9f]=0 flag when SIZE tag set
    constexpr int SIZE_FLAG        = 0xC4C;  // *(byte*)(piVar5+0x313)=1 when SIZE tag present
    constexpr int UNUSED_INIT_FLAG = 0x281;  // DEAD: set to 1 by ctor, never read by any function
    constexpr int VTABLE          = 0x000;   // *piVar5 = vtable pointer
}

// Scene struct offsets
namespace SceneOffsets {
    constexpr int MESHWORLD_PTR    = 0x8AC;  // -> MeshWorld containing section 3 objects
    constexpr int BAD_BALLS_LIST  = 0x29D4;  // AthenaList of AI 8-balls
    constexpr int ALL_BALLS_LIST  = 0x2DEC;  // AthenaList of all balls (player + AI)
}

// Ball allocation sizes
constexpr int BAD_BALL_ALLOC_SIZE = 0xC98;  // 8-ball allocation (larger than player 0xC60)
constexpr float BALL_RADIUS       = 3.0f;   // _DAT_004cf55c — standard ball radius

// External imports from the original binary
extern Ball*  Ball_ctor(void* mem, Scene* scene);
extern void   AthenaList_Append(void* list, int item);
extern int    AthenaList_NextIndex(void* list);
extern void   AthenaString_Set(void* str, const char* text);
extern MWTag* MWParser_ReadTag(int str_handle);
extern void   StreamReader_dtor(void* str);
extern void*  operator_new(size_t size);
extern int    _strnicmp(const char* a, const char* b, size_t n);
extern int    _stricmp(const char* a, const char* b);
extern long   atol(const char* str);


// =============================================================================
// CreateBadBall — Scene* param_1
// Scans MESHWORLD section 3 for objects named "BADBALL<tag>value</tag>..."
// and spawns an AI 8-ball for each match.
// =============================================================================
void CreateBadBall(Scene* scene)
{
    using namespace BallOffsets;
    using namespace SceneOffsets;

    // Get the MESHWORLD section 3 object iterator
    MeshWorld* mw = *(MeshWorld**)((int)scene + MESHWORLD_PTR);
    void* obj_list_root = *(void**)((int)mw + 0x480);  // section 3 root
    int* obj_array     = *(int**)((int)obj_list_root + 0xCA0);   // object pointer array
    int  obj_count     = *(int*)((int)obj_list_root + 0x898);   // total objects
    int* iter_indices  = (int*)((int)obj_list_root + 0x89C);    // per-thread iteration slots

    // Claim an iteration slot for this call
    int slot = AthenaList_NextIndex((void*)((int)obj_list_root + 0x894));
    iter_indices[slot] = 0;

    // Start iterating — bail if no objects
    if (obj_count < 1)
        return;
    MWObject** obj_ptr = (MWObject**)(obj_array[0]);  // first object pointer
    MWObject* obj = *obj_ptr;
    iter_indices[slot] = 1;

    while (true)
    {
        if (obj == nullptr)
            return;

        // Check if object name starts with "BADBALL" (case-insensitive, 7 chars)
        if (_strnicmp(obj->name, "BADBALL", 7) == 0)
        {
            // ---- Allocate and construct the 8-ball ----
            void* mem = operator_new(BAD_BALL_ALLOC_SIZE);
            Ball* ball = nullptr;
            if (mem != nullptr)
                ball = Ball_ctor(mem, scene);

            // Call 2nd virtual function (Ball_Init or similar setup)
            auto vtable = *(void***)ball;
            auto init_fn = (void(__thiscall*)(Ball*))vtable[1];
            init_fn(ball);

            // ---- Set display position (offset by ball radius) ----
            *(float*)((int)ball + DISPLAY_POS_X) = obj->x + BALL_RADIUS;
            *(float*)((int)ball + DISPLAY_POS_Y) = obj->y + *(float*)((int)ball + RADIUS_VALUE);
            *(float*)((int)ball + DISPLAY_POS_Z) = obj->z + BALL_RADIUS;

            // Clear an unknown flag
            *(byte*)((int)ball + SOME_FLAG) = 0;

            // ---- Set home/spawn position (same as display, without radius offset) ----
            *(float*)((int)ball + HOME_POS_X) = obj->x;
            *(float*)((int)ball + HOME_POS_Y) = obj->y;
            *(float*)((int)ball + HOME_POS_Z) = obj->z;

            // ---- Parse XML-style tags from the object name string ----
            // Tags are embedded in the name like: BADBALL<CHASE>25</CHASE><HOME>5</HOME>
            AthenaString tag_str;  // stack temporary for string handling
            Sprite_DrawColoredRect(&tag_str);    // initialize the string object
            AthenaString_Set(&tag_str, obj->name);

            MWTag* tag = MWParser_ReadTag((int)&tag_str);
            while (tag != nullptr)
            {
                // <CHASE>val</CHASE> — AI chase activation distance
                if (_stricmp(tag->tag_name, "CHASE") == 0)
                {
                    long val = atol(tag->tag_value);
                    *(float*)((int)ball + CHASE_DISTANCE) = (float)val;
                }
                // <HOME>val</HOME> — AI patrol/home radius
                if (_stricmp(tag->tag_name, "HOME") == 0)
                {
                    long val = atol(tag->tag_value);
                    *(float*)((int)ball + HOME_RADIUS) = (float)val;
                }
                // <SIZE>val</SIZE> — ball size multiplier (3.0 = giant 8-ball)
                if (_stricmp(tag->tag_name, "SIZE") == 0)
                {
                    long val = atol(tag->tag_value);
                    *(float*)((int)ball + BASE_RADIUS) = 3.0f;    // set collision radius to giant
                    *(float*)((int)ball + RADIUS_VALUE) = (float)val;  // store parsed size
                    *(int*)((int)ball + RADIUS_OVERRIDE) = 0;    // clear override flag
                    *(byte*)((int)ball + SIZE_FLAG) = 1;          // mark SIZE was set
                }
                // <SPINDISTANCE>val</SPINDISTANCE> — AI spin orbit distance
                if (_stricmp(tag->tag_name, "SPINDISTANCE") == 0)
                {
                    long val = atol(tag->tag_value);
                    *(float*)((int)ball + SPIN_DISTANCE) = (float)val;
                }

                // Release the tag and read the next one
                auto release_fn = (void(__cdecl*)(int))tag->release;
                release_fn(1);
                tag = MWParser_ReadTag((int)&tag_str);
            }

            // ---- Register the 8-ball in scene lists ----
            AthenaList_Append((void*)((int)scene + BAD_BALLS_LIST), (int)ball);
            AthenaList_Append((void*)((int)scene + ALL_BALLS_LIST), (int)ball);

            // Clean up the temporary string
            StreamReader_dtor(&tag_str);
        }

        // Advance to next object in the array
        int idx = iter_indices[slot];
        if (obj_count <= idx)
            break;
        obj = ((MWObject**)obj_array)[idx];
        iter_indices[slot] = idx + 1;
    }
}