/*
 * Function: GameObject_Render
 * Address: 0x00402290
 * Signature: void __fastcall GameObject_Render(int param_1)
 *
 * Patterns: vtable dispatch, rendering, scene. Calls: GameObject_Render, SceneObj_SetScale, Sprite_RenderQuad. Offsets: 13, Lines: 28
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall GameObject_Render(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xc60) != 0x3f800000) {
    SceneObj_SetScale(*(void **)(*(int *)(param_1 + 0x10) + 0x174),*(int *)(param_1 + 0xc60));
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
  if (*(char *)(iVar1 + 0x70c) != '\0') {
    (**(code **)(**(int **)(iVar1 + 0x154) + 200))(*(int **)(iVar1 + 0x154),0xe,0);
    *(undefined1 *)(iVar1 + 0x70c) = 0;
    *(int *)(iVar1 + 0x7c8) = *(int *)(iVar1 + 0x7c8) + 1;
  }
  Sprite_RenderQuad(*(void **)(*(int *)(param_1 + 0x10) + 0x368),*(float *)(param_1 + 0x164),
                    *(float *)(param_1 + 0x168),*(float *)(param_1 + 0x16c),1.33,(undefined4 *)0x0);
  iVar1 = *(int *)(*(int *)(param_1 + 0x10) + 0x174);
  if (*(char *)(iVar1 + 0x70c) != '\x01') {
    (**(code **)(**(int **)(iVar1 + 0x154) + 200))(*(int **)(iVar1 + 0x154),0xe,1);
    *(undefined1 *)(iVar1 + 0x70c) = 1;
    *(int *)(iVar1 + 0x7c8) = *(int *)(iVar1 + 0x7c8) + 1;
  }
  if (*(int *)(param_1 + 0xc60) != 0x3f800000) {
    *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7a8) = 0;
  }
  *(undefined4 *)(param_1 + 0xc38) =
       *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x10) + 0x174) + 0x7c4);
  return;
}
