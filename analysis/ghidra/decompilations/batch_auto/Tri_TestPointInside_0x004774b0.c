
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __cdecl Tri_TestPointInside(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float *pfVar7;
  uint uVar8;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c [3];
  
  local_24 = *param_4 - *param_2;
  local_20 = param_4[1] - param_2[1];
  local_1c = param_4[2] - param_2[2];
  fVar1 = *param_3;
  fVar2 = *param_2;
  fVar3 = param_3[1];
  fVar4 = param_2[1];
  fVar5 = param_3[2];
  fVar6 = param_2[2];
  local_18 = fVar1 - fVar2;
  local_14 = fVar3 - fVar4;
  local_10 = fVar5 - fVar6;
  Vec3_CrossProduct(&local_18,local_c,&local_24);
  local_18 = fVar1 - fVar2;
  local_14 = fVar3 - fVar4;
  local_10 = fVar5 - fVar6;
  pfVar7 = (float *)Vec3_CrossProduct(local_c,&local_24,&local_18);
  fVar2 = -_DAT_004d8e08;
  fVar1 = (param_1[1] * pfVar7[1] + param_1[2] * pfVar7[2] + *pfVar7 * *param_1) -
          (pfVar7[1] * param_2[1] + pfVar7[2] * param_2[2] + *pfVar7 * *param_2);
  uVar8 = CONCAT22((short)((uint)pfVar7 >> 0x10),
                   (ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                   (ushort)(fVar1 == fVar2) << 0xe);
  if (fVar1 >= fVar2) {
    local_18 = *param_4 - *param_3;
    local_14 = param_4[1] - param_3[1];
    local_10 = param_4[2] - param_3[2];
    pfVar7 = (float *)Vec3_CrossProduct(local_c,&local_24,&local_18);
    fVar1 = (param_1[1] * pfVar7[1] + *param_1 * *pfVar7 + param_1[2] * pfVar7[2]) -
            (pfVar7[1] * param_3[1] + *param_3 * *pfVar7 + param_3[2] * pfVar7[2]);
    uVar8 = CONCAT22((short)((uint)pfVar7 >> 0x10),
                     (ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                     (ushort)(fVar1 == fVar2) << 0xe);
    if (fVar1 >= fVar2) {
      local_18 = *param_2 - *param_4;
      local_14 = param_2[1] - param_4[1];
      local_10 = param_2[2] - param_4[2];
      pfVar7 = (float *)Vec3_CrossProduct(local_c,&local_24,&local_18);
      fVar1 = (param_1[1] * pfVar7[1] + param_1[2] * pfVar7[2] + *param_1 * *pfVar7) -
              (*pfVar7 * *param_4 + pfVar7[1] * param_4[1] + pfVar7[2] * param_4[2]);
      uVar8 = CONCAT22((short)((uint)pfVar7 >> 0x10),
                       (ushort)(fVar1 < fVar2) << 8 | (ushort)(NAN(fVar1) || NAN(fVar2)) << 10 |
                       (ushort)(fVar1 == fVar2) << 0xe);
      if (fVar1 >= fVar2) {
        return CONCAT31((int3)(uVar8 >> 8),1);
      }
    }
  }
  return uVar8;
}

