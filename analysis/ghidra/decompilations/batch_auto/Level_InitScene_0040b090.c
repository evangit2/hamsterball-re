/*
 * Function: Level_InitScene
 * Address: 0x0040b090
 * Signature: Level_InitScene(...)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, audio, camera, scene, level. Calls: Level_InitScene, operator_new, SoundChannel_Ctor, Color_RandomRGBA, Gfx_SetVTable22, Gfx_SetVTable8B, Scene_ResetObjectSlots, Level_FindObjectByName. Offsets: 28, Lines: 112
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall Level_InitScene(int param_1)

{
  float fVar1;
  char cVar2;
  void *pvVar3;
  int iVar4;
  undefined4 *puVar5;
  int *piVar6;
  uint uVar7;
  int iVar8;
  char *pcVar9;
  char *pcVar10;
  void *pvStack_24;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c95be;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  puVar5 = operator_new(0x80);
  local_4 = 0;
  if (puVar5 == (undefined4 *)0x0) {
    piVar6 = (int *)0x0;
  }
  else {
    piVar6 = (int *)SoundChannel_Ctor(puVar5);
  }
  *(int **)(param_1 + 0x87c) = piVar6;
  local_4 = 0xffffffff;
  (**(code **)(*piVar6 + 8))(0,0xc2480000);
  (**(code **)(**(int **)(param_1 + 0x87c) + 0xc))
            (&stack0xfffffbd8,*(undefined4 *)(param_1 + 0x29c0));
  (**(code **)(**(int **)(*(int *)(param_1 + 0x878) + 0x174) + 4))(*(undefined4 *)(param_1 + 0x87c))
  ;
  if (*(int **)(param_1 + 0x8ac) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x8ac) + 4))();
    uVar7 = Color_RandomRGBA();
    Gfx_SetVTable22(*(void **)(*(int *)(param_1 + 0x878) + 0x174),uVar7);
    uVar7 = Color_RandomRGBA();
    Gfx_SetVTable8B(*(void **)(*(int *)(param_1 + 0x878) + 0x174),uVar7);
    Scene_ResetObjectSlots(*(int *)(param_1 + 0x8ac));
    *(undefined4 *)(param_1 + 0x3f1c) = 0;
    if (*(char *)(*(int *)(*(int *)(param_1 + 0x878) + 0x220) + 0x12) == '\0') {
      iVar8 = Level_FindObjectByName(*(void **)(param_1 + 0x8ac),"CAMERALOCUS");
      *(int *)(param_1 + 0x3f1c) = iVar8;
      if (iVar8 != 0) {
        *(int *)(param_1 + 0x3f20) = iVar8;
        *(undefined4 *)(param_1 + 0x3f24) = 0;
      }
    }
  }
  iVar8 = *(int *)(param_1 + 0x878);
  *(undefined4 *)(param_1 + 0x870) = *(undefined4 *)(iVar8 + 0x240);
  *(undefined4 *)(*(int *)(iVar8 + 0x294) + 4) = *(undefined4 *)(*(int *)(iVar8 + 0x298) + 4);
  *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2b4) + 4) =
       *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2b8) + 4);
  Level_SelectCameraProfile(param_1);
  iVar8 = *(int *)(param_1 + 0x878);
  if (*(char *)(iVar8 + 0x236) != '\0') {
    *(undefined4 *)(*(int *)(iVar8 + 0x294) + 4) = *(undefined4 *)(*(int *)(iVar8 + 0x2a0) + 4);
    *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2b4) + 4) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x878) + 0x2c0) + 4);
  }
  Graphics_SetCullMode2(*(void **)(*(int *)(param_1 + 0x878) + 0x174),1);
  pvVar3 = *(void **)(*(int *)(param_1 + 0x878) + 0x174);
  fVar1 = *(float *)((int)pvVar3 + 0x188);
  Graphics_SetProjection(pvVar3,20.0,(fVar1 + fVar1) * _DAT_004cf6fc + _DAT_004cf6f8);
  pvVar3 = *(void **)(*(int *)(param_1 + 0x878) + 0x174);
  Graphics_SetViewportZ
            (pvVar3,*(float *)((int)pvVar3 + 0x188) * _DAT_004cf6f4 + _DAT_004cf6f4,
             (*(float *)((int)pvVar3 + 0x188) + *(float *)((int)pvVar3 + 0x188)) * _DAT_004cf6fc +
             _DAT_004cf6f8);
  *(undefined1 *)(param_1 + 0x3f28) = 1;
  *(undefined1 *)(param_1 + 0x3f38) = 1;
  Window_Notify(0x5341d0,(byte *)"CamStart: %f");
  if (*(int *)(param_1 + 0x299c) != -0x40800000) {
    *(int *)(param_1 + 0x3f24) = *(int *)(param_1 + 0x299c);
    *(undefined1 *)(param_1 + 0x3f28) = 0;
    *(undefined4 *)(param_1 + 0x3f34) = *(undefined4 *)(param_1 + 0x299c);
    *(undefined1 *)(param_1 + 0x3f38) = 0;
  }
  iVar8 = *(int *)(param_1 + 0x878);
  if (*(void **)(iVar8 + 0x53c) != (void *)0x0) {
    if (*(char *)(*(int *)(iVar8 + 0x220) + 0x10) != '\0') {
      pcVar9 = *(char **)(param_1 + 0x4344);
      iVar4 = -(int)pcVar9;
      do {
        cVar2 = *pcVar9;
        pcVar9[(int)(&stack0xfffffbdc + iVar4)] = cVar2;
        pcVar9 = pcVar9 + 1;
      } while (cVar2 != '\0');
      pcVar9 = &stack0xfffffbdb;
      do {
        pcVar10 = pcVar9;
        pcVar9 = pcVar10 + 1;
      } while (pcVar10[1] != '\0');
      *(undefined4 *)(pcVar10 + 1) = s___No_Intro_004cf6d8._0_4_;
      *(undefined4 *)(pcVar10 + 5) = s___No_Intro_004cf6d8._4_4_;
      *(undefined4 *)(pcVar10 + 9) = s___No_Intro_004cf6d8._8_4_;
      Audio_PlayMusicAtSpeed(*(void **)(iVar8 + 0x53c),&stack0xfffffbdc,4.0);
      ExceptionList = pvStack_24;
      return;
    }
    Audio_PlayMusicAtSpeed(*(void **)(iVar8 + 0x53c),*(char **)(param_1 + 0x4344),2.0);
  }
  ExceptionList = pvStack_24;
  return;
}
