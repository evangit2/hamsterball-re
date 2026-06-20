
int __thiscall D3DTexture_CopyLockedData(void *this,void *param_1)

{
  short sVar1;
  int *piVar2;
  void *pvVar3;
  undefined4 *puVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  short *psVar8;
  int iVar9;
  undefined4 *puVar10;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  pvVar3 = param_1;
  *(uint *)((int)param_1 + 0xc) =
       (*(uint *)((int)this + 0xc) ^ *(uint *)((int)param_1 + 0xc)) & 0x20 ^
       *(uint *)((int)this + 0xc);
  local_c = 0;
  local_10 = 0;
  local_8 = MeshBuffer_Allocate(param_1,*(uint *)((int)this + 0x58),*(uint *)((int)this + 0x30));
  if (((-1 < local_8) &&
      (local_8 = Graphics_DrawIndexedPrimitive(param_1,&local_10,0), -1 < local_8)) &&
     (local_8 = Graphics_DrawIndexedPrimitive(this,&local_c,0x10), -1 < local_8)) {
    if ((*(byte *)((int)this + 0xc) & 4) != 0) {
      puVar4 = *(undefined4 **)((int)this + 0x48);
      puVar10 = *(undefined4 **)((int)param_1 + 0x48);
      for (uVar5 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar5 != 0; uVar5 = uVar5 - 1) {
        *puVar10 = *puVar4;
        puVar4 = puVar4 + 1;
        puVar10 = puVar10 + 1;
      }
      for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
        *(undefined1 *)puVar10 = *(undefined1 *)puVar4;
        puVar4 = (undefined4 *)((int)puVar4 + 1);
        puVar10 = (undefined4 *)((int)puVar10 + 1);
      }
    }
    if ((*(byte *)((int)this + 0xc) & 8) != 0) {
      uVar7 = *(int *)((int)this + 0x58) << 1;
      puVar4 = *(undefined4 **)((int)this + 0x4c);
      puVar10 = *(undefined4 **)((int)param_1 + 0x4c);
      for (uVar5 = uVar7 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *puVar10 = *puVar4;
        puVar4 = puVar4 + 1;
        puVar10 = puVar10 + 1;
      }
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *(undefined1 *)puVar10 = *(undefined1 *)puVar4;
        puVar4 = (undefined4 *)((int)puVar4 + 1);
        puVar10 = (undefined4 *)((int)puVar10 + 1);
      }
    }
    if (*(int *)((int)this + 0x50) != 0) {
      puVar4 = operator_new(*(int *)((int)this + 0x54) * 0x14);
      *(undefined4 **)((int)param_1 + 0x50) = puVar4;
      if (puVar4 == (undefined4 *)0x0) {
        local_8 = -0x7ff8fff2;
        goto LAB_00483f02;
      }
      puVar10 = *(undefined4 **)((int)this + 0x50);
      for (uVar5 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar5 != 0; uVar5 = uVar5 - 1) {
        *puVar4 = *puVar10;
        puVar10 = puVar10 + 1;
        puVar4 = puVar4 + 1;
      }
      for (iVar6 = 0; iVar6 != 0; iVar6 = iVar6 + -1) {
        *(undefined1 *)puVar4 = *(undefined1 *)puVar10;
        puVar10 = (undefined4 *)((int)puVar10 + 1);
        puVar4 = (undefined4 *)((int)puVar4 + 1);
      }
      *(undefined4 *)((int)param_1 + 0x54) = *(undefined4 *)((int)this + 0x54);
    }
    iVar6 = 0;
    param_1 = (void *)0x0;
    if (*(int *)((int)this + 0x58) != 0) {
      do {
        psVar8 = (short *)(local_10 + iVar6);
        iVar9 = (local_c + iVar6) - (int)psVar8;
        local_14 = 3;
        do {
          sVar1 = *(short *)(iVar9 + (int)psVar8);
          if (sVar1 == -1) {
            *psVar8 = -1;
          }
          else {
            *psVar8 = sVar1;
          }
          psVar8 = psVar8 + 1;
          local_14 = local_14 + -1;
        } while (local_14 != 0);
        param_1 = (void *)((int)param_1 + 1);
        iVar6 = iVar6 + 6;
      } while (param_1 < *(void **)((int)this + 0x58));
    }
    if ((*(byte *)((int)this + 0xc) & 1) != 0) {
      iVar6 = 0;
      param_1 = (void *)0x0;
      if (*(int *)((int)this + 0x58) != 0) {
        do {
          psVar8 = (short *)(*(int *)((int)pvVar3 + 0x44) + iVar6);
          iVar9 = (*(int *)((int)this + 0x44) + iVar6) - (int)psVar8;
          local_14 = 3;
          do {
            sVar1 = *(short *)((int)psVar8 + iVar9);
            if (sVar1 == -1) {
              *psVar8 = -1;
            }
            else {
              *psVar8 = sVar1;
            }
            psVar8 = psVar8 + 1;
            local_14 = local_14 + -1;
          } while (local_14 != 0);
          param_1 = (void *)((int)param_1 + 1);
          iVar6 = iVar6 + 6;
        } while (param_1 < *(void **)((int)this + 0x58));
      }
    }
  }
LAB_00483f02:
  if (local_c != 0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_10 != 0) {
    piVar2 = *(int **)((int)pvVar3 + 0x3c);
    (**(code **)(*piVar2 + 0x30))(piVar2);
  }
  return local_8;
}

