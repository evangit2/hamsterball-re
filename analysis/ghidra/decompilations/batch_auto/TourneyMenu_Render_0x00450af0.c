
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall TourneyMenu_Render(void *this,void *param_1)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  ulonglong uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 in_stack_ffffffb8;
  int *in_stack_ffffffbc;
  undefined4 in_stack_ffffffc0;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined1 *this_00;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cca50;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  uVar15 = 0xff7f7fff;
  *(undefined4 *)((int)param_1 + 0x708) = 1;
  uVar14 = 0x450b3b;
  Graphics_ClearViewport(param_1,0xff7f7fff);
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3e851eb8);
  puStack_8 = &stack0xffffff90;
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x410),&PTR_LAB_004cf584,0.0,0.0,660.0,600.0,
             &PTR_LAB_004cf584,0x3f800000,0,0,0x3f800000);
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0,0x3f3851ec);
  puStack_8 = &stack0xffffffa4;
  UI_DrawRectAndReset(param_1);
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0,0x3f3851ec);
  puStack_8 = &stack0xffffffa4;
  UI_DrawRectAndReset(param_1);
  iVar3 = 0x16 - *(int *)((int)this + 0xcfc);
  iVar2 = 0x3b4;
  do {
    puStack_8 = (undefined1 *)iVar3;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Graphics_DrawScreenRect(param_1,0x7f,iVar3 + 5,0x80,0x80);
    this_00 = *(undefined1 **)(iVar2 + *(int *)((int)this + 0x878));
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    Sprite_DrawRect(this_00,122.0,(float)(int)puStack_8);
    iVar3 = iVar3 + 0x9b;
    iVar2 = iVar2 + 4;
  } while (iVar2 < 0x3f0);
  if (((*(char *)((int)this + 0xce0) != '\0') &&
      (iVar2 = *(int *)(*(int *)((int)this + 0x878) + 0x220), *(char *)(iVar2 + 0x95) == '\0')) &&
     (*(char *)(iVar2 + 0x96) == '\0')) {
    puStack_8 = &stack0xffffffb8;
    iVar2 = *(int *)(iVar2 + 8) * 0x9b - *(int *)((int)this + 0xcfc);
    Matrix_Scale4x4(&stack0xffffffb8,0,0x3f800000,0,0x3f800000);
    puStack_8 = (undefined1 *)(float)(iVar2 + 0x11);
    UI_DrawRectAndReset(param_1);
    Matrix_Scale4x4(&stack0xffffffb8,0,0x3f800000,0,0x3f800000);
    UI_DrawRectAndReset(param_1);
    Matrix_Scale4x4(&stack0xffffffb8,0,0x3f800000,0,0x3f800000);
    UI_DrawRectAndReset(param_1);
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0,0x3f800000,0,0x3f800000);
    puStack_8 = (undefined1 *)(iVar2 + 0x91);
    UI_DrawRectAndReset(param_1);
  }
  if (*(char *)((int)this + 0x1108) != '\0') {
    puStack_8 = (undefined1 *)
                ((*(int *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 8) * 0x9b -
                 *(int *)((int)this + 0xcfc)) + 0x42);
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    TourneyMenu_DrawHighlightQuad
              (*(void **)(*(int *)((int)this + 0x878) + 0x42c),250.0,(float)(int)puStack_8,
               *(float *)((int)this + 0x1114),*(float *)((int)this + 0x1110),in_stack_ffffffb8,
               in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  }
  iVar2 = *(int *)(*(int *)((int)this + 0x878) + 0x220);
  if ((*(char *)(iVar2 + 0x95) != '\0') && (*(char *)(iVar2 + 0x96) == '\0')) {
    iVar2 = *(int *)(iVar2 + 8);
    iVar3 = *(int *)((int)this + 0xcfc);
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x414),0xc4,(iVar2 * 0x9b - iVar3) + 0x56,
               in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  }
  iVar2 = *(int *)(*(int *)((int)this + 0x878) + 0x220);
  puStack_8 = *(undefined1 **)(iVar2 + 8);
  if (*(char *)(iVar2 + 0x96) != '\0') {
    puStack_8 = (undefined1 *)((int)puStack_8 + 1);
  }
  iVar2 = 0;
  if (0 < (int)puStack_8) {
    iVar3 = 0x34 - *(int *)((int)this + 0xcfc);
    do {
      Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
      SceneObject_InitAtPosition
                (*(void **)(*(int *)((int)this + 0x878) + 0x400),0xfc,iVar3 + 0x14,in_stack_ffffffb8
                 ,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
      Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      SceneObject_InitAtPosition
                (*(void **)(*(int *)((int)this + 0x878) + 0x400),0xf7,iVar3 + 0xf,in_stack_ffffffb8,
                 in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
      uVar13 = 0x3f800000;
      uVar12 = 0;
      uVar11 = 0;
      uVar10 = 0;
      uVar9 = 0x451028;
      Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
      if (iVar2 == 0) {
        Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        UI_DrawTextCenteredAbsolute
                  (*(void **)(*(int *)((int)this + 0x878) + 800),(byte *)"DONE!",0xf7,iVar3,3,3,
                   uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                   in_stack_ffffffc0,uVar14,uVar15);
      }
      else {
        Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar7 = 3;
        iVar5 = 3;
        iVar6 = iVar3 + -0xf;
        iVar8 = 0x101;
        pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
        UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0x878) + 0x318),pbVar1,iVar8,iVar6,iVar5
                            ,iVar7,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,
                            in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
        uVar13 = 0x3f800000;
        uVar12 = 0;
        uVar11 = 0;
        uVar10 = 0;
        uVar9 = 0x45111d;
        Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
        Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar7 = 3;
        iVar5 = 3;
        iVar8 = 0x102;
        iVar6 = iVar3;
        pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
        UI_DrawTextShadow_Wrapper
                  (*(void **)(*(int *)((int)this + 0x878) + 800),pbVar1,iVar8,iVar6,iVar5,iVar7,
                   uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                   in_stack_ffffffc0,uVar14,uVar15);
      }
      this_00 = &stack0xffffffa4;
      iVar3 = iVar3 + 0x9b;
      iVar2 = iVar2 + 1;
    } while (iVar2 < (int)puStack_8);
  }
  if (*(int *)((int)param_1 + 0x704) != 2) {
    uVar15 = 3;
    uVar14 = 0xd;
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    in_stack_ffffffbc = *(int **)((int)param_1 + 0x154);
    in_stack_ffffffc0 = 0;
    (**(code **)(*in_stack_ffffffbc + 0xfc))();
    in_stack_ffffffb8 = 3;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    *(undefined4 *)((int)param_1 + 0x704) = 2;
  }
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0x878) + 0x40c),0x48,0x52,in_stack_ffffffb8,
             in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  if (*(char *)(*(int *)((int)this + 0x878) + 0x236) != '\0') {
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    puStack_8 = &stack0xffffffa4;
    Scene_CreateObject4f
              (*(void **)(*(int *)((int)this + 0x878) + 0x3fc),&PTR_LAB_004cf584,8.0,318.0,128.0,
               158.0,in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  }
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  puStack_8 = &stack0xffffff90;
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x3f4),&PTR_LAB_004cf584,0.0,125.0,128.0,256.0
             ,&PTR_LAB_004cf584,0x3f000000,0,0x3f800000,0x3f800000);
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  puStack_8 = &stack0xffffff90;
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x3f4),&PTR_LAB_004cf584,0.0,315.0,128.0,256.0
             ,&PTR_LAB_004cf584,0,0,0x3f000000,0x3f800000);
  puStack_8 = &stack0xffffffb8;
  uVar9 = 0x3f800000;
  if (*(char *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x96) == '\0') {
    uVar13 = 0;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0x451390;
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"NEXT UP:",0x140,10,2,
                      2,uVar10,uVar11,uVar12,uVar13,uVar9,in_stack_ffffffb8,in_stack_ffffffbc,
                      in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x4513eb;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar8 = 2;
    iVar6 = 2;
    iVar3 = 0x2e;
    iVar2 = 0x13b;
    pbVar1 = TourneyMenu_GetRaceName(*(int *)(*(int *)((int)this + 0x878) + 0x220));
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x318),pbVar1,iVar2,iVar3,iVar6,iVar8
                      ,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                      in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x451460;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)((int)this + 0xd08),
                      0x140,100,2,2,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,
                      in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  }
  else {
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    puStack_8 = &stack0xffffffa4;
    Scene_CreateObject4f
              (*(void **)(*(int *)((int)this + 0x878) + 0x404),&PTR_LAB_004cf584,276.0,15.0,458.0,
               303.0,in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar3 = 0x5f;
    iVar2 = 0x191;
    uVar4 = __ftol2();
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x330 + (int)uVar4 * 4),iVar2,iVar3,
               in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = *(undefined1 **)((int)this + 0xd04);
    if (puStack_8 != (void *)0x0) {
      Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      Sprite_DrawRect(puStack_8,534.0,271.0);
    }
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x451575;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"FINAL RANKING BY SCORE:",
               0x296,0xf0,2,2,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
               in_stack_ffffffc0,uVar14,uVar15);
    if (*(char *)((int)this + 0xce0) != '\0') {
      puStack_8 = &stack0xffffffb8;
      uVar13 = 0x3f800000;
      uVar12 = 0;
      uVar11 = 0;
      uVar10 = 0;
      uVar9 = 0x4515d7;
      Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
      Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
      UI_DrawTextCenteredAbsolute
                (*(void **)(*(int *)((int)this + 0x878) + 0x31c),
                 (&PTR_s_HAMSTER_PELLET_004f70c8)[*(int *)((int)this + 0xd00)],0x296,0x104,2,2,uVar9
                 ,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,
                 uVar14,uVar15);
    }
  }
  if (*(int *)((int)param_1 + 0x704) != 0) {
    uVar15 = 1;
    uVar14 = 0xd;
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    in_stack_ffffffbc = *(int **)((int)param_1 + 0x154);
    in_stack_ffffffc0 = 0;
    (**(code **)(*in_stack_ffffffbc + 0xfc))();
    in_stack_ffffffb8 = 1;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    *(undefined4 *)((int)param_1 + 0x704) = 0;
  }
  puStack_8 = &stack0xffffffb8;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f400000,0x3f400000,0x3f800000,0x3f800000);
  puStack_8 = &stack0xffffffa4;
  Scene_CreateObject4f
            (*(void **)(*(int *)((int)this + 0x878) + 0x390),&PTR_LAB_004cf584,316.0,447.0,172.0,
             128.0,in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
  puStack_8 = &stack0xffffffb8;
  uVar12 = 0x3f800000;
  uVar11 = 0;
  uVar10 = 0;
  uVar9 = 0;
  if (*(char *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x96) == '\0') {
    uVar13 = 0x451705;
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"YOUR SCORE:",0x149,
                      0x1c7,2,2,uVar13,uVar9,uVar10,uVar11,uVar12,in_stack_ffffffb8,
                      in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x451770;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar8 = 2;
    iVar6 = 2;
    iVar3 = 0x1d7;
    iVar2 = 0x149;
    __ftol2();
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d0408);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),pbVar1,iVar2,iVar3,iVar6,iVar8
                      ,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                      in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x4517fb;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"TIME POOL:",0x148,
                      0x1f6,2,2,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,
                      in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x451880;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar8 = 3;
    iVar6 = 3;
    iVar3 = 0x209;
    iVar2 = 0x192;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCentered(*(void **)(*(int *)((int)this + 0x878) + 0x318),pbVar1,iVar2,iVar3,iVar6,
                        iVar8,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                        in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x45190b;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar8 = 3;
    iVar6 = 3;
    iVar3 = 0x218;
    iVar2 = 0x193;
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,(byte *)".%.1d");
    UI_DrawTextShadow_Wrapper
              (*(void **)(*(int *)((int)this + 0x878) + 800),pbVar1,iVar2,iVar3,iVar6,iVar8,uVar9,
               uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,in_stack_ffffffc0,
               uVar14,uVar15);
  }
  else {
    uVar13 = 0x451991;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"YOUR SCORE:",0x158,
                      0x1e5,2,2,uVar13,uVar9,uVar10,uVar11,uVar12,in_stack_ffffffb8,
                      in_stack_ffffffbc,in_stack_ffffffc0,uVar14,uVar15);
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x4519fc;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar8 = 2;
    iVar6 = 2;
    iVar3 = 0x1f5;
    iVar2 = 0x158;
    __ftol2();
    pbVar1 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d0408);
    UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),pbVar1,iVar2,iVar3,iVar6,iVar8
                      ,uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
                      in_stack_ffffffc0,uVar14,uVar15);
  }
  if (*(char *)(*(int *)(*(int *)((int)this + 0x878) + 0x220) + 0x94) != '\0') {
    puStack_8 = &stack0xffffffb8;
    uVar13 = 0x3f800000;
    uVar12 = 0;
    uVar11 = 0;
    uVar10 = 0;
    uVar9 = 0x451a9d;
    Matrix_Scale4x4(&stack0xffffffb8,0,0,0,0x3f800000);
    Matrix_Scale4x4(&stack0xffffffa4,0x3f800000,0,0,0x3f800000);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)0x4d85cc,0x180,0x23a,2,2,
               uVar9,uVar10,uVar11,uVar12,uVar13,in_stack_ffffffb8,in_stack_ffffffbc,
               in_stack_ffffffc0,uVar14,uVar15);
  }
  UIList_Render(this,param_1);
  if (*(float *)((int)this + 0xcf0) != _DAT_004cf368) {
    puStack_8 = &stack0xffffffb8;
    Matrix_Scale4x4(&stack0xffffffb8,0x3f800000,0x3f800000,0x3f800000,
                    *(undefined4 *)((int)this + 0xcf0));
    Graphics_DrawScreenRect(param_1,0,0,800,600);
  }
  if (*(char *)((int)param_1 + 0x7d2) == '\0') {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  else {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  *(undefined4 *)((int)param_1 + 0x708) = 3;
  ExceptionList = this_00;
  return;
}

