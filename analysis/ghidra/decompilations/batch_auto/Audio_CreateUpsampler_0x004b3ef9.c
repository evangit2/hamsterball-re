
void __cdecl Audio_CreateUpsampler(int *param_1)

{
  int iVar1;
  bool bVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  int *piVar8;
  int iVar9;
  undefined1 local_10;
  int local_c;
  
  puVar3 = (undefined4 *)(**(code **)param_1[1])(param_1,1,0xa0);
  param_1[0x67] = (int)puVar3;
  *(undefined1 *)(puVar3 + 2) = 0;
  *puVar3 = &LAB_004b3abd;
  puVar3[1] = Audio_FlushDecode;
  if ((char)param_1[0x42] != '\0') {
    *(undefined4 *)(*param_1 + 0x14) = 0x17;
    (**(code **)*param_1)(param_1);
  }
  if (((char)param_1[0x12] == '\0') || (param_1[0x45] < 2)) {
    bVar2 = false;
  }
  else {
    bVar2 = true;
  }
  local_c = 0;
  if (0 < param_1[8]) {
    puVar7 = puVar3 + 0xd;
    piVar8 = (int *)(param_1[0x31] + 0x24);
    do {
      iVar4 = (piVar8[-7] * *piVar8) / param_1[0x45];
      iVar5 = (piVar8[-6] * *piVar8) / param_1[0x45];
      iVar9 = param_1[0x44];
      iVar1 = param_1[0x43];
      puVar7[0xc] = iVar5;
      if ((char)piVar8[3] == '\0') {
        *puVar7 = &LAB_004b3ba8;
      }
      else if ((iVar4 == iVar1) && (iVar5 == iVar9)) {
        *puVar7 = &LAB_004b3b9d;
      }
      else {
        if (iVar4 * 2 == iVar1) {
          if (iVar5 == iVar9) {
            if ((bVar2) && (2 < (uint)piVar8[1])) {
              *puVar7 = Audio_Upscale2xInterpolate;
            }
            else {
              *puVar7 = Audio_Upscale2x;
            }
          }
          else {
            if ((iVar4 * 2 != iVar1) || (iVar5 * 2 != iVar9)) goto LAB_004b402e;
            if ((bVar2) && (2 < (uint)piVar8[1])) {
              *puVar7 = Audio_Upscale2xInterpolateStereo;
              *(undefined1 *)(puVar3 + 2) = 1;
            }
            else {
              *puVar7 = Audio_Upscale2xStereo;
            }
          }
        }
        else {
LAB_004b402e:
          if ((iVar1 % iVar4 == 0) && (iVar9 % iVar5 == 0)) {
            local_10 = (undefined1)(iVar1 / iVar4);
            *puVar7 = Audio_UpsampleRows;
            *(undefined1 *)(local_c + 0x8c + (int)puVar3) = local_10;
            *(char *)(local_c + 0x96 + (int)puVar3) = (char)(iVar9 / iVar5);
          }
          else {
            *(undefined4 *)(*param_1 + 0x14) = 0x25;
            (**(code **)*param_1)(param_1);
          }
        }
        iVar9 = param_1[0x44];
        iVar1 = param_1[1];
        iVar4 = Math_AlignUp(param_1[0x17],param_1[0x43]);
        uVar6 = (**(code **)(iVar1 + 8))(param_1,1,iVar4,iVar9);
        puVar7[-10] = uVar6;
      }
      local_c = local_c + 1;
      puVar7 = puVar7 + 1;
      piVar8 = piVar8 + 0x15;
    } while (local_c < param_1[8]);
  }
  return;
}

