
/* WARNING: Function: __chkstk replaced with injection: alloca_probe */

int Mesh_RenderSubset(void *param_1,uint param_2)

{
  bool bVar1;
  void *pvVar2;
  uint uVar3;
  int *piVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  uint local_18;
  int iVar11;
  uint local_c;
  int local_8;
  
  pvVar2 = param_1;
  iVar11 = 0;
  local_8 = 0;
  if (*(int *)((int)param_1 + 0x28) != 0) {
    iVar8 = **(int **)((int)param_1 + 0x24);
    if ((*(byte *)((int)param_1 + 0xc) & 0x40) == 0) {
      (**(code **)(iVar8 + 0x130))(*(int **)((int)param_1 + 0x24),*(undefined4 *)((int)param_1 + 4))
      ;
      (**(code **)(**(int **)((int)param_1 + 0x24) + 0x14c))
                (*(int **)((int)param_1 + 0x24),0,*(undefined4 *)((int)param_1 + 0x28),
                 *(undefined4 *)((int)param_1 + 0x2c));
      (**(code **)(**(int **)((int)param_1 + 0x24) + 0x154))
                (*(int **)((int)param_1 + 0x24),*(undefined4 *)((int)param_1 + 0x3c),0);
      if ((*(byte *)((int)param_1 + 0xc) & 0x10) == 0) {
        iVar8 = 0;
        uVar3 = 0;
        if (*(int *)((int)param_1 + 0x58) != 0) {
          param_1 = (void *)0x0;
          do {
            if (*(uint *)(*(int *)((int)pvVar2 + 0x48) + uVar3 * 4) == param_2) {
              iVar8 = iVar8 + 1;
            }
            else {
              if (iVar8 != 0) {
                (**(code **)(**(int **)((int)pvVar2 + 0x24) + 0x11c))
                          (*(int **)((int)pvVar2 + 0x24),4,0,*(undefined4 *)((int)pvVar2 + 0x30),
                           (void *)((int)param_1 + iVar8 * -3),iVar8);
              }
              iVar8 = 0;
            }
            param_1 = (void *)((int)param_1 + 3);
            uVar3 = uVar3 + 1;
          } while (uVar3 < *(uint *)((int)pvVar2 + 0x58));
          if (iVar8 != 0) {
            (**(code **)(**(int **)((int)pvVar2 + 0x24) + 0x11c))
                      (*(int **)((int)pvVar2 + 0x24),4,0,*(undefined4 *)((int)pvVar2 + 0x30),
                       uVar3 * 3 + iVar8 * -3,iVar8);
          }
        }
      }
      else {
        uVar3 = *(uint *)((int)param_1 + 0x54);
        if (((uVar3 <= param_2) ||
            (uVar6 = param_2, *(uint *)(*(int *)((int)param_1 + 0x50) + param_2 * 0x14) != param_2))
           && (uVar6 = 0, uVar3 != 0)) {
          puVar5 = *(uint **)((int)param_1 + 0x50);
          do {
            if (*puVar5 == param_2) break;
            uVar6 = uVar6 + 1;
            puVar5 = puVar5 + 5;
          } while (uVar6 < *(uint *)((int)param_1 + 0x54));
        }
        if (uVar6 < uVar3) {
          iVar8 = *(int *)((int)param_1 + 0x50) + uVar6 * 0x14;
          if (*(int *)(iVar8 + 8) != 0) {
            (**(code **)(**(int **)((int)param_1 + 0x24) + 0x11c))
                      (*(int **)((int)param_1 + 0x24),4,*(undefined4 *)(iVar8 + 0xc),
                       *(undefined4 *)(iVar8 + 0x10),*(int *)(iVar8 + 4) * 3,*(int *)(iVar8 + 8));
          }
        }
      }
    }
    else {
      (**(code **)(iVar8 + 0x130))();
      local_18 = *(uint *)((int)param_1 + 0x58);
      bVar1 = false;
      local_c = 0;
      if ((*(byte *)((int)param_1 + 0xc) & 0x10) != 0) {
        uVar3 = 0;
        if (*(uint *)((int)param_1 + 0x54) != 0) {
          puVar5 = *(uint **)((int)param_1 + 0x50);
          do {
            if (*puVar5 == param_2) break;
            uVar3 = uVar3 + 1;
            puVar5 = puVar5 + 5;
          } while (uVar3 < *(uint *)((int)param_1 + 0x54));
        }
        if (uVar3 == *(uint *)((int)param_1 + 0x54)) {
          return 0;
        }
        iVar11 = *(int *)((int)param_1 + 0x50) + uVar3 * 0x14;
        local_c = *(uint *)(iVar11 + 4);
        local_18 = *(int *)(iVar11 + 8) + local_c;
        bVar1 = true;
      }
      iVar11 = Graphics_DrawIndexedPrimitiveUP(param_1,&local_8,0x10);
      if (((-1 < iVar11) &&
          (iVar11 = Graphics_DrawIndexedPrimitive(param_1,(int)param_1 + 0x40,0x10), -1 < iVar11))
         && (local_c < local_18)) {
        iVar8 = local_c * 0xc;
        do {
          piVar4 = (int *)(*(int *)((int)param_1 + 0x40) + iVar8);
          if ((bVar1) || (*(uint *)(*(int *)((int)param_1 + 0x48) + local_c * 4) == param_2)) {
            uVar3 = *(uint *)((int)param_1 + 0x2c);
            puVar9 = (undefined4 *)(*piVar4 * uVar3 + local_8);
            puVar10 = (undefined4 *)&stack0xffffffd8;
            for (uVar6 = uVar3 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
              *puVar10 = *puVar9;
              puVar9 = puVar9 + 1;
              puVar10 = puVar10 + 1;
            }
            for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar9;
              puVar9 = (undefined4 *)((int)puVar9 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            uVar3 = *(uint *)((int)param_1 + 0x2c);
            puVar9 = (undefined4 *)(piVar4[1] * uVar3 + local_8);
            puVar10 = (undefined4 *)(&stack0xffffffd8 + uVar3);
            for (uVar6 = uVar3 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
              *puVar10 = *puVar9;
              puVar9 = puVar9 + 1;
              puVar10 = puVar10 + 1;
            }
            for (uVar6 = uVar3 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar9;
              puVar9 = (undefined4 *)((int)puVar9 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            uVar6 = *(uint *)((int)param_1 + 0x2c);
            puVar9 = (undefined4 *)(piVar4[2] * uVar6 + local_8);
            puVar10 = (undefined4 *)(uVar6 + (int)(&stack0xffffffd8 + uVar3));
            for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
              *puVar10 = *puVar9;
              puVar9 = puVar9 + 1;
              puVar10 = puVar10 + 1;
            }
            for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar9;
              puVar9 = (undefined4 *)((int)puVar9 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            iVar11 = (**(code **)(**(int **)((int)param_1 + 0x24) + 0x120))
                               (*(int **)((int)param_1 + 0x24),4,1,&stack0xffffffd8,
                                *(undefined4 *)((int)param_1 + 0x2c));
            if (iVar11 < 0) break;
          }
          local_c = local_c + 1;
          iVar8 = iVar8 + 0xc;
        } while (local_c < local_18);
      }
    }
    if (local_8 != 0) {
      (**(code **)(**(int **)((int)pvVar2 + 0x28) + 0x30))(*(int **)((int)pvVar2 + 0x28));
    }
  }
  if (*(int *)((int)pvVar2 + 0x40) != 0) {
    (**(code **)(**(int **)((int)pvVar2 + 0x3c) + 0x30))(*(int **)((int)pvVar2 + 0x3c));
    *(undefined4 *)((int)pvVar2 + 0x40) = 0;
  }
  return iVar11;
}

