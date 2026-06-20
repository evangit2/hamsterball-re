
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall D3DXMesh_ComputeLighting565(void *this,uint param_1,uint param_2,uint param_3)

{
  undefined2 uVar1;
  ushort uVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  int iVar13;
  float *pfVar14;
  ushort *puVar15;
  
  if (*(int *)((int)this + 0x1090) == 0) {
    iVar13 = -0x7ff8fff2;
  }
  else {
    if ((((param_1 < *(uint *)((int)this + 0x1074)) || (*(uint *)((int)this + 0x107c) <= param_1))
        || (param_2 < *(uint *)((int)this + 0x1080))) || (*(uint *)((int)this + 0x1084) <= param_2))
    {
      iVar13 = D3DXMesh_ComputeLightingFromNormals((int)this);
      if (iVar13 < 0) {
        return iVar13;
      }
      *(uint *)((int)this + 0x107c) = param_1 + 1;
      *(uint *)((int)this + 0x1074) = param_1;
      *(uint *)((int)this + 0x1080) = param_2;
      *(uint *)((int)this + 0x1084) = param_2 + 1;
      fVar12 = _DAT_004dc07c;
      fVar11 = _DAT_004cf368;
      fVar10 = _DAT_004cf310;
      if (param_3 != 0) {
        param_3 = *(uint *)((int)this + 0x1070);
        pfVar14 = *(float **)((int)this + 0x106c);
        puVar15 = (ushort *)
                  (*(int *)((int)this + 0x1050) * param_1 + *(int *)((int)this + 0x1054) * param_2 +
                   param_3 * 2 + *(int *)((int)this + 0x18));
        if (param_3 < *(uint *)((int)this + 0x1078)) {
          do {
            uVar1 = *(undefined2 *)((int)this + 0x1094);
            uVar2 = puVar15[1];
            fVar8 = (float)(*puVar15 >> ((byte)*(undefined2 *)((int)this + 0x1098) & 0x1f) & 0xff) -
                    _DAT_004dc078;
            fVar5 = (float)(uVar2 >> ((byte)*(undefined2 *)((int)this + 0x1098) & 0x1f) & 0xff) -
                    _DAT_004dc078;
            fVar6 = fVar5 * _DAT_004dc14c;
            fVar4 = ((float)(*puVar15 >> ((byte)*(undefined2 *)((int)this + 0x1094) & 0x1f) & 0xff)
                    - fVar12) * _DAT_004dc148;
            fVar7 = fVar4 + fVar6;
            *pfVar14 = fVar7;
            fVar9 = fVar8 * _DAT_004dc144;
            fVar5 = fVar5 * _DAT_004dc140;
            fVar3 = (fVar4 - fVar9) - fVar5;
            pfVar14[1] = fVar3;
            fVar8 = fVar8 * _DAT_004dc13c;
            fVar4 = fVar8 + fVar4;
            pfVar14[2] = fVar4;
            pfVar14[3] = 1.0;
            if (fVar11 <= fVar7) {
              if (fVar10 < fVar7) {
                fVar7 = 1.0;
              }
            }
            else {
              fVar7 = 0.0;
            }
            *pfVar14 = fVar7;
            if (fVar11 <= fVar3) {
              if (fVar10 < fVar3) {
                fVar3 = 1.0;
              }
            }
            else {
              fVar3 = 0.0;
            }
            pfVar14[1] = fVar3;
            if (fVar11 <= fVar4) {
              if (fVar10 < fVar4) {
                fVar4 = 1.0;
              }
            }
            else {
              fVar4 = 0.0;
            }
            pfVar14[2] = fVar4;
            fVar3 = ((float)(uVar2 >> ((byte)uVar1 & 0x1f) & 0xff) - fVar12) * _DAT_004dc148;
            fVar6 = fVar3 + fVar6;
            pfVar14[4] = fVar6;
            fVar5 = (fVar3 - fVar9) - fVar5;
            pfVar14[5] = fVar5;
            fVar3 = fVar3 + fVar8;
            pfVar14[6] = fVar3;
            pfVar14[7] = 1.0;
            if (fVar11 <= fVar6) {
              if (fVar10 < fVar6) {
                fVar6 = 1.0;
              }
            }
            else {
              fVar6 = 0.0;
            }
            pfVar14[4] = fVar6;
            if (fVar11 <= fVar5) {
              if (fVar10 < fVar5) {
                fVar5 = 1.0;
              }
            }
            else {
              fVar5 = 0.0;
            }
            pfVar14[5] = fVar5;
            if (fVar11 <= fVar3) {
              if (fVar10 < fVar3) {
                fVar3 = 1.0;
              }
            }
            else {
              fVar3 = 0.0;
            }
            param_3 = param_3 + 2;
            pfVar14[6] = fVar3;
            puVar15 = puVar15 + 2;
            pfVar14 = pfVar14 + 8;
          } while (param_3 < *(uint *)((int)this + 0x1078));
        }
      }
    }
    iVar13 = 0;
  }
  return iVar13;
}

