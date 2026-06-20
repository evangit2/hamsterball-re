
void NoiseTable_Init(ushort param_1)

{
  undefined4 uVar1;
  undefined2 *puVar2;
  int iVar3;
  
  RNG_SeedSmall(param_1);
  puVar2 = &DAT_005341f0;
  iVar3 = 0x200;
  do {
    uVar1 = RNG_RandSmall();
    *puVar2 = (short)uVar1;
    puVar2 = puVar2 + 1;
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}

