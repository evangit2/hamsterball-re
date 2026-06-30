
// Function: ArenaLevelSelect_Render (0x432D20)
// Arena/Rodent Rumble level selection screen render function.
// vtable[2] of 0x4D47B8 (set by ArenaMenu_ctor at 0x42FC40).
// Previously misnamed TourneyMenu_Render.

void __thiscall ArenaLevelSelect_Render(void *this,void *param_1)

{
  undefined4 *puVar1;
  byte *pbVar2;
  void *this_00;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined1 *puVar11;
  undefined **ppuVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  int iVar17;
  int *piVar18;
  undefined4 auStack_4a4 [5];
  undefined4 uStack_490;
  undefined4 uStack_48c;
  undefined4 uStack_488;
  undefined4 uStack_484;
  undefined1 auStack_480 [4];
  undefined4 auStack_47c [5];
  undefined4 uStack_468;
  undefined4 uStack_464;
  undefined4 uStack_460;
  undefined4 uStack_45c;
  undefined4 uStack_454;
  undefined4 uStack_450;
  undefined4 uStack_44c;
  undefined4 uStack_448;
  undefined4 uStack_440;
  undefined4 uStack_43c;
  undefined4 uStack_438;
  undefined4 uStack_434;
  undefined4 uStack_430;
  int iStack_42c;
  int iStack_24;
  void *pvStack_18;
  int iStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cb634;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  *(undefined4 *)((int)param_1 + 0x708) = 1;
  Graphics_ClearViewport(param_1,0xff3f3f7f);
  Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3e851eb8);
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x410),&PTR_LAB_004cf584,0.0,0.0,660.0,600.0,
             &PTR_LAB_004cf584,0x3f800000,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0,0x3f3851ec);
  UI_DrawRectAndReset(param_1);
  Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0,0x3f3851ec);
  UI_DrawRectAndReset(param_1);
  Gfx_SetAlphaBlendState((int)param_1);
  Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x434),&PTR_LAB_004cf584,5.0,205.0,135.0,256.0
             ,&PTR_LAB_004cf584,0,0,0x3f07ae14,0x3f800000);
  Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  Gfx_DrawQuadRandomColor
            (*(void **)(*(int *)((int)this + 0x878) + 0x434),&PTR_LAB_004cf584,16.0,75.0,120.0,256.0
             ,&PTR_LAB_004cf584,0x3f07ae14,0,0x3f800000,0x3f800000);
  uStack_490 = 0;
  uStack_48c = 1;
  uStack_488 = 2;
  uStack_484 = 3;
  AthenaList_Init(&uStack_430,0);
  puVar1 = &uStack_490;
  iStack_10 = 0;
  iVar17 = 4;
  do {
    if (*(int *)((int)puVar1 + *(int *)((int)this + 0x878) + (0xb28 - (int)&uStack_490)) < 100) {
      AthenaList_Append(&uStack_430,(int)puVar1);
    }
    puVar1 = puVar1 + 1;
    iVar17 = iVar17 + -1;
  } while (iVar17 != 0);
  puVar11 = auStack_480;
  _eh_vector_constructor_iterator_(puVar11,0x14,4,Vec3_Init,Matrix_Identity);
  iStack_10._0_1_ = 1;
  iVar17 = Vec3_Init(auStack_4a4,0x3f800000,0x3f800000,0x3f800000);
  auStack_47c[0] = *(undefined4 *)(iVar17 + 4);
  auStack_47c[1] = *(undefined4 *)(iVar17 + 8);
  auStack_47c[2] = *(undefined4 *)(iVar17 + 0xc);
  auStack_47c[3] = *(undefined4 *)(iVar17 + 0x10);
  Matrix_Identity(auStack_4a4);
  iVar17 = Vec3_Init(auStack_4a4,0,0x3f000000,0x3f800000);
  uStack_468 = *(undefined4 *)(iVar17 + 4);
  uStack_464 = *(undefined4 *)(iVar17 + 8);
  uStack_460 = *(undefined4 *)(iVar17 + 0xc);
  uStack_45c = *(undefined4 *)(iVar17 + 0x10);
  Matrix_Identity(auStack_4a4);
  iVar17 = Vec3_Init(auStack_4a4,0x3f800000,0x3e800000,0x3e800000);
  uStack_454 = *(undefined4 *)(iVar17 + 4);
  uStack_450 = *(undefined4 *)(iVar17 + 8);
  uStack_44c = *(undefined4 *)(iVar17 + 0xc);
  uStack_448 = *(undefined4 *)(iVar17 + 0x10);
  Matrix_Identity(auStack_4a4);
  uVar15 = 0x3f800000;
  uVar14 = 0x3f800000;
  uVar13 = 0x4330a9;
  iVar17 = Vec3_Init(auStack_4a4,0x3f800000,0x3f800000,0);
  uStack_440 = *(undefined4 *)(iVar17 + 4);
  uStack_43c = *(undefined4 *)(iVar17 + 8);
  uStack_438 = *(undefined4 *)(iVar17 + 0xc);
  uStack_434 = *(undefined4 *)(iVar17 + 0x10);
  Matrix_Identity(auStack_4a4);
  uVar16 = 0x4330e6;
  iVar17 = AthenaList_GetSize((int)&uStack_430);
  if (iVar17 == 2) {
    Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x4b,0x4b,puVar11,uVar13,uVar14,
               uVar15,uVar16);
    if (iStack_42c < 2) {
      piVar18 = (int *)0x0;
    }
    else {
      piVar18 = *(int **)(iStack_24 + 4);
    }
    iVar17 = *piVar18;
    ppuVar12 = &PTR_Vec3_dtor_004cf300;
    uVar13 = auStack_47c[iVar17 * 5];
    uVar14 = auStack_47c[iVar17 * 5 + 1];
    uVar15 = auStack_47c[iVar17 * 5 + 2];
    uVar16 = auStack_47c[iVar17 * 5 + 3];
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x4b,0x20d,&PTR_Vec3_dtor_004cf300,
               uVar13,uVar14,uVar15,uVar16);
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    uVar6 = 0x433921;
    Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
    iStack_10._0_1_ = 2;
    Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar5 = 4;
    iVar4 = 4;
    iVar3 = 0x14;
    iVar17 = 0x4b;
    iStack_10._0_1_ = 1;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,uVar6
               ,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    uVar6 = 0x4339ab;
    Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
    iStack_10._0_1_ = 3;
    Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar5 = 4;
    iVar4 = 4;
    iVar3 = 0x1d6;
    iStack_10._0_1_ = 1;
    iVar17 = 0x4b;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    this_00 = *(void **)(*(int *)((int)this + 0x878) + 0x328);
  }
  else {
    if (iVar17 != 3) {
      if (iVar17 == 4) {
        Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        Scene_CreateObject4f
                  (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,-25.0,-25.0,
                   150.0,150.0,puVar11,uVar13,uVar14,uVar15,uVar16);
        if (iStack_42c < 2) {
          piVar18 = (int *)0x0;
        }
        else {
          piVar18 = *(int **)(iStack_24 + 4);
        }
        iVar17 = *piVar18;
        ppuVar12 = &PTR_Vec3_dtor_004cf300;
        uVar13 = auStack_47c[iVar17 * 5];
        uVar14 = auStack_47c[iVar17 * 5 + 1];
        uVar15 = auStack_47c[iVar17 * 5 + 2];
        uVar16 = auStack_47c[iVar17 * 5 + 3];
        Scene_CreateObject4f
                  (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,40.0,40.0,150.0
                   ,150.0,&PTR_Vec3_dtor_004cf300,uVar13,uVar14,uVar15,uVar16);
        uVar10 = 0x3f800000;
        uVar9 = 0;
        uVar8 = 0;
        uVar7 = 0;
        *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x328) + 0x428) = 0x3f000000;
        uVar6 = 0x433206;
        Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
        iStack_10._0_1_ = 7;
        Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar5 = 2;
        iVar4 = 2;
        iVar3 = 0x12;
        iVar17 = 0x32;
        iStack_10._0_1_ = 1;
        pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
        UI_DrawTextCenteredAbsolute
                  (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,
                   uVar6,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
        uVar10 = 0x3f800000;
        uVar9 = 0;
        uVar8 = 0;
        uVar7 = 0;
        uVar6 = 0x433290;
        Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
        iStack_10._0_1_ = 8;
        Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar5 = 2;
        iVar4 = 2;
        iVar3 = 0x53;
        iVar17 = 0x73;
        iStack_10._0_1_ = 1;
        pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
        UI_DrawTextCenteredAbsolute
                  (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,
                   uVar6,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
        if (iStack_42c < 3) {
          piVar18 = (int *)0x0;
        }
        else {
          piVar18 = *(int **)(iStack_24 + 8);
        }
        iVar17 = *piVar18;
        ppuVar12 = &PTR_Vec3_dtor_004cf300;
        uVar13 = auStack_47c[iVar17 * 5];
        uVar14 = auStack_47c[iVar17 * 5 + 1];
        uVar15 = auStack_47c[iVar17 * 5 + 2];
        uVar16 = auStack_47c[iVar17 * 5 + 3];
        Scene_CreateObject4f
                  (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,-25.0,475.0,
                   150.0,150.0,&PTR_Vec3_dtor_004cf300,uVar13,uVar14,uVar15,uVar16);
        uVar10 = 0x3f800000;
        uVar9 = 0;
        uVar8 = 0;
        uVar7 = 0;
        uVar6 = 0x4333a9;
        Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
        iStack_10._0_1_ = 9;
        Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar5 = 2;
        iVar4 = 2;
        iVar3 = 0x206;
        iVar17 = 0x32;
        iStack_10._0_1_ = 1;
        pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
        UI_DrawTextCenteredAbsolute
                  (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,
                   uVar6,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
        if (iStack_42c < 4) {
          piVar18 = (int *)0x0;
        }
        else {
          piVar18 = *(int **)(iStack_24 + 0xc);
        }
        iVar17 = *piVar18;
        ppuVar12 = &PTR_Vec3_dtor_004cf300;
        uVar13 = auStack_47c[iVar17 * 5];
        uVar14 = auStack_47c[iVar17 * 5 + 1];
        uVar15 = auStack_47c[iVar17 * 5 + 2];
        uVar16 = auStack_47c[iVar17 * 5 + 3];
        Scene_CreateObject4f
                  (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,40.0,410.0,
                   150.0,150.0,&PTR_Vec3_dtor_004cf300,uVar13,uVar14,uVar15,uVar16);
        uVar10 = 0x3f800000;
        uVar9 = 0;
        uVar8 = 0;
        uVar7 = 0;
        uVar6 = 0x4334c5;
        Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
        iStack_10._0_1_ = 10;
        Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
        iVar5 = 2;
        iVar4 = 2;
        iVar3 = 0x1c5;
        iVar17 = 0x73;
        iStack_10._0_1_ = 1;
        pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
        UI_DrawTextCenteredAbsolute
                  (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,
                   uVar6,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
        *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x328) + 0x428) = 0x3f800000;
      }
      goto LAB_00433a22;
    }
    Matrix_Scale4x4(&stack0xfffffb2c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    Scene_CreateObject4f
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,-25.0,-25.0,150.0,
               150.0,puVar11,uVar13,uVar14,uVar15,uVar16);
    if (iStack_42c < 2) {
      piVar18 = (int *)0x0;
    }
    else {
      piVar18 = *(int **)(iStack_24 + 4);
    }
    iVar17 = *piVar18;
    ppuVar12 = &PTR_Vec3_dtor_004cf300;
    uVar13 = auStack_47c[iVar17 * 5];
    uVar14 = auStack_47c[iVar17 * 5 + 1];
    uVar15 = auStack_47c[iVar17 * 5 + 2];
    uVar16 = auStack_47c[iVar17 * 5 + 3];
    Scene_CreateObject4f
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),&PTR_LAB_004cf584,40.0,40.0,150.0,
               150.0,&PTR_Vec3_dtor_004cf300,uVar13,uVar14,uVar15,uVar16);
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x328) + 0x428) = 0x3f000000;
    uVar6 = 0x433667;
    Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
    iStack_10._0_1_ = 4;
    Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar5 = 2;
    iVar4 = 2;
    iVar3 = 0x12;
    iVar17 = 0x32;
    iStack_10._0_1_ = 1;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,uVar6
               ,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    uVar6 = 0x4336f1;
    Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
    iStack_10._0_1_ = 5;
    Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar5 = 2;
    iVar4 = 2;
    iVar3 = 0x53;
    iVar17 = 0x73;
    iStack_10._0_1_ = 1;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    UI_DrawTextCenteredAbsolute
              (*(void **)(*(int *)((int)this + 0x878) + 0x328),pbVar2,iVar17,iVar3,iVar4,iVar5,uVar6
               ,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,uVar14,uVar15,uVar16);
    *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x328) + 0x428) = 0x3f800000;
    if (iStack_42c < 3) {
      piVar18 = (int *)0x0;
    }
    else {
      piVar18 = *(int **)(iStack_24 + 8);
    }
    iVar17 = *piVar18;
    ppuVar12 = &PTR_Vec3_dtor_004cf300;
    uVar13 = auStack_47c[iVar17 * 5];
    uVar14 = auStack_47c[iVar17 * 5 + 1];
    uVar15 = auStack_47c[iVar17 * 5 + 2];
    uVar16 = auStack_47c[iVar17 * 5 + 3];
    SceneObject_InitAtPosition
              (*(void **)(*(int *)((int)this + 0x878) + 0x438),0x4b,0x20d,&PTR_Vec3_dtor_004cf300,
               uVar13,uVar14,uVar15,uVar16);
    uVar10 = 0x3f800000;
    uVar9 = 0;
    uVar8 = 0;
    uVar7 = 0;
    uVar6 = 0x4337f9;
    Matrix_Scale4x4(&stack0xfffffb2c,0,0,0,0x3f800000);
    iStack_10._0_1_ = 6;
    Matrix_Scale4x4(&stack0xfffffb18,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar5 = 4;
    iVar4 = 4;
    iVar3 = 0x1d6;
    iVar17 = 0x4b;
    iStack_10._0_1_ = 1;
    pbVar2 = (byte *)AthenaString_Format(0x4f7448,&DAT_004d03f8);
    this_00 = *(void **)(*(int *)((int)this + 0x878) + 0x328);
  }
  UI_DrawTextCenteredAbsolute
            (this_00,pbVar2,iVar17,iVar3,iVar4,iVar5,uVar6,uVar7,uVar8,uVar9,uVar10,ppuVar12,uVar13,
             uVar14,uVar15,uVar16);
LAB_00433a22:
  if (*(int *)((int)param_1 + 0x704) != 0) {
    *(int *)((int)param_1 + 0x7c8) = *(int *)((int)param_1 + 0x7c8) + 1;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    *(undefined4 *)((int)param_1 + 0x704) = 0;
  }
  UIList_Render(this,param_1);
  iStack_10 = (uint)iStack_10._1_3_ << 8;
  _eh_vector_destructor_iterator_(auStack_480,0x14,4,Matrix_Identity);
  iStack_10 = 0xffffffff;
  Vec3List_Free(&uStack_430);
  ExceptionList = pvStack_18;
  return;
}

