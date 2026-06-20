
undefined4 CRT_ClassifyFP(short *param_1,ushort *param_2)

{
  ushort uVar1;
  uint in_EAX;
  undefined4 uVar2;
  int iVar3;
  
  uVar1 = param_2[3];
  if ((uVar1 & 0x7fff) == 0x7fff) {
    *param_1 = 0;
    if ((((param_2[2] & 0x7fff) != 0) || (param_2[1] != 0)) || (uVar2 = 1, *param_2 != 0)) {
      return 2;
    }
  }
  else {
    if (((param_2[2] == 0) && (param_2[1] == 0)) && (*param_2 == 0)) {
      *param_1 = 0;
      return in_EAX & 0xffff0000;
    }
    iVar3 = CRT_NormalizeFP80(param_2);
    param_2[3] = param_2[3] & 0x83fe | 0x3fe;
    *param_1 = (uVar1 & 0x7fff) + (short)iVar3 + -0x3fe;
    uVar2 = CONCAT22((short)((uint)param_1 >> 0x10),0xffff);
  }
  return uVar2;
}

