
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall Vertex_Transform(void *this,int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 *puVar4;
  float *pfVar5;
  int iVar6;
  uint uVar7;
  
  fVar3 = _DAT_004cf48c;
  fVar2 = _DAT_004cf3f0;
  fVar1 = _DAT_004cf310;
  iVar6 = *(int *)((int)this + 8);
  if (iVar6 == 1) {
    if (*(int *)((int)this + 0x1048) == 2) {
      uVar7 = 0;
      if (*(int *)((int)this + 0x1058) != 0) {
        pfVar5 = (float *)(param_1 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)((int)this + 0x104c)) = (pfVar5[-2] + fVar1) * fVar2;
          *(float *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = (pfVar5[-1] + fVar1) * fVar2;
          *(float *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = (*pfVar5 + fVar1) * fVar2;
          *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) = pfVar5[1];
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)((int)this + 0x1058));
      }
    }
    else if ((*(int *)((int)this + 0x1048) == 3) && (uVar7 = 0, *(int *)((int)this + 0x1058) != 0))
    {
      pfVar5 = (float *)(param_1 + 8);
      iVar6 = 0;
      do {
        uVar7 = uVar7 + 1;
        *(float *)(iVar6 + *(int *)((int)this + 0x104c)) = (pfVar5[-2] + fVar1) * fVar2;
        *(float *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = (pfVar5[-1] + fVar1) * fVar2;
        *(float *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = (*pfVar5 + fVar1) * fVar2;
        *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) = (pfVar5[1] + fVar1) * fVar2;
        pfVar5 = pfVar5 + 4;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)((int)this + 0x1058));
    }
  }
  else if (iVar6 == 2) {
    if (*(int *)((int)this + 0x1048) == 1) {
      uVar7 = 0;
      if (*(int *)((int)this + 0x1058) != 0) {
        pfVar5 = (float *)(param_1 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)((int)this + 0x104c)) = pfVar5[-2] * fVar3 - fVar1;
          *(float *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = pfVar5[-1] * fVar3 - fVar1;
          *(float *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = *pfVar5 * fVar3 - fVar1;
          *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) = pfVar5[1];
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)((int)this + 0x1058));
      }
    }
    else if ((*(int *)((int)this + 0x1048) == 3) && (uVar7 = 0, *(int *)((int)this + 0x1058) != 0))
    {
      puVar4 = (undefined4 *)(param_1 + 8);
      iVar6 = 0;
      do {
        *(undefined4 *)(iVar6 + *(int *)((int)this + 0x104c)) = puVar4[-2];
        uVar7 = uVar7 + 1;
        *(undefined4 *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = puVar4[-1];
        *(undefined4 *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = *puVar4;
        pfVar5 = (float *)(puVar4 + 1);
        puVar4 = puVar4 + 4;
        *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) =
             (*pfVar5 + _DAT_004cf310) * _DAT_004cf3f0;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)((int)this + 0x1058));
    }
  }
  else if (iVar6 == 3) {
    if (*(int *)((int)this + 0x1048) == 1) {
      uVar7 = 0;
      if (*(int *)((int)this + 0x1058) != 0) {
        pfVar5 = (float *)(param_1 + 8);
        iVar6 = 0;
        do {
          uVar7 = uVar7 + 1;
          *(float *)(iVar6 + *(int *)((int)this + 0x104c)) = pfVar5[-2] * fVar3 - fVar1;
          *(float *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = pfVar5[-1] * fVar3 - fVar1;
          *(float *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = *pfVar5 * fVar3 - fVar1;
          *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) = pfVar5[1] * fVar3 - fVar1;
          pfVar5 = pfVar5 + 4;
          iVar6 = iVar6 + 0x10;
        } while (uVar7 < *(uint *)((int)this + 0x1058));
      }
    }
    else if ((*(int *)((int)this + 0x1048) == 2) && (uVar7 = 0, *(int *)((int)this + 0x1058) != 0))
    {
      puVar4 = (undefined4 *)(param_1 + 8);
      iVar6 = 0;
      do {
        *(undefined4 *)(iVar6 + *(int *)((int)this + 0x104c)) = puVar4[-2];
        uVar7 = uVar7 + 1;
        *(undefined4 *)(iVar6 + 4 + *(int *)((int)this + 0x104c)) = puVar4[-1];
        *(undefined4 *)(iVar6 + 8 + *(int *)((int)this + 0x104c)) = *puVar4;
        pfVar5 = (float *)(puVar4 + 1);
        puVar4 = puVar4 + 4;
        *(float *)(iVar6 + 0xc + *(int *)((int)this + 0x104c)) = (*pfVar5 + *pfVar5) - _DAT_004cf310
        ;
        iVar6 = iVar6 + 0x10;
      } while (uVar7 < *(uint *)((int)this + 0x1058));
    }
  }
  return *(undefined4 *)((int)this + 0x104c);
}

