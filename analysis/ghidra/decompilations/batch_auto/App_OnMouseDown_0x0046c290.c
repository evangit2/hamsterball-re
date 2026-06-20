
void __thiscall App_OnMouseDown(void *this,undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  ulonglong uVar4;
  ulonglong uVar5;
  
  *(int *)((int)this + 400) = *(int *)((int)this + 400) + 1;
  SetCapture(*(HWND *)((int)this + 8));
  if (param_3 == 0) {
    *(undefined1 *)((int)this + 0x1c8) = 1;
  }
  else if (param_3 == 1) {
    *(undefined1 *)((int)this + 0x1c9) = 1;
  }
  else if (param_3 == 2) {
    *(undefined1 *)((int)this + 0x1ca) = 1;
  }
  uVar4 = __ftol2();
  *(int *)((int)this + 0x1b8) = (int)uVar4;
  uVar5 = __ftol2();
  *(int *)((int)this + 0x1bc) = (int)uVar5;
  if (*(int **)((int)this + 0x1b0) != (int *)0x0) {
    *(char **)((int)this + 0x210) = "MouseDown";
    *(char **)((int)this + 0x20c) = "Mouse Interceptor";
    (**(code **)(**(int **)((int)this + 0x1b0) + 0xc))(param_1,param_2,param_3);
    return;
  }
  uVar2 = UIWidget_HitTest(*(void **)((int)this + 0x184),(int)uVar4,(int)uVar5);
  *(uint *)((int)this + 0x188) = uVar2;
  if (uVar2 != 0) {
    *(char **)((int)this + 0x210) = "MouseDown";
    *(undefined4 *)((int)this + 0x20c) = *(undefined4 *)(uVar2 + 0x868);
    if (param_3 == 0) {
      *(undefined1 *)(uVar2 + 0x86c) = 1;
    }
    else if (param_3 == 1) {
      *(undefined1 *)(uVar2 + 0x86d) = 1;
    }
    else if (param_3 == 2) {
      *(undefined1 *)(uVar2 + 0x86e) = 1;
    }
    iVar1 = **(int **)((int)this + 0x188);
    uVar4 = __ftol2();
    uVar3 = (undefined4)uVar4;
    uVar4 = __ftol2();
    (**(code **)(iVar1 + 0x14))((int)uVar4,uVar3,param_3);
  }
  return;
}

