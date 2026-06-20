/*
 * Function: MeshNode_Level_DeleteDtor3
 * Address: 0x00437EF0
 * Signature: void * __thiscall MeshNode_Level_DeleteDtor3(void *this, byte param_1)
 * Parameters:
 *   this: MeshNode* — the MeshNode-derived object to destroy
 *   param_1: byte — scalar deleting flag. If bit 0 is set (param_1 & 1), free the memory after destructing.
 *
 * Description:
 * Scalar deleting destructor for MeshNode Level objects (vtable variant 3).
 * This is the standard MSVC scalar deleting destructor pattern:
 *   1. Calls MeshNode_LevelDtor3(this) — the actual destructor logic
 *   2. If param_1 & 1, calls _free(this) to release the memory
 *
 * The function is referenced from the vtable at 0x4D4F98 (DATA reference) — this
 * is the vtable pointer that Catapult_ctor and similar objects set, making this
 * their destructor entry point.
 *
 * Struct offsets:
 *   N/A — this is a dispatcher, actual cleanup is in MeshNode_LevelDtor3
 *
 * Cross-references:
 *   Referenced from vtable at 0x4D4F8 [DATA] — this vtable is used by:
 *   Catapult_ctor (0x437E10), and other MeshNode-derived level objects
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
