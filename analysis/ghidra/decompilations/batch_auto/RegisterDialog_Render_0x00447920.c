
void __thiscall RegisterDialog_Render(void *this,void *param_1)

{
  int iVar1;
  byte *pbVar2;
  undefined4 unaff_EBX;
  undefined1 *this_00;
  ulonglong uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int iVar8;
  undefined4 uVar9;
  int iVar10;
  undefined4 in_stack_ffffff90;
  undefined **ppuVar11;
  undefined4 in_stack_ffffff94;
  undefined4 in_stack_ffffff98;
  undefined4 in_stack_ffffff9c;
  undefined4 uVar12;
  undefined4 uStack_54;
  undefined4 *puStack_40;
  undefined4 uStack_3c;
  char ***pppcStack_38;
  undefined4 uStack_34;
  char ***apppcStack_30 [3];
  void *pvStack_24;
  undefined4 uStack_20;
  uint uStack_1c;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cc368;
  pvStack_c = ExceptionList;
  uStack_54 = 1;
  uVar12 = 0x447959;
  ExceptionList = &pvStack_c;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  puStack_8 = &stack0xffffff90;
  *(undefined4 *)((int)param_1 + 0x708) = 1;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0x3f000000,0x3f000000);
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xcc,0xf,0x18b,0x23a);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0x3f800000,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xd6,0x19,0x177,0x226);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f000000,0x3f000000,0x3f800000,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xe0,0x23,0x163,0x28);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f000000,0x3f19999a,0x3f800000,0x3f400000);
  Graphics_DrawScreenRect(param_1,0xe0,0x14a,0x163,0xe9);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0x3f400000);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0x878) + 0x330),400,0x113,in_stack_ffffff90,
             in_stack_ffffff94,in_stack_ffffff98,in_stack_ffffff9c,uVar12);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447ac5;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10 = 0;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10 = 0xffffffff;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 800),(byte *)"REGISTER HAMSTERBALL!",400,0x28,
             2,2,uVar4,uVar5,uVar6,uVar7,uVar9,in_stack_ffffff90,in_stack_ffffff94,in_stack_ffffff98
             ,in_stack_ffffff9c,uVar12);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447b3a;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10 = 1;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10 = 0xffffffff;
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)((int)this + 0x87c),0xe0
                    ,0x55,2,2,uVar4,uVar5,uVar6,uVar7,uVar9,in_stack_ffffff90,in_stack_ffffff94,
                    in_stack_ffffff98,in_stack_ffffff9c,uVar12);
  uVar9 = 0x3f800000;
  uVar7 = 0x3f000000;
  uVar6 = 0x3f800000;
  uVar5 = 0x3f000000;
  uVar4 = 0x447bac;
  Matrix_Scale4x4(&stack0xffffffb8,0x3f000000,0x3f800000,0x3f000000,0x3f800000);
  uStack_10 = 2;
  if (*(int *)((int)this + 0x864) == (int)this + 0x107c) {
    uVar7 = 0;
    uVar6 = 0x3f800000;
    uVar5 = 0x3f800000;
    uVar4 = 0x447bd8;
    iVar1 = Matrix_Scale4x4(&uStack_34,0x3f800000,0x3f800000,0,0x3f800000);
    unaff_EBX = *(undefined4 *)(iVar1 + 4);
    puStack_40 = *(undefined4 **)(iVar1 + 8);
    uStack_3c = *(undefined4 *)(iVar1 + 0xc);
    pppcStack_38 = *(char ****)(iVar1 + 0x10);
    uVar9 = 0x447bfd;
    Matrix_Identity(&uStack_34);
  }
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  puStack_8 = &stack0xffffff7c;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 800),(byte *)"CLICK HERE TO BUY!",400,0x113,2,
             2,&PTR_Vec3_dtor_004cf300,unaff_EBX,puStack_40,uStack_3c,pppcStack_38,uVar4,uVar5,uVar6
             ,uVar7,uVar9);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  iVar10 = 3;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  iVar8 = (int)uVar3;
  uVar3 = __ftol2();
  Graphics_DrawScreenRect(param_1,(int)uVar3,iVar8,iVar1,iVar10);
  puStack_8 = &stack0xffffff90;
  ppuVar11 = &PTR_Vec3_dtor_004cf300;
  iVar10 = 3;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  iVar8 = (int)uVar3;
  uVar3 = __ftol2();
  Graphics_DrawScreenRect(param_1,(int)uVar3,iVar8,iVar1,iVar10);
  *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x31c) + 0x428) = 0x3f4ccccd;
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447d54;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 3;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10._0_1_ = 2;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x31c),
             (byte *)"(IF YOU HAVE ALREADY BOUGHT HAMSTERBALL,",0x195,0x154,2,2,uVar4,uVar5,uVar6,
             uVar7,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,pppcStack_38);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447dc0;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 4;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10._0_1_ = 2;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x31c),
             (byte *)"TYPE YOUR CUSTOMER INFORMATION IN THE",0x195,0x163,2,2,uVar4,uVar5,uVar6,uVar7
             ,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,pppcStack_38);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447e2c;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 5;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10._0_1_ = 2;
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x31c),
             (byte *)"SPACES BELOW TO UNLOCK THE GAME!)",0x195,0x172,2,2,uVar4,uVar5,uVar6,uVar7,
             uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,pppcStack_38);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  *(undefined4 *)(*(int *)(*(int *)((int)this + 0x878) + 0x31c) + 0x428) = 0x3f800000;
  uVar4 = 0x447eae;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 6;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10._0_1_ = 2;
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"CUSTOMER NAME:",0xf9,
                    0x195,2,2,uVar4,uVar5,uVar6,uVar7,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,
                    pppcStack_38);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xef,0x1ae,0x145,0x19);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  if (*(uint *)((int)this + 0x1138) < 0x10) {
    pbVar2 = (byte *)((int)this + 0x1124);
  }
  else {
    pbVar2 = *(byte **)((int)this + 0x1124);
  }
  Font_DrawGlyph(*(void **)(*(int *)((int)this + 0x878) + 0x324),pbVar2,0xf2,0x1b0,ppuVar11,
                 unaff_EBX,puStack_40,uStack_3c,pppcStack_38);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x447fa2;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 7;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  uStack_10._0_1_ = 2;
  UI_DrawTextShadow(*(void **)(*(int *)((int)this + 0x878) + 0x31c),(byte *)"SERIAL NUMBER:",0xf9,
                    0x1c7,2,2,uVar4,uVar5,uVar6,uVar7,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,
                    pppcStack_38);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xef,0x1e0,0x145,0x19);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  if (*(uint *)((int)this + 0x1154) < 0x10) {
    pbVar2 = (byte *)((int)this + 0x1140);
  }
  else {
    pbVar2 = *(byte **)((int)this + 0x1140);
  }
  Font_DrawGlyph(*(void **)(*(int *)((int)this + 0x878) + 0x324),pbVar2,0xf2,0x1e2,ppuVar11,
                 unaff_EBX,puStack_40,uStack_3c,pppcStack_38);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  Graphics_DrawScreenRect(param_1,599,0x23,10,0x226);
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  Graphics_DrawScreenRect(param_1,0xe0,0x249,0x181,10);
  iVar1 = *(int *)((int)this + 0x1108);
  if ((iVar1 != 0) && (*(char *)((int)this + 0x1110) != '\0')) {
    this_00 = (undefined1 *)((int)this + 0x1120);
    if (iVar1 != (int)this + 0x1098) {
      this_00 = puStack_8;
    }
    if (iVar1 == (int)this + 0x10b4) {
      this_00 = (undefined1 *)((int)this + 0x113c);
    }
    puStack_40 = &uStack_34;
    unaff_EBX = 0x448129;
    StdString_SubstrInit(this_00,puStack_40,0,*(uint *)((int)this + 0x1158));
    uStack_10._0_1_ = 8;
    pppcStack_38 = apppcStack_30[0];
    if (uStack_1c < 0x10) {
      pppcStack_38 = (char ***)apppcStack_30;
    }
    uStack_3c = 0x44814f;
    uVar3 = Font_MeasureText((char *)pppcStack_38);
    puStack_8 = (undefined1 *)uVar3;
    Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    iVar10 = 0x14;
    iVar8 = 5;
    uVar3 = __ftol2();
    iVar1 = (int)uVar3;
    uVar3 = __ftol2();
    Graphics_DrawScreenRect(param_1,(int)uVar3,iVar1,iVar8,iVar10);
    uStack_10._0_1_ = 2;
    if (0xf < uStack_1c) {
      uStack_3c = 0x4481c3;
      pppcStack_38 = apppcStack_30[0];
      _free(apppcStack_30[0]);
    }
    uStack_1c = 0xf;
    uStack_20 = 0;
    apppcStack_30[0] = (char ***)((uint)apppcStack_30[0] & 0xffffff00);
  }
  puStack_8 = &stack0xffffff90;
  if (*(int *)((int)this + 0x864) == (int)this + 0x10d0) {
    uVar4 = 0x3f800000;
  }
  else {
    uVar4 = 0;
  }
  Matrix_Scale4x4(&stack0xffffff90,uVar4,0x3f800000,0,0x3f800000);
  puStack_8 = &stack0xffffff7c;
  UI_DrawRectAndReset(param_1);
  puStack_8 = &stack0xffffff90;
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x448255;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 9;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar10 = 2;
  uStack_10._0_1_ = 2;
  iVar8 = 2;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 800),(byte *)"UNLOCK!",(int)uVar3,iVar1,iVar8,
             iVar10,uVar4,uVar5,uVar6,uVar7,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,
             pppcStack_38);
  puStack_8 = &stack0xffffff90;
  if (*(int *)((int)this + 0x864) == (int)this + 0x10ec) {
    uVar4 = 0x3f800000;
  }
  else {
    uVar4 = 0;
  }
  Matrix_Scale4x4(&stack0xffffff90,0x3f800000,0,uVar4,0x3f800000);
  puStack_8 = &stack0xffffff7c;
  UI_DrawRectAndReset(param_1);
  uVar9 = 0x3f800000;
  uVar7 = 0;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0x44833f;
  puStack_8 = &stack0xffffff90;
  Matrix_Scale4x4(&stack0xffffff90,0,0,0,0x3f800000);
  uStack_10._0_1_ = 10;
  Matrix_Scale4x4(&stack0xffffff7c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  iVar10 = 2;
  uStack_10 = CONCAT31(uStack_10._1_3_,2);
  iVar8 = 2;
  uVar3 = __ftol2();
  iVar1 = (int)uVar3;
  uVar3 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 800),(byte *)0x4d3ac8,(int)uVar3,iVar1,iVar8,
             iVar10,uVar4,uVar5,uVar6,uVar7,uVar9,ppuVar11,unaff_EBX,puStack_40,uStack_3c,
             pppcStack_38);
  if (*(char *)((int)param_1 + 0x7d2) == '\0') {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  else {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  *(undefined4 *)((int)param_1 + 0x708) = 3;
  uStack_1c = 0xffffffff;
  Matrix_Identity(&uStack_54);
  ExceptionList = pvStack_24;
  return;
}

