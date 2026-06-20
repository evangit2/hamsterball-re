
void __thiscall Input_OnMouseUp(void *this,undefined4 param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  ulonglong uVar4;
  int iVar5;
  
  iVar3 = *(int *)((int)this + 400) + -1;
  *(int *)((int)this + 400) = iVar3;
  if (iVar3 < 1) {
    *(undefined4 *)((int)this + 400) = 0;
    ReleaseCapture();
  }
  if (*(int **)((int)this + 0x1b0) != (int *)0x0) {
    *(char **)((int)this + 0x210) = "MouseUp";
    *(char **)((int)this + 0x20c) = "Mouse Interceptor";
    (**(code **)(**(int **)((int)this + 0x1b0) + 0x10))();
    *(undefined4 *)((int)this + 0x188) = 0;
    return;
  }
  if (*(int *)((int)this + 0x188) != 0) {
    uVar1 = AthenaList_ContainsValue
                      ((void *)(*(int *)((int)this + 0x184) + 4),*(int *)((int)this + 0x188));
    if ((char)uVar1 != '\0') {
      iVar3 = *(int *)((int)this + 0x188);
      *(char **)((int)this + 0x210) = "MouseUp";
      *(undefined4 *)((int)this + 0x20c) = *(undefined4 *)(iVar3 + 0x868);
      if (param_3 == 0) {
        *(undefined1 *)(iVar3 + 0x86c) = 0;
      }
      else if (param_3 == 1) {
        *(undefined1 *)(iVar3 + 0x86d) = 0;
      }
      else if (param_3 == 2) {
        *(undefined1 *)(iVar3 + 0x86e) = 0;
      }
      iVar3 = **(int **)((int)this + 0x188);
      iVar5 = param_3;
      uVar4 = __ftol2();
      uVar2 = (undefined4)uVar4;
      uVar4 = __ftol2();
      (**(code **)(iVar3 + 0x18))((int)uVar4,uVar2,iVar5);
    }
    *(undefined4 *)((int)this + 0x188) = 0;
  }
  if (param_3 == 0) {
    *(undefined1 *)((int)this + 0x1c8) = 0;
  }
  else {
    if (param_3 == 1) {
      *(undefined1 *)((int)this + 0x1c9) = 0;
      return;
    }
    if (param_3 == 2) {
      *(undefined1 *)((int)this + 0x1ca) = 0;
      return;
    }
  }
  return;
}

