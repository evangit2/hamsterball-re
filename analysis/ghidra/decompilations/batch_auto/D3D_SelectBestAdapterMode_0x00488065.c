
int __cdecl
D3D_SelectBestAdapterMode(int *param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined1 local_18 [12];
  undefined4 local_c;
  int *local_8;
  
  local_8 = (int *)0x0;
  D3D8_DebugSetMute(1);
  if (param_1 != (int *)0x0) {
    (**(code **)(*param_1 + 0x18))(param_1,&local_8);
    (**(code **)(*param_1 + 0x1c))(param_1,&local_ec);
    (**(code **)(*param_1 + 0x20))(param_1,local_18);
  }
  uVar3 = 0xffffffff;
  piVar4 = &DAT_004db5a8;
  piVar6 = (int *)&DAT_004db580;
  piVar5 = (int *)&DAT_004db580;
  if (&DAT_004db5a8 < PTR_DAT_004f7fd0) {
    do {
      piVar6 = piVar5;
      if ((*piVar4 != 0) &&
         ((param_1 == (int *)0x0 ||
          (iVar1 = (**(code **)(*local_8 + 0x28))
                             (local_8,local_e8,local_ec,local_c,param_2,param_3,*piVar4), -1 < iVar1
          )))) {
        piVar6 = piVar4;
        if (*param_4 == *piVar4) break;
        piVar6 = piVar5;
        if ((((piVar4[8] != 0) &&
             (uVar2 = D3D_CompareRefreshRates((int)param_4,(int)piVar4), uVar2 != 0xffffffff)) &&
            (uVar2 <= uVar3)) && ((uVar2 != uVar3 || ((uint)piVar4[2] < (uint)piVar5[2])))) {
          uVar3 = uVar2;
          piVar6 = piVar4;
        }
      }
      piVar4 = piVar4 + 9;
      piVar5 = piVar6;
    } while (piVar4 < PTR_DAT_004f7fd0);
  }
  if (local_8 != (int *)0x0) {
    (**(code **)(*local_8 + 8))(local_8);
    local_8 = (int *)0x0;
  }
  D3D8_DebugSetMute(0);
  return *piVar6;
}

