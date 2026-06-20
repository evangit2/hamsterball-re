
void __cdecl Audio_InitSynthesis(int param_1,char param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  int *piVar8;
  int iVar9;
  
  iVar9 = param_1;
  puVar3 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x74);
  *(undefined4 **)(param_1 + 0x184) = puVar3;
  *puVar3 = &LAB_004b14df;
  puVar3[2] = Audio_SelectHybridMode;
  puVar3[0x1c] = 0;
  if (param_2 == '\0') {
    iVar4 = (**(code **)(*(int *)(param_1 + 4) + 4))(param_1,1,0x500);
    piVar8 = puVar3 + 8;
    iVar9 = 10;
    do {
      *piVar8 = iVar4;
      piVar8 = piVar8 + 1;
      iVar4 = iVar4 + 0x80;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
    puVar3[1] = ReturnZero;
    puVar3[3] = Audio_DecodeGranuleMono;
    puVar3[4] = 0;
  }
  else {
    piVar8 = (int *)(param_1 + 0x20);
    piVar1 = (int *)(param_1 + 0xc4);
    param_1 = 0;
    if (0 < *piVar8) {
      piVar8 = (int *)(*piVar1 + 0xc);
      _param_2 = puVar3 + 0x12;
      do {
        iVar4 = *piVar8;
        iVar7 = iVar4;
        if (*(char *)(iVar9 + 200) != '\0') {
          iVar7 = iVar4 * 3;
        }
        iVar2 = *(int *)(iVar9 + 4);
        iVar4 = Math_AlignUp(piVar8[5],iVar4);
        iVar5 = Math_AlignUp(piVar8[4],piVar8[-1]);
        uVar6 = (**(code **)(iVar2 + 0x14))(iVar9,1,1,iVar5,iVar4,iVar7);
        param_1 = param_1 + 1;
        *_param_2 = uVar6;
        piVar8 = piVar8 + 0x15;
        _param_2 = _param_2 + 1;
      } while (param_1 < *(int *)(iVar9 + 0x20));
    }
    puVar3[1] = Audio_DecodeGranuleStereo;
    puVar3[3] = Audio_ReadFrame;
    puVar3[4] = puVar3 + 0x12;
  }
  return;
}

