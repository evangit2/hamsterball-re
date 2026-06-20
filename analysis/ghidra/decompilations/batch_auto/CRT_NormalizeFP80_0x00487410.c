
/* WARNING: Removing unreachable block (ram,0x004874b9) */

int CRT_NormalizeFP80(ushort *param_1)

{
  ushort uVar1;
  int iVar2;
  ushort uVar3;
  
  uVar1 = param_1[3];
  iVar2 = 0;
  param_1[3] = 0;
  do {
    if (0xff < param_1[2]) break;
    param_1[3] = param_1[2];
    param_1[2] = param_1[1];
    param_1[1] = *param_1;
    *param_1 = 0;
    iVar2 = iVar2 + -0x10;
  } while (param_1[3] == 0);
  if (param_1[3] == 0) {
    uVar3 = param_1[2];
    while (uVar3 < 0x10) {
      param_1[2] = param_1[1] >> 0xf | param_1[2] << 1;
      param_1[1] = *param_1 >> 0xf | param_1[1] * 2;
      *param_1 = *param_1 * 2;
      iVar2 = iVar2 + -1;
      uVar3 = param_1[2];
    }
  }
  else {
    do {
      *param_1 = *param_1 >> 1 | param_1[1] << 0xf;
      param_1[1] = param_1[1] >> 1 | param_1[2] << 0xf;
      uVar3 = param_1[3] >> 1;
      iVar2 = iVar2 + 1;
      param_1[2] = param_1[2] >> 1 | param_1[3] << 0xf;
      param_1[3] = uVar3;
    } while (uVar3 != 0);
  }
  param_1[3] = uVar1;
  return iVar2;
}

