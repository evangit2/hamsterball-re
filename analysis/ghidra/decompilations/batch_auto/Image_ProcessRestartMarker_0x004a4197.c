
uint __cdecl Image_ProcessRestartMarker(int *param_1)

{
  uint uVar1;
  
  if (param_1[0x5e] == 0) {
    uVar1 = Image_ScanMarker(param_1);
    if ((char)uVar1 == '\0') goto LAB_004a4211;
  }
  if (param_1[0x5e] == *(int *)(param_1[100] + 0x54) + 0xd0) {
    *(undefined4 *)(*param_1 + 0x14) = 0x61;
    *(undefined4 *)(*param_1 + 0x18) = *(undefined4 *)(param_1[100] + 0x54);
    (**(code **)(*param_1 + 4))(param_1,3);
    param_1[0x5e] = 0;
  }
  else {
    uVar1 = (**(code **)(param_1[5] + 0x14))(param_1,*(int *)(param_1[100] + 0x54));
    if ((char)uVar1 == '\0') {
LAB_004a4211:
      return uVar1 & 0xffffff00;
    }
  }
  *(uint *)(param_1[100] + 0x54) = *(int *)(param_1[100] + 0x54) + 1U & 7;
  return 1;
}

