/*
 * Function: Level_SetObjectTransform
 * Address: 0x0040b9c0
 *
 * Description:
 *
Copies transform data from a source SceneObject to this SceneObject.
Copies 4 groups of 3-floats (position, rotation, scale, and another vector)
from param_1 to this at offsets +0x04/+0x08/+0x0C, +0x14/+0x18/+0x1C,
+0x24/+0x28/+0x2C, +0x34/+0x38/+0x3C. Also copies flags at +0x48/+0x4C
and texture/material info at +0x50. Sets the "transform changed" flag at
+0x4C each time. Calls Graphics_ApplyMaterialAndDraw via the caller.

Cross-refs: 4 calls — Graphics_ApplyMaterialAndDraw, Level_AssignTextures,
Level_InitMeshes4, Level_InitMeshes.

 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* See GhidraMCP for full decompiled body */
