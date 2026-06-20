
int __thiscall D3DTexture_CopyIndexData16(void *this,void *param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 *puVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int *piVar7;
  int iVar8;
  undefined4 *puVar9;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  pvVar2 = param_1;
  *(uint *)((int)param_1 + 0xc) =
       (*(uint *)((int)this + 0xc) ^ *(uint *)((int)param_1 + 0xc)) & 0x20 ^
       *(uint *)((int)this + 0xc);
  local_c = 0;
  local_10 = 0;
  local_8 = D3DTexture_ResizeAndValidate
                      (param_1,*(uint *)((int)this + 0x58),*(uint *)((int)this + 0x30));
  if (((-1 < local_8) &&
      (local_8 = Graphics_DrawIndexedPrimitive(param_1,&local_10,0), -1 < local_8)) &&
     (local_8 = Graphics_DrawIndexedPrimitive(this,&local_c,0x10), -1 < local_8)) {
    if ((*(byte *)((int)this + 0xc) & 4) != 0) {
      puVar3 = *(undefined4 **)((int)this + 0x48);
      puVar9 = *(undefined4 **)((int)param_1 + 0x48);
      for (uVar4 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar9 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar9 = puVar9 + 1;
      }
      for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
        *(undefined1 *)puVar9 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
    }
    if ((*(byte *)((int)this + 0xc) & 8) != 0) {
      uVar6 = *(int *)((int)this + 0x58) << 1;
      puVar3 = *(undefined4 **)((int)this + 0x4c);
      puVar9 = *(undefined4 **)((int)param_1 + 0x4c);
      for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar9 = *puVar3;
        puVar3 = puVar3 + 1;
        puVar9 = puVar9 + 1;
      }
      for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *(undefined1 *)puVar9 = *(undefined1 *)puVar3;
        puVar3 = (undefined4 *)((int)puVar3 + 1);
        puVar9 = (undefined4 *)((int)puVar9 + 1);
      }
    }
    if (*(int *)((int)this + 0x50) != 0) {
      puVar3 = operator_new(*(int *)((int)this + 0x54) * 0x14);
      *(undefined4 **)((int)param_1 + 0x50) = puVar3;
      if (puVar3 == (undefined4 *)0x0) {
        local_8 = -0x7ff8fff2;
        goto LAB_00484c12;
      }
      puVar9 = *(undefined4 **)((int)this + 0x50);
      for (uVar4 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar4 != 0; uVar4 = uVar4 - 1) {
        *puVar3 = *puVar9;
        puVar9 = puVar9 + 1;
        puVar3 = puVar3 + 1;
      }
      for (iVar5 = 0; iVar5 != 0; iVar5 = iVar5 + -1) {
        *(undefined1 *)puVar3 = *(undefined1 *)puVar9;
        puVar9 = (undefined4 *)((int)puVar9 + 1);
        puVar3 = (undefined4 *)((int)puVar3 + 1);
      }
      *(undefined4 *)((int)param_1 + 0x54) = *(undefined4 *)((int)this + 0x54);
    }
    iVar5 = 0;
    param_1 = (void *)0x0;
    if (*(int *)((int)this + 0x58) != 0) {
      do {
        piVar7 = (int *)(local_10 + iVar5);
        iVar8 = (local_c + iVar5) - (int)piVar7;
        local_14 = 3;
        do {
          iVar1 = *(int *)(iVar8 + (int)piVar7);
          if (iVar1 == -1) {
            *piVar7 = -1;
          }
          else {
            *piVar7 = iVar1;
          }
          piVar7 = piVar7 + 1;
          local_14 = local_14 + -1;
        } while (local_14 != 0);
        param_1 = (void *)((int)param_1 + 1);
        iVar5 = iVar5 + 0xc;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
    if ((*(byte *)((int)this + 0xc) & 1) != 0) {
      iVar5 = 0;
      param_1 = (void *)0x0;
      if (*(int *)((int)this + 0x58) != 0) {
        do {
          piVar7 = (int *)(*(int *)((int)pvVar2 + 0x44) + iVar5);
          iVar8 = (*(int *)((int)this + 0x44) + iVar5) - (int)piVar7;
          local_14 = 3;
          do {
            iVar1 = *(int *)((int)piVar7 + iVar8);
            if (iVar1 == -1) {
              *piVar7 = -1;
            }
            else {
              *piVar7 = iVar1;
            }
            piVar7 = piVar7 + 1;
            local_14 = local_14 + -1;
          } while (local_14 != 0);
          param_1 = (void *)((int)param_1 + 1);
          iVar5 = iVar5 + 0xc;
        } while (param_1 < *(void **)((int)this + 0x58));
      }
    }
  }
LAB_00484c12:
  if (local_c != 0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_10 != 0) {
    piVar7 = *(int **)((int)pvVar2 + 0x3c);
    (**(code **)(*piVar7 + 0x30))(piVar7);
  }
  return local_8;
}

