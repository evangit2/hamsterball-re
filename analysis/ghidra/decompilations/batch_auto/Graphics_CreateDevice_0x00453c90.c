
void __thiscall
Graphics_CreateDevice(void *this,undefined4 *param_1,undefined4 *param_2,undefined1 *param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  
  if (*(char *)((int)this + 4) != '\0') {
    puVar3 = (undefined1 *)0x4b;
    puVar2 = param_1;
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1);
    if (-1 < iVar1) goto LAB_00453d6e;
    puVar3 = (undefined1 *)0x4f;
    puVar2 = param_1;
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1);
    if (-1 < iVar1) goto LAB_00453dae;
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x4f);
    if (-1 < iVar1) goto LAB_00453dae;
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x49);
    if (-1 < iVar1) goto LAB_00453dd9;
  }
  puVar3 = (undefined1 *)0x47;
  puVar2 = param_1;
  iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                    (*(int **)((int)this + 0x7c),0,1,param_1);
  if (iVar1 < 0) {
    puVar3 = (undefined1 *)0x50;
    puVar2 = param_1;
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1);
    if (-1 < iVar1) {
LAB_00453e5a:
      *puVar2 = 0x50;
      *puVar3 = 0;
      return;
    }
    iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                      (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x4d);
    if (iVar1 < 0) {
      if (*(char *)((int)this + 4) == '\0') {
        puVar3 = (undefined1 *)0x4b;
        puVar2 = param_1;
        iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                          (*(int **)((int)this + 0x7c),0,1,param_1);
        if (-1 < iVar1) {
LAB_00453d6e:
          *puVar2 = 0x4b;
          *puVar3 = 1;
          return;
        }
        puVar3 = (undefined1 *)0x4f;
        puVar2 = param_1;
        iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                          (*(int **)((int)this + 0x7c),0,1,param_1);
        if (-1 < iVar1) {
LAB_00453dae:
          *puVar2 = 0x4f;
          *puVar3 = 1;
          return;
        }
        iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                          (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x4f);
        if (-1 < iVar1) goto LAB_00453dae;
        iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                          (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x49);
        if (-1 < iVar1) {
LAB_00453dd9:
          *param_2 = 0x49;
          *param_3 = 1;
          return;
        }
      }
      puVar3 = (undefined1 *)0x47;
      puVar2 = param_1;
      iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                        (*(int **)((int)this + 0x7c),0,1,param_1);
      if (-1 < iVar1) goto LAB_00453e04;
      puVar2 = param_1;
      iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                        (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x4d);
      if (iVar1 < 0) {
        puVar3 = (undefined1 *)0x453e56;
        iVar1 = (**(code **)(**(int **)((int)this + 0x7c) + 0x30))
                          (*(int **)((int)this + 0x7c),0,1,param_1,param_1,0x50);
        if (iVar1 < 0) {
          *puVar2 = 0;
          *puVar3 = 0;
          return;
        }
        goto LAB_00453e5a;
      }
    }
    *param_2 = 0x4d;
    *param_3 = 0;
    return;
  }
LAB_00453e04:
  *puVar2 = 0x47;
  *puVar3 = 0;
  return;
}

