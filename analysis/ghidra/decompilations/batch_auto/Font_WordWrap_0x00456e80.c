
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

byte * __thiscall Font_WordWrap(void *this,byte *param_1,int param_2,byte *param_3)

{
  byte bVar1;
  float fVar2;
  byte *pbVar3;
  byte *pbVar4;
  byte *pbVar5;
  byte *pbVar6;
  byte *pbVar7;
  
  pbVar4 = param_1;
  bVar1 = *param_1;
  param_1 = (byte *)0x0;
  pbVar6 = param_3;
  if (bVar1 != 0) {
    pbVar7 = pbVar4;
    do {
      bVar1 = *pbVar4;
      if (bVar1 == 10) {
        param_1 = (byte *)0x0;
      }
      if (bVar1 == 0x20) {
        fVar2 = (float)*(int *)((int)this + 0x420);
      }
      else {
        fVar2 = _DAT_004cf368;
        if (*(char *)((int)this + (uint)bVar1 * 0x14 + 0x42c) != '\0') {
          fVar2 = (float)*(int *)((int)this + (uint)bVar1 * 0x14 + 0x430) *
                  *(float *)((int)this + 0x428);
        }
      }
      if ((float)param_2 < (float)param_1 + fVar2) {
        param_1 = (byte *)0x0;
        pbVar5 = pbVar6;
        pbVar3 = pbVar4;
        do {
          pbVar3 = pbVar3 + -1;
          if (pbVar3 == pbVar7) goto LAB_00456f32;
          pbVar5 = pbVar5 + -1;
        } while (*pbVar3 != 0x20);
        pbVar4 = pbVar3 + 1;
        pbVar6 = pbVar5;
LAB_00456f32:
        *pbVar6 = 10;
        pbVar6 = pbVar6 + 1;
        pbVar7 = pbVar4;
      }
      bVar1 = *pbVar4;
      param_1 = (byte *)(fVar2 + (float)param_1);
      pbVar4 = pbVar4 + 1;
      *pbVar6 = bVar1;
      pbVar6 = pbVar6 + 1;
    } while (*pbVar4 != 0);
  }
  *pbVar6 = 0;
  return param_3;
}

