/*
 * Function: Scene_FindTextureByName
 * Address: 0x0044aa40
 * Signature: void __thiscall ...(void *this, undefined4 *param_1, char *param_2)
 * Parameters:
 *   this: Scene* | param_1: output struct (5 fields) | param_2: char* texture name
 *
 * Description:
 * Searches AthenaList for texture by name via __stricmp. If found: copies texture data from item+0x1C into 5-field output (vtable, data ptr, w, h, format). If not found: zeros output.
 *
 * Struct offsets:
 *   +0x88C (AthenaList), +0x890 (count), +0xC98 (array). Item: +4 (name), +0x1C (texture data ptr→+4/+8/+0xC/+0x10)
 *
 * Cross-references:
 *   4+ calls from GraphicsOptionsMenu_Update
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
