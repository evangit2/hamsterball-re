
int __thiscall D3DTexture_CopyIndexData(void *this,void *param_1)

{
  ushort uVar1;
  int *piVar2;
  void *pvVar3;
  undefined4 *puVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  undefined4 *puVar9;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  pvVar3 = param_1;
  *(uint *)((int)param_1 + 0xc) =
       (*(uint *)((int)this + 0xc) ^ *(uint *)((int)param_1 + 0xc)) & 0x20 ^
       *(uint *)((int)this + 0xc);
  local_14 = 0;
  local_18 = 0;
  local_8 = D3DTexture_ResizeAndValidate
                      (param_1,*(uint *)((int)this + 0x58),*(uint *)((int)this + 0x30));
  if (((-1 < local_8) &&
      (local_8 = Graphics_DrawIndexedPrimitive(param_1,&local_18,0), -1 < local_8)) &&
     (local_8 = Graphics_DrawIndexedPrimitive(this,&local_14,0x10), -1 < local_8)) {
    if ((*(byte *)((int)this + 0xc) & 4) != 0) {
      puVar4 = *(undefined4 **)((int)this + 0x48);
      puVar9 = *(undefined4 **)((int)param_1 + 0x48);
      for (uVar6 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar6 != 0; uVar6 = uVar6 - 1) {
        *puVar9 = *puVar4;
        puVar4 = puVar4 + 1;
        puVar9 = puVar9 + 1;
      }
      for (iVar7 = 0; iVar7 != 0; iVar7 = iVar7 + -1) {
        *(undefined1 *)puVar9 = *(undefined1 *)puVar4;
        puVar4 = (undefined4 *)((int)puVar4 + 1);
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
    }
    if ((*(byte *)((int)this + 0xc) & 8) != 0) {
      uVar8 = *(int *)((int)this + 0x58) << 1;
      puVar4 = *(undefined4 **)((int)this + 0x4c);
      puVar9 = *(undefined4 **)((int)param_1 + 0x4c);
      for (uVar6 = uVar8 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
        *puVar9 = *puVar4;
        puVar4 = puVar4 + 1;
        puVar9 = puVar9 + 1;
      }
      for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined1 *)puVar9 = *(undefined1 *)puVar4;
        puVar4 = (undefined4 *)((int)puVar4 + 1);
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
    }
    if (*(int *)((int)this + 0x50) != 0) {
      puVar4 = operator_new(*(int *)((int)this + 0x54) * 0x14);
      *(undefined4 **)((int)param_1 + 0x50) = puVar4;
      if (puVar4 == (undefined4 *)0x0) {
        local_8 = -0x7ff8fff2;
        goto LAB_004840d1;
      }
      puVar9 = *(undefined4 **)((int)this + 0x50);
      for (uVar6 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar6 != 0; uVar6 = uVar6 - 1) {
        *puVar4 = *puVar9;
        puVar9 = puVar9 + 1;
        puVar4 = puVar4 + 1;
      }
      for (iVar7 = 0; iVar7 != 0; iVar7 = iVar7 + -1) {
        *(undefined1 *)puVar4 = *(undefined1 *)puVar9;
        puVar9 = (undefined4 *)((int)puVar9 + 1);
        puVar4 = (undefined4 *)((int)puVar4 + 1);
      }
      *(undefined4 *)((int)param_1 + 0x54) = *(undefined4 *)((int)this + 0x54);
    }
    param_1 = (void *)0x0;
    if (*(int *)((int)this + 0x58) != 0) {
      local_10 = 0;
      local_c = 0;
      do {
        uVar6 = 0;
        do {
          uVar1 = *(ushort *)(local_10 + local_14 + uVar6 * 2);
          if (uVar1 == 0xffff) {
            *(undefined4 *)(local_c + local_18 + uVar6 * 4) = 0xffffffff;
          }
          else {
            *(uint *)(local_c + local_18 + uVar6 * 4) = (uint)uVar1;
          }
          uVar6 = uVar6 + 1;
        } while (uVar6 < 3);
        param_1 = (void *)((int)param_1 + 1);
        local_c = local_c + 0xc;
        local_10 = local_10 + 6;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
    if (((*(byte *)((int)this + 0xc) & 1) != 0) &&
       (param_1 = (void *)0x0, *(int *)((int)this + 0x58) != 0)) {
      local_c = 0;
      local_10 = 0;
      do {
        iVar7 = *(int *)((int)this + 0x44);
        iVar5 = *(int *)((int)pvVar3 + 0x44) + local_10;
        uVar6 = 0;
        do {
          uVar1 = *(ushort *)(iVar7 + local_c + uVar6 * 2);
          if (uVar1 == 0xffff) {
            *(undefined4 *)(iVar5 + uVar6 * 4) = 0xffffffff;
          }
          else {
            *(uint *)(iVar5 + uVar6 * 4) = (uint)uVar1;
          }
          uVar6 = uVar6 + 1;
        } while (uVar6 < 3);
        param_1 = (void *)((int)param_1 + 1);
        local_10 = local_10 + 0xc;
        local_c = local_c + 6;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
  }
LAB_004840d1:
  if (local_14 != 0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_18 != 0) {
    piVar2 = *(int **)((int)pvVar3 + 0x3c);
    (**(code **)(*piVar2 + 0x30))(piVar2);
  }
  return local_8;
}

