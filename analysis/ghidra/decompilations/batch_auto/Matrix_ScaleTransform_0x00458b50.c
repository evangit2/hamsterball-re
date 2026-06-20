
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
Matrix_ScaleTransform
          (void *this,undefined4 param_1,float param_2,float param_3,float param_4,float param_5)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 *puVar7;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_004ccda8;
  local_c = ExceptionList;
  local_4 = 0;
  ExceptionList = &local_c;
  puVar5 = operator_new(0x50);
  if (puVar5 == (undefined4 *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    *puVar5 = &PTR_LAB_004d8e68;
    puVar5[0x12] = 0;
    puVar7 = puVar5;
    for (iVar6 = 0x11; puVar7 = puVar7 + 1, iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar7 = 0;
    }
    *(undefined1 *)(puVar5 + 0x13) = 0;
    *(undefined1 *)((int)puVar5 + 0x4d) = 0;
  }
  fVar4 = param_5 * *(float *)((int)this + 0x10);
  fVar1 = *(float *)((int)this + 0xc);
  fVar2 = *(float *)((int)this + 8);
  puVar5[1] = param_2 * *(float *)((int)this + 4);
  puVar5[2] = param_3 * fVar2;
  puVar5[3] = param_4 * fVar1;
  fVar1 = (float)_DAT_004cf3c8;
  puVar5[4] = fVar4;
  *(bool *)(puVar5 + 0x13) = fVar4 != fVar1;
  fVar1 = *(float *)((int)this + 0x20);
  fVar2 = *(float *)((int)this + 0x1c);
  fVar3 = *(float *)((int)this + 0x18);
  puVar5[5] = param_2 * *(float *)((int)this + 0x14);
  puVar5[6] = param_3 * fVar3;
  puVar5[7] = param_4 * fVar2;
  puVar5[8] = param_5 * fVar1;
  *(bool *)(puVar5 + 0x13) = fVar4 != (float)_DAT_004cf3c8;
  fVar1 = *(float *)((int)this + 0x40);
  fVar2 = *(float *)((int)this + 0x3c);
  fVar3 = *(float *)((int)this + 0x38);
  puVar5[0xd] = param_2 * *(float *)((int)this + 0x34);
  puVar5[0xe] = param_3 * fVar3;
  puVar5[0xf] = param_4 * fVar2;
  puVar5[0x10] = param_5 * fVar1;
  *(bool *)(puVar5 + 0x13) = fVar4 != (float)_DAT_004cf3c8;
  fVar1 = *(float *)((int)this + 0x30);
  fVar2 = *(float *)((int)this + 0x2c);
  fVar3 = *(float *)((int)this + 0x28);
  puVar5[9] = param_2 * *(float *)((int)this + 0x24);
  puVar5[10] = param_3 * fVar3;
  puVar5[0xb] = param_4 * fVar2;
  puVar5[0xc] = param_5 * fVar1;
  *(bool *)(puVar5 + 0x13) = fVar4 != (float)_DAT_004cf3c8;
  puVar5[0x12] = *(undefined4 *)((int)this + 0x48);
  *(undefined1 *)((int)puVar5 + 0x4d) = *(undefined1 *)((int)this + 0x4d);
  puVar5[0x11] = *(undefined4 *)((int)this + 0x44);
  local_4 = 0xffffffff;
  Matrix_Identity(&param_1);
  ExceptionList = local_c;
  return puVar5;
}

