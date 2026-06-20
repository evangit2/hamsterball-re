
int __cdecl Audio_CodecAllocSamples16(int *param_1,int param_2,int param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  int *this;
  void *extraout_ECX;
  void *this_00;
  void *extraout_ECX_00;
  uint uVar4;
  uint uVar5;
  
  this = (int *)(param_3 * 0x80);
  uVar1 = (uint)(0x3b9ac9f0 / ZEXT48(this));
  iVar2 = param_1[1];
  if (uVar1 == 0) {
    *(undefined4 *)(*param_1 + 0x14) = 0x45;
    this = param_1;
    (**(code **)*param_1)();
  }
  if ((int)param_4 <= (int)uVar1) {
    uVar1 = param_4;
  }
  *(uint *)(iVar2 + 0x4c) = uVar1;
  iVar2 = Audio_CodecAllocBuffer(this,param_1,param_2,(undefined4 *)(param_4 << 2));
  uVar5 = 0;
  this_00 = extraout_ECX;
  if (param_4 != 0) {
    do {
      if (param_4 - uVar5 <= uVar1) {
        uVar1 = param_4 - uVar5;
      }
      puVar3 = Audio_CodecAllocSimple(this_00,param_1,param_2,uVar1 * param_3 * 0x80);
      this_00 = extraout_ECX_00;
      for (uVar4 = uVar1; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 **)(iVar2 + uVar5 * 4) = puVar3;
        puVar3 = puVar3 + param_3 * 0x20;
        uVar5 = uVar5 + 1;
        this_00 = (void *)0x0;
      }
    } while (uVar5 < param_4);
  }
  return iVar2;
}

