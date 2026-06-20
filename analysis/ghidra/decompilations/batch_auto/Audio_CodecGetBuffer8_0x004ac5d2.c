
int __cdecl Audio_CodecGetBuffer8(int *param_1,int *param_2,uint param_3,uint param_4,char param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  
  uVar1 = param_3 + param_4;
  if ((((uint)param_2[1] < uVar1) || ((uint)param_2[3] < param_4)) || (*param_2 == 0)) {
    *(undefined4 *)(*param_1 + 0x14) = 0x14;
    (**(code **)*param_1)(param_1);
  }
  if ((param_3 < (uint)param_2[6]) || ((uint)(param_2[4] + param_2[6]) < uVar1)) {
    if (*(char *)((int)param_2 + 0x22) == '\0') {
      *(undefined4 *)(*param_1 + 0x14) = 0x44;
      (**(code **)*param_1)(param_1);
    }
    if (*(char *)((int)param_2 + 0x21) != '\0') {
      Audio_CodecRenderSamples8(param_1,'\x01');
      *(undefined1 *)((int)param_2 + 0x21) = 0;
    }
    uVar4 = param_3;
    if ((param_3 <= (uint)param_2[6]) && (uVar4 = uVar1 - param_2[4], (int)uVar4 < 0)) {
      uVar4 = 0;
    }
    param_2[6] = uVar4;
    Audio_CodecRenderSamples8(param_1,'\0');
  }
  uVar4 = param_2[7];
  if (uVar4 < uVar1) {
    if ((uVar4 < param_3) && (uVar4 = param_3, param_5 != '\0')) {
      *(undefined4 *)(*param_1 + 0x14) = 0x14;
      (**(code **)*param_1)(param_1);
    }
    if (param_5 != '\0') {
      param_2[7] = uVar1;
    }
    if ((char)param_2[8] != '\0') {
      uVar2 = param_2[2];
      iVar3 = param_2[6];
      for (uVar4 = uVar4 - iVar3; uVar4 < uVar1 - iVar3; uVar4 = uVar4 + 1) {
        Mem_Zero(*(undefined4 **)(*param_2 + uVar4 * 4),uVar2);
      }
      goto LAB_004ac6d6;
    }
    if (param_5 == '\0') {
      *(undefined4 *)(*param_1 + 0x14) = 0x14;
      (**(code **)*param_1)(param_1);
      goto LAB_004ac6d6;
    }
  }
  else {
LAB_004ac6d6:
    if (param_5 == '\0') goto LAB_004ac6e0;
  }
  *(undefined1 *)((int)param_2 + 0x21) = 1;
LAB_004ac6e0:
  return *param_2 + (param_3 - param_2[6]) * 4;
}

