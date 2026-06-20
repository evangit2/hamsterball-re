
void __thiscall Mesh_CopyFrom(void *this,void *param_1)

{
  void *pvVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined4 *puVar5;
  undefined4 *puVar6;
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
    puVar5 = local_14;
    puVar6 = local_18;
    for (uVar2 = *(int *)((int)this + 0x58) * 3 & 0x3fffffff; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar6 = *puVar5;
      puVar5 = puVar5 + 1;
      puVar6 = puVar6 + 1;
    }
    for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
    if (((*(byte *)((int)this + 0xc) & 1) != 0) && ((*(byte *)((int)param_1 + 0xc) & 1) != 0)) {
      puVar5 = *(undefined4 **)((int)param_1 + 0x44);
      puVar6 = *(undefined4 **)((int)this + 0x44);
      for (uVar2 = *(int *)((int)this + 0x58) * 3 & 0x3fffffff; uVar2 != 0; uVar2 = uVar2 - 1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
        *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
    }
    if (((*(byte *)((int)this + 0xc) & 4) != 0) && ((*(byte *)((int)param_1 + 0xc) & 4) != 0)) {
      puVar5 = *(undefined4 **)((int)param_1 + 0x48);
      puVar6 = *(undefined4 **)((int)this + 0x48);
      for (uVar2 = *(uint *)((int)this + 0x58) & 0x3fffffff; uVar2 != 0; uVar2 = uVar2 - 1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
        *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
    }
    if (((*(byte *)((int)this + 0xc) & 8) != 0) && ((*(byte *)((int)param_1 + 0xc) & 8) != 0)) {
      uVar4 = *(int *)((int)this + 0x58) << 1;
      puVar5 = *(undefined4 **)((int)param_1 + 0x4c);
      puVar6 = *(undefined4 **)((int)this + 0x4c);
      for (uVar2 = uVar4 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
    }
    if ((*(byte *)((int)param_1 + 0xc) & 0x10) != 0) {
      if (((*(byte *)((int)this + 0xc) & 0x10) == 0) ||
         (*(int *)((int)this + 0x54) != *(int *)((int)param_1 + 0x54))) {
        _free(*(void **)((int)this + 0x50));
        iVar3 = *(int *)((int)param_1 + 0x54);
        *(int *)((int)this + 0x54) = iVar3;
        pvVar1 = operator_new(iVar3 * 0x14);
        *(void **)((int)this + 0x50) = pvVar1;
        if (pvVar1 == (void *)0x0) {
          local_8 = -0x7ff8fff2;
          goto LAB_004815e2;
        }
        *(uint *)((int)this + 0xc) = *(uint *)((int)this + 0xc) | 0x10;
      }
      puVar5 = *(undefined4 **)((int)param_1 + 0x50);
      puVar6 = *(undefined4 **)((int)this + 0x50);
      for (uVar2 = *(int *)((int)this + 0x54) * 5 & 0x3fffffff; uVar2 != 0; uVar2 = uVar2 - 1) {
        *puVar6 = *puVar5;
        puVar5 = puVar5 + 1;
        puVar6 = puVar6 + 1;
      }
      for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
        *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
        puVar5 = (undefined4 *)((int)puVar5 + 1);
        puVar6 = (undefined4 *)((int)puVar6 + 1);
      }
    }
    iVar3 = *(int *)((int)this + 0x30);
    if (iVar3 != 0) {
      if (((*(byte *)((int)this + 0xc) & 2) != 0) && ((*(byte *)((int)param_1 + 0xc) & 2) != 0)) {
        puVar5 = *(undefined4 **)((int)param_1 + 0x38);
        puVar6 = *(undefined4 **)((int)this + 0x38);
        for (; iVar3 != 0; iVar3 = iVar3 + -1) {
          *puVar6 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar6 = puVar6 + 1;
        }
      }
      if ((((*(byte *)((int)this + 0xc) & 0x20) == 0) &&
          (local_8 = Graphics_DrawIndexedPrimitiveUP(param_1,&local_10,0x10), -1 < local_8)) &&
         (local_8 = Graphics_DrawIndexedPrimitiveUP(this,&local_c,0), -1 < local_8)) {
        uVar4 = *(int *)((int)this + 0x2c) * *(int *)((int)this + 0x30);
        puVar5 = local_10;
        puVar6 = local_c;
        for (uVar2 = uVar4 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
          *puVar6 = *puVar5;
          puVar5 = puVar5 + 1;
          puVar6 = puVar6 + 1;
        }
        for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined1 *)puVar6 = *(undefined1 *)puVar5;
          puVar5 = (undefined4 *)((int)puVar5 + 1);
          puVar6 = (undefined4 *)((int)puVar6 + 1);
        }
      }
    }
  }
LAB_004815e2:
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

