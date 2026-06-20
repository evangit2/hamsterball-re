
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall UI_MeasureTextExtent(void *this,float *param_1,byte *param_2)

{
  byte bVar1;
  float fVar2;
  byte *pbVar3;
  float local_c;
  byte *local_8;
  
  bVar1 = *param_2;
  local_8 = (byte *)0x0;
  local_c = 0.0;
  pbVar3 = param_2;
  param_2 = (byte *)(float)*(int *)((int)this + 0x424);
  fVar2 = _DAT_004cf368;
  do {
    if (bVar1 == 0) {
      *param_1 = local_c;
      param_1[1] = (float)local_8;
      return;
    }
    if (bVar1 == 10) {
      param_2 = (byte *)((float)*(int *)((int)this + 0x424) + (float)param_2);
      fVar2 = _DAT_004cf368;
LAB_004570b8:
      if (*(char *)((int)this + (uint)bVar1 * 0x14 + 0x42c) != '\0') {
        fVar2 = (float)*(int *)((int)this + (uint)bVar1 * 0x14 + 0x430) *
                *(float *)((int)this + 0x428) + fVar2;
        if (local_c < fVar2) {
          local_c = fVar2;
        }
        if ((float)local_8 < (float)param_2) {
          local_8 = param_2;
        }
      }
    }
    else {
      if (bVar1 != 0x20) goto LAB_004570b8;
      fVar2 = fVar2 + (float)*(int *)((int)this + 0x420);
    }
    bVar1 = pbVar3[1];
    pbVar3 = pbVar3 + 1;
  } while( true );
}

