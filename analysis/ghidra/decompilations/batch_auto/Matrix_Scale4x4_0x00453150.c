/*
 * Function: Matrix_Scale4x4
 * Address: 0x00453150
 * Signature: void __thiscall ...(void *this, undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4)
 * Parameters:
 *   this: Matrix* | param_1-3: scale x/y/z | param_4: w component
 *
 * Description:
 * Initializes a 4x4 scale matrix. Sets vtable to 0x4CF300, stores 4 scale components at +4/+8/+0xC/+0x10. 4+ calls.
 *
 * Struct offsets:
 *   +0x00 (vtable=0x4CF300), +0x04 (scaleX), +0x08 (scaleY), +0x0C (scaleZ), +0x10 (w)
 *
 * Cross-references:
 *   4+ calls from SceneObj_SetScale, Graphics_SetViewportClip, ConfirmMenu_ctor
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
