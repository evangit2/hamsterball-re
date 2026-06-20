/*
 * Function: StdString_Replace
 * Address: 0x004471E0
 * Signature: void * __thiscall StdString_Replace(void *this, uint param_1, void *param_2, uint param_3, uint param_4)
 * Parameters:
 *   this: StdString* — the string being modified
 *   param_1: uint — start position in 'this' to replace at
 *   param_2: void* — source StdString to copy from
 *   param_3: uint — start position in source string
 *   param_4: uint — number of characters to copy
 *
 * Description:
 * MSVC STL std::string::replace implementation. Replaces a range of characters
 * in 'this' starting at param_1 with characters from param_2 starting at param_3.
 * Handles:
 *   - Bounds checking (throws CRT_ThrowInvalidStringPosition if pos > length)
 *   - Length overflow checking (throws CRT_ThrowStringTooLong)
 *   - SSO (Small String Optimization): strings <= 15 chars stored inline at +4,
 *     longer strings use heap pointer at *+4, capacity at +0x18
 *   - Self-replacement (this == param_2) with adjusted offsets
 *   - Buffer reallocation via String_AllocBuffer when needed
 *   - _memmove for shifting existing content
 *
 * Struct offsets (StdString layout):
 *   +0x04: char* data (inline SSO buffer or heap pointer)
 *   +0x14: uint length
 *   +0x18: uint capacity (if > 0xF, data is heap-allocated)
 *
 * Cross-references:
 *   Called from StdString_Insert (0x4473DB) — UNCONDITIONAL_CALL
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386, MSVC STL)
 */
