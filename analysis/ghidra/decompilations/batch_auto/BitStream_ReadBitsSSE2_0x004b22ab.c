
undefined4 __cdecl BitStream_ReadBitsSSE2(undefined4 *param_1,uint param_2,int param_3,int param_4)

{
  uint uVar1;
  int iVar2;
  byte *pbVar3;
  
  iVar2 = param_1[1];
  pbVar3 = (byte *)*param_1;
  for (; param_3 < 0x19; param_3 = param_3 + 8) {
    if (param_1[2] == 0) {
      if (iVar2 == 0) {
        uVar1 = (**(code **)(*(int *)(param_1[7] + 0x14) + 0xc))(param_1[7]);
        if ((char)uVar1 == '\0') {
LAB_004b2391:
          return uVar1 & 0xffffff00;
        }
        pbVar3 = (byte *)**(undefined4 **)(param_1[7] + 0x14);
        iVar2 = (*(undefined4 **)(param_1[7] + 0x14))[1];
      }
      uVar1 = (uint)*pbVar3;
      iVar2 = iVar2 + -1;
      pbVar3 = pbVar3 + 1;
      if (uVar1 == 0xff) {
        do {
          if (iVar2 == 0) {
            uVar1 = (**(code **)(*(int *)(param_1[7] + 0x14) + 0xc))(param_1[7]);
            if ((char)uVar1 == '\0') goto LAB_004b2391;
            pbVar3 = (byte *)**(undefined4 **)(param_1[7] + 0x14);
            iVar2 = (*(undefined4 **)(param_1[7] + 0x14))[1];
          }
          uVar1 = (uint)*pbVar3;
          iVar2 = iVar2 + -1;
          pbVar3 = pbVar3 + 1;
        } while (uVar1 == 0xff);
        if (uVar1 != 0) {
          param_1[2] = uVar1;
          goto LAB_004b232f;
        }
        uVar1 = 0xff;
      }
    }
    else {
LAB_004b232f:
      if (param_4 <= param_3) break;
      if (*(char *)param_1[8] == '\0') {
        *(undefined4 *)(*(int *)param_1[7] + 0x14) = 0x71;
        (**(code **)(*(int *)param_1[7] + 4))((int *)param_1[7],0xffffffff);
        *(undefined1 *)param_1[8] = 1;
      }
      uVar1 = 0;
    }
    param_2 = param_2 << 8 | uVar1;
  }
  param_1[3] = param_2;
  param_1[6] = param_3;
  *param_1 = pbVar3;
  param_1[1] = iVar2;
  return CONCAT31((int3)((uint)param_3 >> 8),1);
}

