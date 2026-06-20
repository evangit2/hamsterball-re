/*
 * Function: Ball_SetupCollisionRender
 * Address: 0x004015b0
 * Signature: void __fastcall Ball_SetupCollisionRender(int param_1)
 *
 * Patterns: audio, collision, rendering, ball, scene. Calls: Ball_SetupCollisionRender, Sound_GetNextChannel, Scene_RenderIfVisible, Sound_Play3DAtPosition. Offsets: 7, Lines: 27
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall Ball_SetupCollisionRender(int param_1)

{
  int iVar1;
  
  if (*(char *)(param_1 + 0x324) == '\0') {
    iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x440);
    if (iVar1 != 0) {
      iVar1 = Sound_GetNextChannel(iVar1);
      *(int *)(param_1 + 600) = iVar1;
      if (iVar1 != 0) {
        Scene_RenderIfVisible(iVar1);
        Sound_Play3DAtPosition(*(int *)(param_1 + 600));
      }
    }
    *(undefined4 *)(param_1 + 0x25c) = 0;
    if ((*(int *)(*(int *)(*(int *)(param_1 + 0x10) + 0x220) + 8) == 0xd) &&
       (iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x4fc), iVar1 != 0)) {
      iVar1 = Sound_GetNextChannel(iVar1);
      *(int *)(param_1 + 0xc54) = iVar1;
      if (iVar1 != 0) {
        Scene_RenderIfVisible(iVar1);
        Sound_Play3DAtPosition(*(int *)(param_1 + 0xc54));
      }
    }
  }
  return;
}
