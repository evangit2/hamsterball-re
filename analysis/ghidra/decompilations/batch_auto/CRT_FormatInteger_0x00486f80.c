
void CRT_FormatInteger(uint *param_1,char param_2)

{
  uint uVar1;
  int extraout_EDX;
  char *pcVar2;
  int iVar3;
  uint uVar4;
  char *pcVar5;
  char acStack_18 [24];
  
  if (param_2 == 'X') {
    pcVar2 = s_0123456789ABCDEF_004f780c;
  }
  else {
    pcVar2 = s_0123456789abcdef_004f77f8;
    if (param_2 == 'o') {
      uVar4 = 8;
      goto LAB_00486fb4;
    }
    uVar4 = 10;
    if (param_2 != 'x') goto LAB_00486fb4;
  }
  uVar4 = 0x10;
LAB_00486fb4:
  uVar1 = *param_1;
  iVar3 = 0x18;
  if ((uVar1 != 0) || (param_1[9] != 0)) {
    iVar3 = 0x17;
    acStack_18[0x17] = pcVar2[uVar1 % uVar4];
  }
  uVar1 = uVar1 / uVar4;
  *param_1 = uVar1;
  for (; (0 < (int)uVar1 && (0 < iVar3)); iVar3 = iVar3 + -1) {
    uVar1 = CRT_DivCeil(*param_1,uVar4);
    *param_1 = uVar1;
    (&stack0xffffffe7)[iVar3] = pcVar2[extraout_EDX];
  }
  uVar1 = 0x18 - iVar3;
  param_1[5] = uVar1;
  pcVar2 = acStack_18 + iVar3;
  pcVar5 = (char *)param_1[2];
  for (uVar4 = uVar1 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar1 = uVar1 & 3; uVar1 != 0; uVar1 = uVar1 - 1) {
    *pcVar5 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar5 = pcVar5 + 1;
  }
  uVar4 = param_1[5];
  uVar1 = param_1[9];
  if ((int)uVar4 < (int)uVar1) {
    param_1[4] = uVar1 - uVar4;
  }
  if ((((int)uVar1 < 0) && (((byte)param_1[0xc] & 0x14) == 0x10)) &&
     (uVar1 = (param_1[10] - param_1[3]) - param_1[4], uVar1 != uVar4 && -1 < (int)(uVar1 - uVar4)))
  {
    param_1[4] = param_1[4] + 1;
  }
  return;
}

