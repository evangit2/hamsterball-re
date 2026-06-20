
void __thiscall Scene_SubdivideRandom(void *this,int param_1,char param_2)

{
  float fVar1;
  char cVar2;
  void *this_00;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  ulonglong uVar10;
  int local_48;
  int local_44;
  int local_40;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cd063;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaList_Free((int)this + 0x18);
  __ftol2();
  __ftol2();
  uVar10 = __ftol2();
  iVar6 = ((int)((longlong)((ulonglong)(uint)((int)uVar10 >> 0x1f) << 0x20 | uVar10 & 0xffffffff) /
                (longlong)param_1) + -1) * param_1;
  __ftol2();
  __ftol2();
  uVar10 = __ftol2();
  iVar7 = iVar6 + param_1;
  iVar4 = ((int)((longlong)((ulonglong)(uint)((int)uVar10 >> 0x1f) << 0x20 | uVar10 & 0xffffffff) /
                (longlong)param_1) + 1) * param_1 + param_1;
  if (iVar7 < iVar4) {
    local_40 = iVar7 + -1;
    iVar5 = iVar7;
    do {
      if (iVar7 <= iVar4) {
        local_44 = iVar7 + -1;
        iVar3 = iVar7;
        do {
          if (iVar7 <= iVar4) {
            fVar1 = (float)iVar6;
            local_48 = iVar7 + -1;
            iVar8 = iVar7;
            do {
              InitMaterialArray(&local_24);
              local_10 = (float)local_48;
              piVar9 = (int *)0x0;
              local_4 = 0;
              local_24 = fVar1;
              local_20 = fVar1;
              local_1c = fVar1;
              local_18 = (float)local_40;
              local_14 = (float)local_44;
              this_00 = operator_new(0x10d0);
              local_4._0_1_ = 1;
              if (this_00 != (void *)0x0) {
                piVar9 = Level_ctor(this_00,*(undefined4 *)((int)this + 4));
              }
              local_4 = (uint)local_4._1_3_ << 8;
              cVar2 = (**(code **)(*piVar9 + 0x5c))(this,&local_24,0);
              if (cVar2 == '\x01') {
                AthenaList_Append((void *)((int)this + 0x18),(int)piVar9);
                if (param_2 == '\x01') {
                  MeshWorld_ClearObjectLists(piVar9[2]);
                }
              }
              else {
                (**(code **)*piVar9)(1);
              }
              local_4 = 0xffffffff;
              NoOp();
              iVar8 = iVar8 + param_1;
              local_48 = local_48 + param_1;
            } while (iVar8 <= iVar4);
          }
          iVar3 = iVar3 + param_1;
          local_44 = local_44 + param_1;
        } while (iVar3 <= iVar4);
      }
      iVar5 = iVar5 + param_1;
      local_40 = local_40 + param_1;
    } while (iVar5 < iVar4);
  }
  *(undefined1 *)((int)this + 0x430) = 0;
  iVar4 = AthenaList_GetSize((int)this + 0x18);
  if (0 < iVar4) {
    *(undefined1 *)((int)this + 0x430) = 1;
  }
  ExceptionList = pvStack_c;
  return;
}

