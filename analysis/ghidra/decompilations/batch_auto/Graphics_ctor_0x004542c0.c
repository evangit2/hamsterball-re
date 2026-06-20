/*
 * Function: Graphics_ctor
 * Address: 0x004542c0
 * Signature: void * __thiscall ...(void *this, undefined4 param_1, undefined1 param_2)
 * Parameters:
 *   this: Graphics* (vtable=0x4D88A0) | param_1: App/parent (stored at +0x5C) | param_2: undefined1 (stored at +4)
 *
 * Description:
 * Main Graphics constructor. Initializes RenderContext (+0xC), AthenaList (+0x2E4), Timer (+0x74C), Vec3 (+0x7AC). Allocates D3DX_ErrorHandler (0x68 bytes) at +0x748. Calls GetModuleHandle for D3D8. 2 calls.
 *
 * Struct offsets:
 *   +0x04 (param_2), +0x0C (RenderContext), +0x2E4 (AthenaList), +0x5C (parent), +0x154 (D3D device), +0x748 (D3DX_ErrorHandler), +0x74C (Timer), +0x7AC (Vec3)
 *
 * Cross-references:
 *   2 calls from App_CreateGraphics
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */
