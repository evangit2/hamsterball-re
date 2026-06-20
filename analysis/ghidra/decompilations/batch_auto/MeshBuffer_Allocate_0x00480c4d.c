
int __thiscall MeshBuffer_Allocate(void *this,uint param_1,uint param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  bool bVar8;
  undefined1 local_540 [1032];
  int local_138;
  uint local_134;
  undefined1 local_114 [4];
  undefined4 local_110;
  uint local_40;
  uint local_3c;
  uint local_38;
  uint local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 *local_20;
  int *local_1c;
  undefined4 *local_18;
  undefined4 *local_14;
  undefined4 *local_10;
  int *local_c;
  int *local_8;
  
  local_3c = *(uint *)((int)this + 0x5c);
  local_40 = *(uint *)((int)this + 0x34);
  local_18 = (undefined4 *)0x0;
  local_14 = (undefined4 *)0x0;
  local_8 = (int *)0x0;
  local_c = (int *)0x0;
  local_20 = (undefined4 *)0x0;
  local_10 = (undefined4 *)0x0;
  local_1c = (int *)0x0;
  local_38 = local_3c;
  local_34 = local_3c;
  if ((0xfffe < param_1) || (0xfffe < param_2)) {
    iVar2 = -0x7789f794;
    goto LAB_004810ab;
  }
  if (local_3c < param_1) {
    iVar2 = (**(code **)(**(int **)((int)this + 0x24) + 0x60))
                      (*(int **)((int)this + 0x24),param_1 * 6,*(undefined4 *)((int)this + 0x18),
                       0x65,*(undefined4 *)((int)this + 0x10),&local_c);
    if (iVar2 < 0) goto LAB_004810ab;
    if (*(int *)((int)this + 0x58) != 0) {
      iVar2 = (**(code **)(**(int **)((int)this + 0x3c) + 0x2c))
                        (*(int **)((int)this + 0x3c),0,0,&local_20,0x800);
      if ((iVar2 < 0) ||
         (iVar2 = (**(code **)(*local_c + 0x2c))(local_c,0,0,&local_10,0x800), iVar2 < 0))
      goto LAB_004810ab;
      uVar4 = *(int *)((int)this + 0x58) * 6;
      puVar6 = local_20;
      puVar7 = local_10;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *puVar7 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar7 = puVar7 + 1;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined1 *)puVar7 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar7 = (undefined4 *)((int)puVar7 + 1);
      }
      uVar4 = (param_1 - *(int *)((int)this + 0x58)) * 6;
      puVar6 = (undefined4 *)((int)local_10 + *(int *)((int)this + 0x58) * 6);
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *puVar6 = 0;
        puVar6 = puVar6 + 1;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined1 *)puVar6 = 0;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
      iVar2 = (**(code **)(*local_c + 0x30))(local_c);
      if (iVar2 < 0) goto LAB_004810ab;
      local_10 = (undefined4 *)0x0;
      iVar2 = (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
      if (iVar2 < 0) goto LAB_004810ab;
      local_20 = (undefined4 *)0x0;
    }
    piVar1 = *(int **)((int)this + 0x3c);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 8))(piVar1);
      *(undefined4 *)((int)this + 0x3c) = 0;
    }
    piVar1 = local_c;
    local_c = (int *)0x0;
    *(int **)((int)this + 0x3c) = piVar1;
    *(uint *)((int)this + 0x5c) = param_1;
    if (((*(byte *)((int)this + 0xc) & 1) == 0) || (iVar2 = DWord6Array_Grow(), iVar2 != 0)) {
      if ((*(byte *)((int)this + 0xc) & 4) != 0) {
        iVar2 = MeshContainer_GrowArray
                          ((int *)((int)this + 0x48),param_1,*(int *)((int)this + 0x58),&local_38);
        if (iVar2 == 0) goto LAB_00480dbf;
        puVar6 = (undefined4 *)(*(int *)((int)this + 0x48) + *(int *)((int)this + 0x58) * 4);
        for (uVar5 = param_1 - *(int *)((int)this + 0x58) & 0x3fffffff; uVar5 != 0;
            uVar5 = uVar5 - 1) {
          *puVar6 = 0;
          puVar6 = puVar6 + 1;
        }
        for (iVar2 = 0; iVar2 != 0; iVar2 = iVar2 + -1) {
          *(undefined1 *)puVar6 = 0;
          puVar6 = (undefined4 *)((int)puVar6 + 1);
        }
      }
      if (((*(byte *)((int)this + 0xc) & 8) == 0) ||
         (iVar2 = WideArray_Grow((undefined4 *)((int)this + 0x4c),param_1,*(int *)((int)this + 0x58)
                                 ,&local_34), iVar2 != 0)) goto LAB_00480e24;
    }
LAB_00480dbf:
    iVar2 = -0x7ff8fff2;
  }
  else {
LAB_00480e24:
    if (*(uint *)((int)this + 0x34) < param_2) {
      if (((*(byte *)((int)this + 0xc) & 2) != 0) &&
         (iVar2 = WideArray_Grow((undefined4 *)((int)this + 0x38),param_2,*(int *)((int)this + 0x30)
                                 ,&local_40), iVar2 == 0)) goto LAB_00480dbf;
      if ((*(byte *)((int)this + 0xc) & 0x20) == 0) {
        iVar2 = (**(code **)(**(int **)((int)this + 0x24) + 0x5c))
                          (*(int **)((int)this + 0x24),*(int *)((int)this + 0x2c) * param_2,
                           *(undefined4 *)((int)this + 0x1c),*(undefined4 *)((int)this + 4),
                           *(undefined4 *)((int)this + 0x14),&local_8);
        if (iVar2 < 0) goto LAB_004810ab;
        if (*(int *)((int)this + 0x30) != 0) {
          iVar2 = (**(code **)(**(int **)((int)this + 0x28) + 0x2c))
                            (*(int **)((int)this + 0x28),0,0,&local_18,0x800);
          if ((iVar2 < 0) ||
             (iVar2 = (**(code **)(*local_8 + 0x2c))(local_8,0,0,&local_14,0x800), iVar2 < 0))
          goto LAB_004810ab;
          uVar4 = *(int *)((int)this + 0x30) * *(int *)((int)this + 0x2c);
          puVar6 = local_18;
          puVar7 = local_14;
          for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
            *puVar7 = *puVar6;
            puVar6 = puVar6 + 1;
            puVar7 = puVar7 + 1;
          }
          for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
            *(undefined1 *)puVar7 = *(undefined1 *)puVar6;
            puVar6 = (undefined4 *)((int)puVar6 + 1);
            puVar7 = (undefined4 *)((int)puVar7 + 1);
          }
          uVar4 = (param_2 - *(int *)((int)this + 0x30)) * *(int *)((int)this + 0x2c);
          puVar6 = (undefined4 *)
                   (*(int *)((int)this + 0x2c) * *(int *)((int)this + 0x30) + (int)local_14);
          for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
            *puVar6 = 0;
            puVar6 = puVar6 + 1;
          }
          for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
            *(undefined1 *)puVar6 = 0;
            puVar6 = (undefined4 *)((int)puVar6 + 1);
          }
          iVar2 = (**(code **)(*local_8 + 0x30))(local_8);
          if (iVar2 < 0) goto LAB_004810ab;
          local_14 = (undefined4 *)0x0;
          iVar2 = (**(code **)(**(int **)((int)this + 0x28) + 0x30))(*(int **)((int)this + 0x28));
          if (iVar2 < 0) goto LAB_004810ab;
          local_18 = (undefined4 *)0x0;
        }
        piVar1 = *(int **)((int)this + 0x28);
        if (piVar1 != (int *)0x0) {
          (**(code **)(*piVar1 + 8))(piVar1);
          *(undefined4 *)((int)this + 0x28) = 0;
        }
        *(int **)((int)this + 0x28) = local_8;
        local_8 = (int *)0x0;
        *(uint *)((int)this + 0x34) = param_2;
      }
    }
    *(uint *)((int)this + 0x30) = param_2;
    *(uint *)((int)this + 0x58) = param_1;
    iVar2 = (**(code **)(**(int **)((int)this + 0x24) + 0x1c))
                      (*(int **)((int)this + 0x24),local_114);
    if (((-1 < iVar2) &&
        (iVar2 = (**(code **)(**(int **)((int)this + 0x24) + 0x18))
                           (*(int **)((int)this + 0x24),&local_1c), -1 < iVar2)) &&
       (iVar2 = (**(code **)(*local_1c + 0x14))(local_1c,local_110,2,local_540), -1 < iVar2)) {
      if (((local_138 == 0x1002) && (0x5143 < local_134)) && (local_134 < 0x5148)) {
        *(byte *)((int)this + 0xc) = *(byte *)((int)this + 0xc) | 0x80;
      }
      if ((*(byte *)((int)this + 0xc) & 0x80) == 0) {
        bVar8 = DAT_00534650 != *(int *)((int)this + 0x24);
        *(undefined4 *)((int)this + 0x68) = DAT_00534654;
        *(undefined4 *)((int)this + 0x6c) = DAT_00534658;
        if (bVar8) {
          local_30 = 0;
          local_2c = 0;
          local_28 = 0;
          local_24 = 0;
          D3D8_DebugSetMute(1);
          iVar3 = (**(code **)(**(int **)((int)this + 0x24) + 0x104))
                            (*(int **)((int)this + 0x24),4,&local_30,0x10);
          iVar2 = 0;
          D3D8_DebugSetMute(0);
          if (((iVar3 < 0) || (iVar3 == 1)) || (local_30 != 0x48434143)) {
            local_2c = 1;
            local_28 = 0xc;
            local_24 = 7;
          }
          if (local_2c == 0) {
            local_28 = 0;
            local_24 = 0;
          }
          if (*(uint *)((int)this + 0x68) < *(uint *)((int)this + 0x6c)) {
            *(uint *)((int)this + 0x6c) = *(uint *)((int)this + 0x68);
          }
          DAT_00534650 = *(int *)((int)this + 0x24);
          DAT_00534654 = local_28;
          DAT_00534658 = local_24;
          *(undefined4 *)((int)this + 0x68) = local_28;
          *(undefined4 *)((int)this + 0x6c) = local_24;
        }
      }
      else {
        *(undefined4 *)((int)this + 0x68) = 0;
        *(undefined4 *)((int)this + 0x6c) = 0;
      }
    }
  }
LAB_004810ab:
  if (local_1c != (int *)0x0) {
    (**(code **)(*local_1c + 8))(local_1c);
    local_1c = (int *)0x0;
  }
  if (local_10 != (undefined4 *)0x0) {
    (**(code **)(*local_c + 0x30))(local_c);
  }
  if (local_20 != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_14 != (undefined4 *)0x0) {
    (**(code **)(*local_8 + 0x30))(local_8);
  }
  if (local_18 != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)this + 0x28) + 0x30))(*(int **)((int)this + 0x28));
  }
  return iVar2;
}

