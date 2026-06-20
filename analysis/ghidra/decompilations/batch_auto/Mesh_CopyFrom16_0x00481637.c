
void __thiscall Mesh_CopyFrom16(void *this,void *param_1)

{
  void *pvVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined2 *puVar5;
  undefined4 *puVar6;
  undefined2 *puVar7;
  undefined4 *puVar8;
  undefined4 *local_18;
  undefined4 *local_14;
  undefined4 *local_10;
  undefined4 *local_c;
  int local_8;
  
  local_c = (undefined4 *)0x0;
  local_10 = (undefined4 *)0x0;
  local_14 = (undefined4 *)0x0;
  local_18 = (undefined4 *)0x0;
  local_8 = Graphics_DrawIndexedPrimitive(this,&local_18,0);
  if ((-1 < local_8) &&
     (local_8 = Graphics_DrawIndexedPrimitive(param_1,&local_14,0x10), -1 < local_8)) {
    uVar2 = *(int *)((int)this + 0x58) * 6;
    puVar6 = local_14;
    puVar8 = local_18;
    for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
      *puVar8 = *puVar6;
      puVar6 = puVar6 + 1;
      puVar8 = puVar8 + 1;
    }
    for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
      *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
      puVar8 = (undefined4 *)((int)puVar8 + 1);
    }
    if (((*(byte *)((int)this + 0xc) & 1) != 0) && ((*(byte *)((int)param_1 + 0xc) & 1) != 0)) {
      uVar2 = *(int *)((int)this + 0x58) * 6;
      puVar6 = *(undefined4 **)((int)param_1 + 0x44);
      puVar8 = *(undefined4 **)((int)this + 0x44);
      for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar8 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar8 = puVar8 + 1;
      }
      for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    if (((*(byte *)((int)this + 0xc) & 4) != 0) && ((*(byte *)((int)param_1 + 0xc) & 4) != 0)) {
      puVar6 = *(undefined4 **)((int)param_1 + 0x48);
      puVar8 = *(undefined4 **)((int)this + 0x48);
      for (uVar3 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar8 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar8 = puVar8 + 1;
      }
      for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    if (((*(byte *)((int)this + 0xc) & 8) != 0) && ((*(byte *)((int)param_1 + 0xc) & 8) != 0)) {
      uVar2 = *(int *)((int)this + 0x58) << 1;
      puVar6 = *(undefined4 **)((int)param_1 + 0x4c);
      puVar8 = *(undefined4 **)((int)this + 0x4c);
      for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar8 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar8 = puVar8 + 1;
      }
      for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    if ((*(byte *)((int)param_1 + 0xc) & 0x10) != 0) {
      if (((*(byte *)((int)this + 0xc) & 0x10) == 0) ||
         (*(int *)((int)this + 0x54) != *(int *)((int)param_1 + 0x54))) {
        _free(*(void **)((int)this + 0x50));
        iVar4 = *(int *)((int)param_1 + 0x54);
        *(int *)((int)this + 0x54) = iVar4;
        pvVar1 = operator_new(iVar4 * 0x14);
        *(void **)((int)this + 0x50) = pvVar1;
        if (pvVar1 == (void *)0x0) {
          local_8 = -0x7ff8fff2;
          goto LAB_004817e8;
        }
        *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x10;
      }
      puVar6 = *(undefined4 **)((int)param_1 + 0x50);
      puVar8 = *(undefined4 **)((int)this + 0x50);
      for (uVar3 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar3 != 0; uVar3 = uVar3 - 1) {
        *puVar8 = *puVar6;
        puVar6 = puVar6 + 1;
        puVar8 = puVar8 + 1;
      }
      for (iVar4 = 0; iVar4 != 0; iVar4 = iVar4 + -1) {
        *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
        puVar6 = (undefined4 *)((int)puVar6 + 1);
        puVar8 = (undefined4 *)((int)puVar8 + 1);
      }
    }
    iVar4 = *(int *)((int)this + 0x30);
    if (iVar4 != 0) {
      if (((*(byte *)((int)this + 0xc) & 2) != 0) && ((*(byte *)((int)param_1 + 0xc) & 2) != 0)) {
        puVar5 = *(undefined2 **)((int)param_1 + 0x38);
        puVar7 = *(undefined2 **)((int)this + 0x38);
        for (; iVar4 != 0; iVar4 = iVar4 + -1) {
          *puVar7 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar7 = puVar7 + 1;
        }
      }
      if ((((*(byte *)((int)this + 0xc) & 0x20) == 0) &&
          (local_8 = Graphics_DrawIndexedPrimitiveUP(param_1,&local_10,0x10), -1 < local_8)) &&
         (local_8 = Graphics_DrawIndexedPrimitiveUP(this,&local_c,0), -1 < local_8)) {
        uVar2 = *(int *)((int)this + 0x2c) * *(int *)((int)this + 0x30);
        puVar6 = local_10;
        puVar8 = local_c;
        for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
          *puVar8 = *puVar6;
          puVar6 = puVar6 + 1;
          puVar8 = puVar8 + 1;
        }
        for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
          *(undefined1 *)puVar8 = *(undefined1 *)puVar6;
          puVar6 = (undefined4 *)((int)puVar6 + 1);
          puVar8 = (undefined4 *)((int)puVar8 + 1);
        }
      }
    }
  }
LAB_004817e8:
  if (local_c != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)this + 0x28) + 0x30))(*(int **)((int)this + 0x28));
  }
  if (local_10 != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x28) + 0x30))(*(int **)((int)param_1 + 0x28));
  }
  if (local_14 != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)param_1 + 0x3c) + 0x30))(*(int **)((int)param_1 + 0x3c));
  }
  if (local_18 != (undefined4 *)0x0) {
    (**(code **)(**(int **)((int)this + 0x3c) + 0x30))(*(int **)((int)this + 0x3c));
  }
  if (local_8 < 0) {
    *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) & 0xffffffef;
    *(undefined4 *)((int)this + 0x30) = 0;
    *(undefined4 *)((int)this + 0x58) = 0;
  }
  return;
}

