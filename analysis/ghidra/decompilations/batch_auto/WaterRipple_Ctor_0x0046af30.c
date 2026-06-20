
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall WaterRipple_Ctor(void *this,undefined4 param_1)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  float local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd4fb;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_WaterRipple_DeletingDtor_004d9344;
  RenderContext_Init((undefined4 *)((int)this + 0x30));
  uVar6 = 0x3f800000;
  uVar5 = 0x3f800000;
  uVar4 = 0x3f800000;
  local_4 = 0;
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 0x84) = 0;
  *(undefined4 *)((int)this + 0x10) = 0x42c80000;
  *(undefined4 *)((int)this + 0x14) = 0x42960000;
  *(undefined4 *)((int)this + 8) = 10;
  *(undefined4 *)((int)this + 0x20) = 0x3fc00000;
  *(undefined4 *)((int)this + 0x1c) = 0x41a00000;
  uVar3 = 0x46afb7;
  Matrix_Scale4x4(&local_20,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  *(float *)((int)this + 0x40) = local_10;
  fVar1 = (float)_DAT_004cf3c8;
  *(undefined4 *)((int)this + 0x3c) = local_14;
  *(undefined4 *)((int)this + 0x34) = local_1c;
  *(undefined4 *)((int)this + 0x38) = local_18;
  *(bool *)((int)this + 0x7c) = local_10 != fVar1;
  uVar7 = 0x46aff5;
  Matrix_Identity(&local_20);
  Matrix_Scale4x4(&stack0xffffffbc,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  SceneObj_SetBounds((undefined4 *)((int)this + 0x30),uVar3,uVar4,uVar5,uVar6,uVar7);
  *(undefined4 *)((int)this + 0x78) = 0;
  WaterRipple_AllocBuffers((int)this);
  *(undefined4 *)((int)this + 0x24) = 0;
  iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,0x168,'\0');
  *(undefined1 *)((int)this + 0x80) = 0;
  *(undefined4 *)((int)this + 0x2c) = 0xffffffff;
  *(float *)((int)this + 0x28) = (float)iVar2;
  ExceptionList = local_c;
  return this;
}

