/*
 * Function: StdString_Insert
 * Address: 0x00447390
 * Signature: void * __thiscall StdString_Insert(void *this, uint param_1, undefined4 *param_2, uint param_3)
 * Parameters:
 *   this: StdString* — the string being modified
 *   param_1: uint — position in 'this' to insert at
 *   param_2: undefined4* — source StdString to insert from
 *   param_3: uint — number of characters to insert
 *
 * Description:
 * MSVC STL std::string::insert implementation. Inserts characters from a source
 * StdString into 'this' at position param_1. Handles:
 *   - Self-insertion: if param_2 points within 'this' string's buffer, delegates
 *     to StdString_Replace with adjusted offset
 *   - Bounds checking (throws CRT_ThrowInvalidStringPosition)
 *   - Length overflow checking (throws CRT_ThrowStringTooLong)
 *   - SSO handling (inline buffer at +4 vs heap at *+4)
 *   - Buffer reallocation via String_AllocBuffer
 *   - _memmove for shifting existing content right
 *
 * Struct offsets (StdString layout):
 *   +0x04: char* data (inline SSO buffer or heap pointer)
 *   +0x14: uint length
 *   +0x18: uint capacity (if > 0xF, data is heap-allocated)
 *
 * Cross-references:
 *   Called from StdString_InsertCStr (0x447590) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386, MSVC STL)
 */
