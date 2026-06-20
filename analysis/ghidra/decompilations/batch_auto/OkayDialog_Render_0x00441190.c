
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall OkayDialog_Render(void *this,void *param_1)

{
  float fVar1;
  float fVar2;
  void *this_00;
  int iVar3;
  ulonglong uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 in_stack_ffffffb0;
  undefined4 in_stack_ffffffb4;
  undefined4 in_stack_ffffffb8;
  undefined4 in_stack_ffffffbc;
  undefined4 uVar12;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cbe20;
  pvStack_c = ExceptionList;
  uVar12 = 0x4411c4;
  ExceptionList = &pvStack_c;
  (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  puStack_8 = &stack0xffffffb0;
  *(undefined4 *)((int)param_1 + 0x708) = 1;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0,0x3f400000);
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  puStack_8 = &stack0xffffffb0;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0x3f800000,0x3f000000);
  Graphics_DrawRectAndReset
            (param_1,this,in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,
             uVar12);
  puStack_8 = &stack0xffffffb0;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0x3f800000,0x3f800000);
  puStack_8 = &stack0xffffff9c;
  UI_DrawRectAndReset(param_1);
  puStack_8 = &stack0xffffffb0;
  Matrix_Scale4x4(&stack0xffffffb0,0,0x3f800000,0x3f800000,0x3f000000);
  puStack_8 = &stack0xffffff9c;
  UI_DrawRectAndReset(param_1);
  puStack_8 = &stack0xffffffb0;
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x4412fb;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xffffff9c,0x3f800000,0x3f800000,0,0x3f800000);
  iVar6 = 5;
  iVar5 = 5;
  uVar4 = __ftol2();
  iVar3 = (int)uVar4;
  uVar4 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x318),*(byte **)((int)this + 0x880),
             (int)uVar4,iVar3,iVar5,iVar6,uVar7,uVar8,uVar9,uVar10,uVar11,in_stack_ffffffb0,
             in_stack_ffffffb4,in_stack_ffffffb8,in_stack_ffffffbc,uVar12);
  puStack_8 = &stack0xffffffb0;
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x44137f;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xffffff9c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  this_00 = *(void **)(*(int *)((int)this + 0x878) + 0x318);
  iVar6 = 5;
  iVar5 = 5;
  uVar4 = __ftol2();
  iVar3 = (int)uVar4;
  uVar4 = __ftol2();
  UI_DrawTextShadow(this_00,*(byte **)((int)this + 0x884),(int)uVar4,iVar3,iVar5,iVar6,uVar7,uVar8,
                    uVar9,uVar10,uVar11,in_stack_ffffffb0,in_stack_ffffffb4,in_stack_ffffffb8,
                    in_stack_ffffffbc,uVar12);
  fVar1 = *(float *)((int)this + 0x890);
  puStack_8 = &stack0xffffffb0;
  fVar2 = *(float *)((int)this + 8);
  if (*(int *)((int)this + 0x864) == (int)this + 0x888) {
    uVar10 = 0x3f800000;
    uVar9 = 0x3f800000;
    uVar8 = 0x3f000000;
    uVar7 = 0x3f000000;
  }
  else {
    uVar10 = 0x3f400000;
    uVar9 = 0x3f000000;
    uVar8 = 0;
    uVar7 = 0;
  }
  Matrix_Scale4x4(&stack0xffffffb0,uVar7,uVar8,uVar9,uVar10);
  puStack_8 = &stack0xffffff9c;
  UI_DrawRectAndReset(param_1);
  puStack_8 = &stack0xffffffb0;
  uVar11 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  uVar8 = 0;
  uVar7 = 0x4414b0;
  Matrix_Scale4x4(&stack0xffffffb0,0,0,0,0x3f800000);
  Matrix_Scale4x4(&stack0xffffff9c,0x3f400000,0x3f800000,0x3f400000,0x3f800000);
  iVar6 = 5;
  iVar5 = 5;
  uVar4 = __ftol2();
  iVar3 = (int)uVar4;
  uVar4 = __ftol2();
  UI_DrawTextCenteredAbsolute
            (*(void **)(*(int *)((int)this + 0x878) + 0x318),(byte *)"OKAY!",(int)uVar4,iVar3,iVar5,
             iVar6,uVar7,uVar8,uVar9,uVar10,uVar11,in_stack_ffffffb0,in_stack_ffffffb4,
             in_stack_ffffffb8,in_stack_ffffffbc,uVar12);
  if (*(char *)((int)param_1 + 0x7d2) == '\0') {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  else {
    (**(code **)(**(int **)((int)param_1 + 0x154) + 200))();
  }
  *(undefined4 *)((int)param_1 + 0x708) = 3;
  ExceptionList = (void *)(fVar1 + fVar2);
  return;
}

