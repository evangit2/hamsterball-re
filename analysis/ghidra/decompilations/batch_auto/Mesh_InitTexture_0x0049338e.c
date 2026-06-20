
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void * __thiscall Mesh_InitTexture(void *this,undefined4 *param_1,uint param_2,undefined4 param_3)

{
  float fVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined *puVar4;
  float *pfVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined4 *puVar9;
  
  *(undefined ***)this = &PTR_D3DXObject_ScalarDtor_004dc044;
  RepeatCall((int)this + 0x30,0x10,0x100,&LAB_0047d949);
  *(undefined4 *)((int)this + 0x18) = *param_1;
  *(undefined4 *)((int)this + 4) = param_1[1];
  *(undefined4 *)((int)this + 0x1050) = param_1[2];
  *(undefined4 *)((int)this + 0x1054) = param_1[3];
  puVar2 = param_1 + 10;
  puVar9 = (undefined4 *)((int)this + 0x1030);
  for (iVar3 = 6; fVar1 = _DAT_004db358, iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar9 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar9 = puVar9 + 1;
  }
  puVar4 = &DAT_004fa6c0;
  if (param_1[0x10] == 0) {
    puVar4 = &DAT_004fa640;
  }
  *(undefined **)((int)this + 0x2c) = puVar4;
  *(uint *)((int)this + 0x1068) = param_2 >> 3;
  *(undefined4 *)((int)this + 0x1048) = 0;
  *(undefined4 *)((int)this + 0x104c) = 0;
  *(uint *)((int)this + 0xc) = (uint)(param_2 != 0);
  *(undefined4 *)((int)this + 8) = param_3;
  iVar3 = param_1[0x11];
  *(int *)((int)this + 0x10) = iVar3;
  if (iVar3 != 0) {
    uVar7 = param_1[0x11];
    *(float *)((int)this + 0x1c) = (float)(uVar7 >> 0x10 & 0xff) * fVar1;
    *(float *)((int)this + 0x20) = (float)(uVar7 >> 8 & 0xff) * fVar1;
    *(float *)((int)this + 0x24) = (float)(uVar7 & 0xff) * fVar1;
    *(float *)((int)this + 0x28) = (float)(uVar7 >> 0x18) * fVar1;
  }
  if ((*(int *)((int)this + 4) == 0x29) || (*(int *)((int)this + 4) == 0x28)) {
    *(undefined4 *)((int)this + 0x14) = 1;
    if (param_1[0x12] == 0) {
      iVar3 = 0x100;
      puVar2 = (undefined4 *)((int)this + 0x38);
      do {
        puVar2[1] = 0x3f800000;
        *puVar2 = 0x3f800000;
        iVar3 = iVar3 + -1;
        puVar2[-1] = 0x3f800000;
        puVar2[-2] = 0x3f800000;
        puVar2 = puVar2 + 4;
      } while (iVar3 != 0);
    }
    else {
      pfVar5 = (float *)((int)this + 0x34);
      uVar7 = 0;
      do {
        uVar8 = uVar7 + 4;
        pfVar5[-1] = (float)*(byte *)(uVar7 + param_1[0x12]) * fVar1;
        *pfVar5 = (float)*(byte *)(uVar7 + 1 + param_1[0x12]) * fVar1;
        pfVar5[1] = (float)*(byte *)(uVar7 + 2 + param_1[0x12]) * fVar1;
        pfVar5[2] = (float)*(byte *)(uVar7 + 3 + param_1[0x12]) * fVar1;
        pfVar5 = pfVar5 + 4;
        uVar7 = uVar8;
      } while (uVar8 < 0x400);
    }
  }
  else {
    *(undefined4 *)((int)this + 0x14) = 0;
  }
  *(int *)((int)this + 0x105c) = *(int *)((int)this + 0x103c) - *(int *)((int)this + 0x1034);
  iVar3 = *(int *)((int)this + 0x1030);
  iVar6 = *(int *)((int)this + 0x1038) - iVar3;
  *(int *)((int)this + 0x1060) = *(int *)((int)this + 0x1044) - *(int *)((int)this + 0x1040);
  *(int *)((int)this + 0x1058) = iVar6;
  *(int *)((int)this + 0x1064) = *(int *)((int)this + 0x1068) * iVar6;
  if (*(int *)((int)this + 0xc) != 0) {
    *(int *)((int)this + 0x18) =
         *(int *)((int)this + 0x18) +
         *(int *)((int)this + 0x1068) * iVar3 +
         *(int *)((int)this + 0x1050) * *(int *)((int)this + 0x1034) +
         *(int *)((int)this + 0x1054) * *(int *)((int)this + 0x1040);
    *(undefined4 *)((int)this + 0x1038) = *(undefined4 *)((int)this + 0x1058);
    *(undefined4 *)((int)this + 0x103c) = *(undefined4 *)((int)this + 0x105c);
    *(int *)((int)this + 0x1030) = 0;
    *(undefined4 *)((int)this + 0x1034) = 0;
    *(undefined4 *)((int)this + 0x1040) = 0;
    *(undefined4 *)((int)this + 0x1044) = *(undefined4 *)((int)this + 0x1060);
  }
  return this;
}

