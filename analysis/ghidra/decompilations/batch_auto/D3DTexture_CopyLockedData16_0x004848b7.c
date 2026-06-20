
int __thiscall D3DTexture_CopyLockedData16(void *this,void *param_1)

{
  int *piVar1;
  void *pvVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  pvVar2 = param_1;
  *(uint *)((int)param_1 + 0xc) =
       (*(uint *)((int)this + 0xc) ^ *(uint *)((int)param_1 + 0xc)) & 0x20 ^
       *(uint *)((int)this + 0xc);
  local_14 = 0;
  local_18 = 0;
  local_8 = MeshBuffer_Allocate(param_1,*(uint *)((int)this + 0x58),*(uint *)((int)this + 0x30));
  if (((-1 < local_8) &&
      (local_8 = Graphics_DrawIndexedPrimitive(param_1,&local_18,0), -1 < local_8)) &&
     (local_8 = Graphics_DrawIndexedPrimitive(this,&local_14,0x10), -1 < local_8)) {
    if ((*(byte *)((int)this + 0xc) & 4) != 0) {
      puVar3 = *(undefined4 **)((int)this + 0x48);
      puVar8 = *(undefined4 **)((int)param_1 + 0x48);
      for (uVar4 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar8 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar8 = puVar8 + 1;
      }
      for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    if ((*(byte *)((int)this + 0xc) & 8) != 0) {
      uVar6 = *(int *)((int)this + 0x58) << 1;
      puVar3 = *(undefined4 **)((int)this + 0x4c);
      puVar8 = *(undefined4 **)((int)param_1 + 0x4c);
      for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar8 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar8 = puVar8 + 1;
      }
      for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    if (*(int *)((int)this + 0x50) != 0) {
      puVar3 = operator_new(*(int *)((int)this + 0x54) * 0x14);
      *(undefined4 **)((int)param_1 + 0x50) = puVar3;
      if (puVar3 == (undefined4 *)0x0) {
        local_8 = -0x7ff8fff2;
        goto LAB_00484a61;
      }
      puVar8 = *(undefined4 **)((int)this + 0x50);
      for (uVar4 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar3 = *puVar8;
        puVar8 = puVar8 + 1;
        puVar3 = puVar3 + 1;
      }
      for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
        *(undefined1 *)puVar3 = *(undefined1 *)puVar8;
        puVar8 = (undefined4 *)((int)puVar8 + 1);
        puVar3 = (undefined4 *)((int)puVar3 + 1);
      }
      *(undefined4 *)((int)param_1 + 0x54) = *(undefined4 *)((int)this + 0x54);
    }
    param_1 = (void *)0x0;
    if (*(int *)((int)this + 0x58) != 0) {
      local_10 = 0;
      local_c = 0;
      do {
        uVar4 = 0;
        do {
          if (*(int *)(local_10 + local_14 + uVar4 * 4) == -1) {
            *(undefined2 *)(local_c + local_18 + uVar4 * 2) = 0xffff;
          }
          else {
            *(undefined2 *)(local_c + local_18 + uVar4 * 2) =
                 *(undefined2 *)(local_10 + local_14 + uVar4 * 4);
          }
          uVar4 = uVar4 + 1;
        } while (uVar4 < 3);
        param_1 = (void *)((int)param_1 + 1);
        local_c = local_c + 6;
        local_10 = local_10 + 0xc;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
    if (((*(byte *)((int)this + 0xc) & 1) != 0) &&
       (param_1 = (void *)0x0, *(int *)((int)this + 0x58) != 0)) {
      local_c = 0;
      local_10 = 0;
      do {
        iVar5 = *(int *)((int)pvVar2 + 0x44) + local_10;
        iVar7 = *(int *)((int)this + 0x44) + local_c;
        uVar4 = 0;
        do {
          if (*(int *)(iVar7 + uVar4 * 4) == -1) {
            *(undefined2 *)(iVar5 + uVar4 * 2) = 0xffff;
          }
          else {
            *(undefined2 *)(iVar5 + uVar4 * 2) = *(undefined2 *)(iVar7 + uVar4 * 4);
          }
          uVar4 = uVar4 + 1;
        } while (uVar4 < 3);
        param_1 = (void *)((int)param_1 + 1);
        local_10 = local_10 + 6;
        local_c = local_c + 0xc;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
  }
LAB_00484a61:
  if (local_14 != 0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_18 != 0) {
    piVar1 = *(int **)((int)pvVar2 + 0x3c);
    (**(code **)(*piVar1 + 0x30))(piVar1);
  }
  return local_8;
}

