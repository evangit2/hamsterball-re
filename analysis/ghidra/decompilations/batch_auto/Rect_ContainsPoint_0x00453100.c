/*
 * Function: Rect_ContainsPoint
 * Address: 0x00453100
 * Signature: ushort __thiscall ...(void *this, float param_1, float param_2)
 * Parameters:
 *   this: Rect* (x,y,w,h at +4/+8/+0xC/+0x10) | param_1: float x | param_2: float y
 *
 * Description:
 * Tests if point (x,y) is inside rectangle. Checks x>=left(+4), y>=top(+8), x<left+width(+4+0xC), y<top+height(+8+0x10). Returns 1 if inside, 0 otherwise. Handles NaN comparison flags. 1 call.
 *
 * Struct offsets:
 *   +0x04 (left), +0x08 (top), +0x0C (width), +0x10 (height)
 *
 * Cross-references:
 *   1 call from UIWidget_HitTest (0x469B40)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
