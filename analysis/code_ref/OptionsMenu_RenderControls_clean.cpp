// OptionsMenu_RenderControls — clean C++ reference
// ============================================================================
// Original: OptionsMenu_RenderControls @ 0x42E840
// Decompiled from Hamsterball.exe (Athena engine, VS2003)
//
// This file is a structurally faithful rewrite of the decompiled function.
// All magic numbers are preserved exactly as in the original binary.
//
// Related functions / data referenced:
//   - OptionsMenu_RenderControls ............ 0x42E840 (this function)
//   - Matrix_Scale4x4 ....................... 0x453150
//   - Matrix_Identity ....................... 0x4530E0 (inferred from nearby)
//   - UIList_SetColorsByName ................ 0x455D60 (inferred, draw call)
//   - AthenaString_SprintfToBuffer .......... 0x46A310
//   - App+0x878 ............................. Scene* (current scene pointer)
//   - App+0xB28 .. 0xB38 .................... CONTROL1..4 registry-backed keys
//
// Control type constants:
//   99  = KeyboardDevice (DIK code stored in lower byte)
//   100 = Mouse (axis/button index)
//
// STRUCTURAL NOTES FROM GHIDRA:
//   - __fastcall calling convention (ECX = this)
//   - SEH prologue/epilogue is omitted in this clean version
//   - The original uses raw int offsets (0xB28..0xB38) for the 4 control slots
//   - Matrix_Scale4x4 writes to a 20-byte struct: {vtable, x, y, z, w}
//   - Colors are passed as 4 floats to UIList_SetColorsByName
// ============================================================================

#include <cstdint>

// ---------------------------------------------------------------------------
// Forward declarations with original addresses
// ---------------------------------------------------------------------------
struct App;          // App object; param_1 in the original is App* cast to void*
struct Scene;        // Scene object at App+0x878
struct Matrix4f;     // 20-byte scale matrix: {vtable_ptr, x, y, z, w}

// 0x453150 — Matrix_Scale4x4(void* this, float x, float y, float z, float w)
// Writes the 4 scale components into a 20-byte structure.
extern Matrix4f* __thiscall Matrix_Scale4x4(Matrix4f* out, float x, float y, float z, float w);

// 0x4530E0 — Matrix_Identity(void* this)
// Zeroes/identity-fills a 20-byte matrix structure.
extern void __thiscall Matrix_Identity(Matrix4f* out);

// 0x46A310 — AthenaString_SprintfToBuffer(char* buf, const char* fmt, ...)
extern void AthenaString_SprintfToBuffer(char* buf, const char* fmt, ...);

// 0x455D60 — UIList_SetColorsByName(void* param_1, void* color_vtable,
//                                    float r, float g, float b, float a,
//                                    const char* name)
// Draws a UI element with the given color and name label.
extern void __thiscall UIList_SetColorsByName(
    void*        param_1,
    void*        color_vtable,   // &PTR_Vec3_dtor_004cf300
    float        r,
    float        g,
    float        b,
    float        a,
    const char*  name
);

// ---------------------------------------------------------------------------
// Constants (preserved from original binary)
// ---------------------------------------------------------------------------
static constexpr float FLOAT_1_0     = 0x3f800000; // 1.0f
static constexpr float FLOAT_0_5     = 0x3f000000; // 0.5f
static constexpr float FLOAT_0_0     = 0.0f;

static constexpr int   CONTROL_1     = 0x0B28; // App+0xB28 = CONTROL1
static constexpr int   CONTROL_2     = 0x0B2C; // App+0xB2C = CONTROL2
static constexpr int   CONTROL_3     = 0x0B30; // App+0xB30 = CONTROL3
static constexpr int   CONTROL_4     = 0x0B34; // App+0xB34 = CONTROL4
static constexpr int   CONTROL_END   = 0x0B38; // loop boundary

static constexpr int   TYPE_KEYBOARD = 99;   // KeyboardDevice binding
static constexpr int   TYPE_MOUSE    = 100;  // Mouse binding

// ---------------------------------------------------------------------------
// OptionsMenu_RenderControls  @ 0x42E840
// ---------------------------------------------------------------------------
// Iterates the 4 control bindings stored at App+0xB28..0xB34.
// For each binding:
//   - Keyboard (99)  → color = half-width white  (0.5, 1.0, 0.5, 1.0)
//   - Mouse    (100) → color = half-width white  (0.5, 1.0, 1.0, 1.0)
//   - Duplicate key  → color = red               (1.0, 0.0, 0.0, 1.0)
//
// Then draws "CONTROL1".."CONTROL4" labels with the chosen color.
// ---------------------------------------------------------------------------
void __fastcall OptionsMenu_RenderControls(void* param_1)
{
    // -----------------------------------------------------------------------
    // Local stack frame (preserved from decompilation)
    // -----------------------------------------------------------------------
    int        iter;            // local_7c  — outer loop counter (0..3)
    Matrix4f   color_mtx;       // local_78  — 20-byte temp for color matrix
    float      color_r;         // local_74
    float      color_g;         // local_70
    float      color_b;         // local_6c
    float      color_a;         // local_68
    Matrix4f   tmp_mtx_1;       // local_64  — temp matrix (keyboard path)
    Matrix4f   tmp_mtx_2;       // local_50  — temp matrix (mouse path)
    Matrix4f   tmp_mtx_3;       // local_3c  — temp matrix (duplicate path)
    char       label_buf[28];   // local_28  — "CONTROL%d" output buffer

    // Dereference App → Scene pointer at +0x878 once.
    Scene* scene = *(Scene**)((uint8_t*)param_1 + 0x878);

    iter = 0;

    // -----------------------------------------------------------------------
    // Outer loop: walk CONTROL1..4 (offsets 0xB28, 0xB2C, 0xB30, 0xB34)
    // -----------------------------------------------------------------------
    for (int ctrl_offset = CONTROL_1;
         ctrl_offset < CONTROL_END;
         ctrl_offset += 4, ++iter)
    {
        // Default color matrix = identity (white, full size)
        Matrix_Scale4x4(&color_mtx, FLOAT_1_0, FLOAT_1_0, FLOAT_1_0, FLOAT_1_0);

        // Read the control type from the App binding slot
        int ctrl_type = *(int*)((uint8_t*)scene + ctrl_offset);

        int inner_idx = 0;

        // -------------------------------------------------------------------
        // Branch 1: Keyboard binding (type == 99)
        // Color = (0.5, 1.0, 0.5, 1.0)  — half-width green-tinted white
        // -------------------------------------------------------------------
        if (ctrl_type == TYPE_KEYBOARD)
        {
            Matrix4f* p = Matrix_Scale4x4(&tmp_mtx_1,
                                           FLOAT_0_5, FLOAT_1_0,
                                           FLOAT_0_5, FLOAT_1_0);
            color_r = p->x;
            color_g = p->y;
            color_b = p->z;
            color_a = p->w;
            Matrix_Identity(&tmp_mtx_1);
        }
        // -------------------------------------------------------------------
        // Branch 2: Mouse binding (type == 100)
        // Color = (0.5, 1.0, 1.0, 1.0)  — half-width cyan-tinted white
        // -------------------------------------------------------------------
        else if (ctrl_type == TYPE_MOUSE)
        {
            Matrix4f* p = Matrix_Scale4x4(&tmp_mtx_2,
                                           FLOAT_0_5, FLOAT_1_0,
                                           FLOAT_1_0, FLOAT_1_0);
            color_r = p->x;
            color_g = p->y;
            color_b = p->z;
            color_a = p->w;
            Matrix_Identity(&tmp_mtx_2);
        }
        // -------------------------------------------------------------------
        // Branch 3: Duplicate-key detection
        // If any OTHER control slot has the SAME value, color = RED.
        // -------------------------------------------------------------------
        else
        {
            for (int other_offset = CONTROL_1;
                 other_offset < CONTROL_END;
                 other_offset += 4, ++inner_idx)
            {
                // Skip self-comparison
                if (inner_idx == iter)
                    continue;

                int other_type = *(int*)((uint8_t*)scene + other_offset);
                int this_type  = *(int*)((uint8_t*)scene + ctrl_offset);

                if (this_type == other_type)
                {
                    // Duplicate found → color = pure red (1.0, 0.0, 0.0, 1.0)
                    Matrix4f* p = Matrix_Scale4x4(&tmp_mtx_3,
                                                   FLOAT_1_0, FLOAT_0_0,
                                                   FLOAT_0_0, FLOAT_1_0);
                    color_r = p->x;
                    color_g = p->y;
                    color_b = p->z;
                    color_a = p->w;
                    Matrix_Identity(&tmp_mtx_3);
                }
            }
        }

        // -------------------------------------------------------------------
        // Build label: "CONTROL1", "CONTROL2", "CONTROL3", "CONTROL4"
        // -------------------------------------------------------------------
        AthenaString_SprintfToBuffer(label_buf, "CONTROL%d", iter + 1);

        // -------------------------------------------------------------------
        // Draw the UI element with the computed color
        // -------------------------------------------------------------------
        UIList_SetColorsByName(
            param_1,
            (void*)0x004CF300,   // &PTR_Vec3_dtor_004cf300 — color vtable
            color_r,
            color_g,
            color_b,
            color_a,
            label_buf
        );

        // Reset color matrix to identity for next iteration
        Matrix_Identity(&color_mtx);
    }
}
