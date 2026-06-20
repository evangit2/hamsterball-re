
void __cdecl Sound_Init(int *param_1,int param_2,int param_3)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  
  param_1[1] = 0;
  if (param_2 != 0x3d) {
    *(undefined4 *)(*param_1 + 0x14) = 10;
    *(undefined4 *)(*param_1 + 0x18) = 0x3d;
    *(int *)(*param_1 + 0x1c) = param_2;
    (**(code **)*param_1)(param_1);
  }
  if (param_3 != 0x1a8) {
    *(undefined4 *)(*param_1 + 0x14) = 0x13;
    *(undefined4 *)(*param_1 + 0x18) = 0x1a8;
    *(int *)(*param_1 + 0x1c) = param_3;
    (**(code **)*param_1)(param_1);
  }
  iVar4 = *param_1;
  piVar1 = param_1;
  for (iVar3 = 0x6a; iVar3 != 0; iVar3 = iVar3 + -1) {
    *piVar1 = 0;
    piVar1 = piVar1 + 1;
  }
  *param_1 = iVar4;
  *(undefined1 *)(param_1 + 3) = 1;
  Audio_CodecInitStruct(param_1);
  param_1[2] = 0;
  param_1[5] = 0;
  param_1[0x24] = 0;
  param_1[0x25] = 0;
  param_1[0x26] = 0;
  param_1[0x27] = 0;
  iVar4 = 4;
  piVar1 = param_1 + 0x2c;
  do {
    piVar1[-4] = 0;
    *piVar1 = 0;
    piVar1 = piVar1 + 1;
    iVar4 = iVar4 + -1;
  } while (iVar4 != 0);
  Media_InitParser((int)param_1);
  Audio_CodecCreate((int)param_1);
  if (DAT_004fb258 == '\x02') {
    uVar2 = CPUID_GetPolarityBit();
    DAT_004fb258 = (char)uVar2;
  }
  param_1[4] = 200;
  return;
}

