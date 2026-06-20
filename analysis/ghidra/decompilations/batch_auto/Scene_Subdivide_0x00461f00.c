
void __thiscall Scene_Subdivide(void *this,int param_1,int param_2,int param_3,char param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  char cVar4;
  void *this_00;
  int *piVar5;
  int iVar6;
  int local_44;
  int local_40;
  int local_30;
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
  puStack_8 = &LAB_004cd043;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  AthenaList_Free((int)this + 0x18);
  iVar6 = *(int *)((int)this + 8);
  local_40 = 0;
  fVar1 = (*(float *)(iVar6 + 0x468) - *(float *)(iVar6 + 0x45c)) / (float)param_1;
  fVar2 = (*(float *)(iVar6 + 0x46c) - *(float *)(iVar6 + 0x460)) / (float)param_2;
  fVar3 = (*(float *)(iVar6 + 0x470) - *(float *)(iVar6 + 0x464)) / (float)param_3;
  if (0 < param_1) {
    do {
      local_44 = 0;
      if (0 < param_2) {
        do {
          local_30 = 0;
          if (0 < param_3) {
            do {
              InitMaterialArray(&local_24);
              iVar6 = *(int *)((int)this + 8);
              local_24 = (float)local_40 * fVar1 + *(float *)(iVar6 + 0x45c);
              local_4 = 0;
              local_20 = (float)local_44 * fVar2 + *(float *)(iVar6 + 0x460);
              local_1c = (float)local_30 * fVar3 + *(float *)(iVar6 + 0x464);
              local_18 = local_24 + fVar1;
              local_14 = local_20 + fVar2;
              local_10 = local_1c + fVar3;
              this_00 = operator_new(0x10d0);
              local_4._0_1_ = 1;
              if (this_00 == (void *)0x0) {
                piVar5 = (int *)0x0;
              }
              else {
                piVar5 = Level_ctor(this_00,*(undefined4 *)((int)this + 4));
              }
              local_4 = (uint)local_4._1_3_ << 8;
              cVar4 = (**(code **)(*piVar5 + 0x5c))(this,&local_24,0);
              if (cVar4 == '\x01') {
                AthenaList_Append((void *)((int)this + 0x18),(int)piVar5);
                if (param_4 == '\x01') {
                  MeshWorld_ClearObjectLists(piVar5[2]);
                }
              }
              else {
                (**(code **)*piVar5)(1);
              }
              local_4 = 0xffffffff;
              NoOp();
              local_30 = local_30 + 1;
            } while (local_30 < param_3);
          }
          local_44 = local_44 + 1;
        } while (local_44 < param_2);
      }
      local_40 = local_40 + 1;
    } while (local_40 < param_1);
  }
  *(undefined1 *)((int)this + 0x430) = 0;
  iVar6 = AthenaList_GetSize((int)this + 0x18);
  if (0 < iVar6) {
    *(undefined1 *)((int)this + 0x430) = 1;
  }
  ExceptionList = pvStack_c;
  return;
}

