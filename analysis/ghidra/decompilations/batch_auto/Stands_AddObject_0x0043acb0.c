/*
 * Function: Stands_AddObject
 * Address: 0x0043ACB0
 * Signature: void __thiscall Stands_AddObject(void *this, int param_1)
 * Parameters:
 *   this: Stands* — the stands object to add an object to
 *   param_1: int — pointer to the object being added (likely a Ball* or collision object)
 *
 * Description:
 * Adds an object to the stands' tracking list (AthenaList at +0x1108).
 * Steps:
 *   1. Gets next free index from AthenaList at +0x1108
 *   2. Sets the slot at +0x1110 + index×4 to 0 (pending)
 *   3. Iterates through existing entries in the list:
 *      a. If an existing entry matches param_1 (same object pointer), resets its
 *         timer to 10 (refreshes the tracking — the object is still on the stands)
 *      b. Otherwise continues to next entry
 *   4. If no matching entry found, allocates a new 8-byte tracking node:
 *      - node[0] = param_1 (object pointer)
 *      - node[1] = 10 (timer, 10 frames before auto-removal)
 *   5. Appends the new node to the AthenaList at +0x1108
 *
 * This function is called when a ball or other object touches/lands on the stands.
 * The timer of 10 frames means the object must remain in contact or it will be
 * automatically removed from the tracking list by Stands_Update.
 *
 * Struct offsets:
 *   +0x1108: AthenaList (tracked objects on stands)
 *   +0x110C: AthenaList count
 *   +0x1110+: per-entry active flags (array)
 *   +0x1514: AthenaList data pointer
 *
 * Cross-references:
 *   Called from 0x40F9DD — UNCONDITIONAL_CALL (Arena_HandleCollision)
 *   Called from 0x41558A — UNCONDITIONAL_CALL (CreateWobbly1 or similar)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
