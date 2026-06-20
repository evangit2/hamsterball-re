
void __cdecl D3D_CreateYCbCrConverter(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  undefined4 unaff_ESI;
  undefined4 unaff_EDI;
  int iVar7;
  int iVar8;
  
  puVar3 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x30);
  *(undefined4 **)(param_1 + 0x19c) = puVar3;
  *(undefined1 *)(puVar3 + 2) = 0;
  *puVar3 = &LAB_004b4855;
  puVar3[10] = *(int *)(param_1 + 100) * *(int *)(param_1 + 0x5c);
  if (*(int *)(param_1 + 0x110) == 2) {
    puVar3[1] = D3D_DecodePaired;
    puVar3[3] = D3D_ConvertYCbCrToRGB_Paired;
    uVar4 = (**(code **)(*(int *)(param_1 + 4) + 4))(param_1,1,puVar3[10]);
    puVar3[8] = uVar4;
  }
  else {
    puVar3[8] = 0;
    puVar3[1] = D3D_DecodeSingle;
    puVar3[3] = D3D_ConvertYCbCrToRGB_Single;
  }
  iVar1 = *(int *)(param_1 + 0x19c);
  uVar4 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x400,unaff_EDI,unaff_ESI);
  *(undefined4 *)(iVar1 + 0x10) = uVar4;
  uVar4 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x400);
  *(undefined4 *)(iVar1 + 0x14) = uVar4;
  uVar4 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x400);
  *(undefined4 *)(iVar1 + 0x18) = uVar4;
  uVar4 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x400);
  *(undefined4 *)(iVar1 + 0x1c) = uVar4;
  iVar2 = 0;
  iVar6 = 0x5b6900;
  iVar5 = 0x2c8d00;
  iVar7 = -0xe25100;
  iVar8 = -0xb2f480;
  do {
    *(int *)(iVar2 + *(int *)(iVar1 + 0x10)) = iVar8 >> 0x10;
    *(int *)(iVar2 + *(int *)(iVar1 + 0x14)) = iVar7 >> 0x10;
    *(int *)(iVar2 + *(int *)(iVar1 + 0x18)) = iVar6;
    *(int *)(iVar2 + *(int *)(iVar1 + 0x1c)) = iVar5;
    iVar5 = iVar5 + -0x581a;
    iVar6 = iVar6 + -0xb6d2;
    iVar2 = iVar2 + 4;
    iVar7 = iVar7 + 0x1c5a2;
    iVar8 = iVar8 + 0x166e9;
  } while (-0x2b34e7 < iVar5);
  return;
}

