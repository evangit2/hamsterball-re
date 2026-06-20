
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* SceneObject_ctor - constructs base scene object. Size ~0xD4. Fields: vtable@0 (0x4D934C),
   param_1@4, field_8-1C=0, visible@88=1, render_state@8C=-1, scale_vec@90, rotation_vec@A4,
   position_vec@B8, sqrt_val@CC, obj_type@D0=3 */

void * __thiscall SceneObject_ctor(void *this,undefined4 param_1)

{
  float fVar1;
  int iVar2;
  undefined4 local_20 [5];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd5ba;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined ***)this = &PTR_SceneObject_dtor_004d934c;
  Vec3_Init((undefined4 *)((int)this + 0x90));
  local_4 = 0;
  Vec3_Init((undefined4 *)((int)this + 0xa4));
  local_4._0_1_ = 1;
  Vec3_Init((undefined4 *)((int)this + 0xb8));
  *(undefined4 *)((int)this + 4) = param_1;
  local_4 = CONCAT31(local_4._1_3_,2);
  *(undefined4 *)((int)this + 8) = 0;
  *(undefined4 *)((int)this + 0xc) = 0;
  *(undefined4 *)((int)this + 0x10) = 0;
  *(undefined4 *)((int)this + 0x14) = 0;
  *(undefined4 *)((int)this + 0x18) = 0;
  *(undefined4 *)((int)this + 0x1c) = 0;
  *(undefined1 *)((int)this + 0x88) = 1;
  *(undefined4 *)((int)this + 0x8c) = 0xffffffff;
  iVar2 = Matrix_Scale4x4(local_20,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  *(undefined4 *)((int)this + 0x94) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)((int)this + 0x98) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)((int)this + 0x9c) = *(undefined4 *)(iVar2 + 0xc);
  *(undefined4 *)((int)this + 0xa0) = *(undefined4 *)(iVar2 + 0x10);
  Matrix_Identity(local_20);
  iVar2 = Matrix_Scale4x4(local_20,0,0,0,0);
  *(undefined4 *)((int)this + 0xa8) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)((int)this + 0xac) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)((int)this + 0xb0) = *(undefined4 *)(iVar2 + 0xc);
  *(undefined4 *)((int)this + 0xb4) = *(undefined4 *)(iVar2 + 0x10);
  Matrix_Identity(local_20);
  iVar2 = Matrix_Scale4x4(local_20,0,0,0,0);
  *(undefined4 *)((int)this + 0xbc) = *(undefined4 *)(iVar2 + 4);
  *(undefined4 *)((int)this + 0xc0) = *(undefined4 *)(iVar2 + 8);
  *(undefined4 *)((int)this + 0xc4) = *(undefined4 *)(iVar2 + 0xc);
  *(undefined4 *)((int)this + 200) = *(undefined4 *)(iVar2 + 0x10);
  Matrix_Identity(local_20);
  fVar1 = (float)_DAT_004d9360;
  *(undefined4 *)((int)this + 0xd0) = 3;
  *(float *)((int)this + 0xcc) = SQRT(fVar1);
  ExceptionList = local_c;
  return this;
}

