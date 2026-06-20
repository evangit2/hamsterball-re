
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __cdecl
Ray_PlaneIntersectT(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5)

{
  float fVar1;
  
  fVar1 = *param_2 * *param_4 + param_2[1] * param_4[1] + param_2[2] * param_4[2];
  if (fVar1 == _DAT_004cf368) {
    return CONCAT22((short)((uint)param_2 >> 0x10),
                    (ushort)(fVar1 < _DAT_004cf368) << 8 |
                    (ushort)(NAN(fVar1) || NAN(_DAT_004cf368)) << 10 |
                    (ushort)(fVar1 == _DAT_004cf368) << 0xe);
  }
  *param_5 = ((*param_3 * *param_4 + param_3[1] * param_4[1] + param_3[2] * param_4[2]) -
             (*param_1 * *param_4 + param_1[1] * param_4[1] + param_1[2] * param_4[2])) / fVar1;
  return CONCAT31((int3)((uint)param_5 >> 8),1);
}

