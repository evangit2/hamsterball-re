/*
 * Function: Graphics_SetTexturePath
 * Address: 0x00454000
 * Signature: void __thiscall ...(void *this, char *param_1)
 * Parameters:
 *   this: Graphics* | param_1: char* texture directory path
 *
 * Description:
 * Frees existing texture path at +0x7D8. If param_1 non-NULL: allocates new string via operator_new (strlen+1), copies it. 1 call from App_Initialize.
 *
 * Struct offsets:
 *   +0x7D8 (texture path string)
 *
 * Cross-references:
 *   1 call from App_Initialize (0x46BC84)
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
