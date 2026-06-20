/*
 * Function: Scene_FindTextureDimensions
 * Address: 0x0044ab00
 * Signature: void __thiscall ...(void *this, undefined4 *param_1, char *param_2)
 * Parameters:
 *   this: Scene* | param_1: output struct (5 fields) | param_2: char* texture name
 *
 * Description:
 * Like FindTextureByName but also measures text width via Font_MeasureText. Returns vtable, data ptr, width, height, measured text width.
 *
 * Struct offsets:
 *   +0x88C (AthenaList), +0x890 (count), +0xC98 (array). Item: +4 (ptr→name), +0x1C (tex→+4/+8/+0x10)
 *
 * Cross-references:
 *   4+ calls from GraphicsOptionsMenu_Update
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
