
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall HighScoreEntry_DeletingDtor(void *this,void *param_1)

{
  int iVar1;
  undefined **ppuVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc848;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  Matrix_Scale4x4(&stack0xffffffac,0,0,0x3f800000,0x3f400000);
  Graphics_DrawScreenRect(param_1,*(int *)((int)this + 0x1c),0x103,800,0x41);
  Matrix_Scale4x4(&local_34,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  local_4 = 0;
  if (*(int *)(*(int *)((int)this + 8) + 0x14) == 1) {
    iVar1 = Matrix_Scale4x4(local_20,0,0x3f000000,0x3f800000,0x3f800000);
    local_30 = *(undefined4 *)(iVar1 + 4);
    local_2c = *(undefined4 *)(iVar1 + 8);
    local_28 = *(undefined4 *)(iVar1 + 0xc);
    local_24 = *(undefined4 *)(iVar1 + 0x10);
    Matrix_Identity(local_20);
  }
  if (*(int *)(*(int *)((int)this + 8) + 0x14) == 2) {
    iVar1 = Matrix_Scale4x4(local_20,0x3f800000,0x3e800000,0x3e800000,0x3f800000);
    local_30 = *(undefined4 *)(iVar1 + 4);
    local_2c = *(undefined4 *)(iVar1 + 8);
    local_28 = *(undefined4 *)(iVar1 + 0xc);
    local_24 = *(undefined4 *)(iVar1 + 0x10);
    Matrix_Identity(local_20);
  }
  if (*(int *)(*(int *)((int)this + 8) + 0x14) == 3) {
    iVar1 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0,0x3f800000);
    local_30 = *(undefined4 *)(iVar1 + 4);
    local_2c = *(undefined4 *)(iVar1 + 8);
    local_28 = *(undefined4 *)(iVar1 + 0xc);
    local_24 = *(undefined4 *)(iVar1 + 0x10);
    Matrix_Identity(local_20);
  }
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0xc) + 0x330),400,300,&PTR_Vec3_dtor_004cf300,local_30,
             local_2c,local_28,local_24);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0xc) + 0x330),400,300,&PTR_Vec3_dtor_004cf300,local_30,
             local_2c,local_28,local_24);
  ppuVar2 = &PTR_Vec3_dtor_004cf300;
  uVar3 = local_30;
  uVar4 = local_2c;
  uVar5 = local_28;
  uVar6 = local_24;
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0xc) + 0x330),400,300,&PTR_Vec3_dtor_004cf300,local_30,
             local_2c,local_28,local_24);
  Matrix_Scale4x4(&stack0xffffffac,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0xc) + 0x408),0xa5 - *(int *)((int)this + 0x1c),0x122,
             ppuVar2,uVar3,uVar4,uVar5,uVar6);
  Matrix_Scale4x4(&stack0xffffffac,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  SceneObject_InitAtPosition
            (*(void **)(*(int *)((int)this + 0xc) + 0x408),*(int *)((int)this + 0x1c) + 0x27b,0x122,
             ppuVar2,uVar3,uVar4,uVar5,uVar6);
  if (_DAT_004cf368 < *(float *)((int)this + 0x20)) {
    Matrix_Scale4x4(&stack0xffffffac,0x3f800000,0x3f800000,0x3f800000,
                    *(undefined4 *)((int)this + 0x20));
    Graphics_DrawScreenRect(param_1,0,0,800,600);
  }
  local_4 = 0xffffffff;
  Matrix_Identity(&local_34);
  ExceptionList = local_c;
  return;
}

