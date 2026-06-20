
uint __thiscall
MeshStrip_ComputeWinding(void *this,int *param_1,int *param_2,int param_3,int *param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  uint local_14;
  int *local_c;
  int local_8;
  
  local_14 = 0;
  if (*(int *)((int)this + 0x18) <= *param_2) {
LAB_0047cae0:
    *param_4 = param_3;
    return local_14;
  }
LAB_0047ca64:
  iVar3 = *param_2;
  if (iVar3 == 0) {
    iVar8 = *(int *)((int)this + 0x28);
    iVar7 = param_3 * 0x24;
    iVar3 = *(int *)(iVar8 + 0xc + iVar7);
    iVar5 = 0;
    if (0 < iVar3) {
      piVar6 = (int *)(iVar8 + 0x10 + iVar7);
      while (*(int *)(iVar8 + 0x1c + *piVar6 * 0x24) != 0) {
        iVar5 = iVar5 + 1;
        piVar6 = piVar6 + 1;
        if (iVar3 <= iVar5) goto LAB_0047caa8;
      }
      iVar3 = *(int *)(iVar8 + 0x10 + (iVar5 + param_3 * 9) * 4);
      if (iVar3 == -1) goto LAB_0047caa8;
      local_c = *(int **)(iVar8 + iVar7);
      iVar5 = iVar3 * 0x24;
      if ((((local_c == (int *)*(int *)(iVar5 + iVar8)) ||
           (local_c == (int *)*(int *)(iVar5 + 4 + iVar8))) ||
          (local_c == (int *)*(int *)(iVar5 + 8 + iVar8))) &&
         (((local_c = *(int **)(iVar8 + 4 + iVar7), local_c == (int *)*(int *)(iVar5 + iVar8) ||
           (local_c == (int *)*(int *)(iVar5 + 4 + iVar8))) ||
          (local_c == (int *)*(int *)(iVar5 + 8 + iVar8))))) {
        local_c = *(int **)(iVar8 + 8 + iVar7);
      }
      *param_1 = (int)local_c;
      iVar8 = *(int *)((int)this + 0x28);
      local_14 = *(int *)(iVar8 + iVar7);
      if (((local_14 != *(int *)(iVar5 + iVar8)) &&
          (iVar4 = *(int *)(iVar5 + 4 + iVar8), local_14 != iVar4)) &&
         (((iVar1 = *(int *)(iVar5 + 8 + iVar8), local_14 != iVar1 &&
           ((local_14 = *(int *)(iVar8 + 4 + iVar7), local_14 != *(int *)(iVar5 + iVar8) &&
            (local_14 != iVar4)))) && (local_14 != iVar1)))) {
        local_14 = -1;
      }
      param_1[1] = local_14;
      if (local_14 == -1) {
        return 0xffffffff;
      }
      iVar8 = *(int *)((int)this + 0x28);
      iVar4 = *(int *)(iVar8 + 8 + iVar7);
      if (((((iVar4 != *(int *)(iVar5 + iVar8)) &&
            (iVar1 = *(int *)(iVar5 + 4 + iVar8), iVar4 != iVar1)) &&
           (iVar2 = *(int *)(iVar5 + 8 + iVar8), iVar4 != iVar2)) &&
          ((iVar4 = *(int *)(iVar8 + 4 + iVar7), iVar4 != *(int *)(iVar5 + iVar8) &&
           (iVar4 != iVar1)))) && (iVar4 != iVar2)) {
        iVar4 = -1;
      }
      param_1[2] = iVar4;
      if (iVar4 == -1) {
        return 0xffffffff;
      }
      piVar6 = (int *)(iVar7 + *(int *)((int)this + 0x28));
      iVar8 = *piVar6;
      if (((local_c == (int *)iVar8) && (local_14 == piVar6[1])) && (iVar4 == piVar6[2])) {
LAB_0047cc70:
        local_14 = 1;
        *param_2 = 3;
      }
      else {
        if (((local_c != (int *)piVar6[1]) || (local_14 != iVar8)) || (iVar4 != piVar6[2])) {
          if (((local_c != (int *)piVar6[2]) || (local_14 != iVar8)) || (iVar4 != piVar6[1])) {
            Window_Notify(0x5341d0,(byte *)"WINDING FACTS");
            Window_Notify(0x5341d0,(byte *)"Strip[0] = %d %d %d ?? %d %d %d");
            MessageBoxA((HWND)0x0,"Could not determine WINDING","Severe Error",0);
            return 0xffffffff;
          }
          goto LAB_0047cc70;
        }
        local_14 = 0;
        *param_2 = 3;
      }
      goto LAB_0047cd33;
    }
LAB_0047caa8:
    iVar3 = param_3 * 0x24;
    *param_1 = *(int *)(iVar3 + *(int *)((int)this + 0x28));
    param_1[1] = *(int *)(iVar3 + 4 + *(int *)((int)this + 0x28));
    param_1[2] = *(int *)(iVar3 + 8 + *(int *)((int)this + 0x28));
    local_14 = 1;
    *param_2 = 3;
  }
  else {
    iVar8 = *(int *)((int)this + 0x28);
    iVar5 = param_3 * 0x24;
    local_8 = 0;
    if (0 < *(int *)(iVar8 + 0xc + iVar5)) {
      local_c = (int *)(iVar8 + 0x10 + iVar5);
      do {
        iVar7 = *local_c;
        if (*(int *)(iVar8 + 0x1c + iVar7 * 0x24) == 0) {
          iVar1 = param_1[iVar3 + -2];
          iVar4 = iVar8 + iVar7 * 0x24;
          if (((iVar1 == *(int *)(iVar8 + iVar7 * 0x24)) || (iVar1 == *(int *)(iVar4 + 4))) ||
             (iVar1 == *(int *)(iVar4 + 8))) {
            iVar1 = param_1[iVar3 + -1];
            iVar4 = iVar8 + iVar7 * 0x24;
            if (((iVar1 == *(int *)(iVar8 + iVar7 * 0x24)) || (iVar1 == *(int *)(iVar4 + 4))) ||
               (iVar1 == *(int *)(iVar4 + 8))) goto LAB_0047cd1e;
          }
        }
        local_8 = local_8 + 1;
        local_c = local_c + 1;
        if (*(int *)(iVar8 + 0xc + iVar5) <= local_8) break;
      } while( true );
    }
  }
  goto LAB_0047cae0;
LAB_0047cd1e:
  iVar3 = *(int *)(iVar8 + 0x10 + local_8 * 4 + param_3 * 0x24);
LAB_0047cd33:
  iVar8 = param_3 * 0x24;
  if (iVar3 == -1) goto LAB_0047cae0;
  iVar5 = *(int *)((int)this + 0x28);
  iVar4 = iVar3 * 0x24;
  iVar7 = *(int *)(iVar4 + iVar5);
  if ((((iVar7 == *(int *)(iVar5 + iVar8)) || (iVar7 == *(int *)(iVar5 + 4 + iVar8))) ||
      (iVar7 == *(int *)(iVar5 + 8 + iVar8))) &&
     (((iVar7 = *(int *)(iVar4 + 4 + iVar5), iVar7 == *(int *)(iVar5 + iVar8) ||
       (iVar7 == *(int *)(iVar5 + 4 + iVar8))) || (iVar7 == *(int *)(iVar5 + 8 + iVar8))))) {
    iVar7 = *(int *)(iVar4 + 8 + iVar5);
  }
  param_1[*param_2] = iVar7;
  local_14 = local_14 ^ 1;
  *param_2 = *param_2 + 1;
  *(undefined4 *)(iVar4 + 0x1c + *(int *)((int)this + 0x28)) = 1;
  param_3 = iVar3;
  if (*(int *)((int)this + 0x18) <= *param_2) goto LAB_0047cae0;
  goto LAB_0047ca64;
}

