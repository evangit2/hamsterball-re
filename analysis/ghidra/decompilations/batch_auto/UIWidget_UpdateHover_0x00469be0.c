
undefined4 __thiscall UIWidget_UpdateHover(void *this,int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  
  piVar1 = *(int **)((int)this + 0x41c);
  piVar2 = (int *)UIWidget_HitTest(this,param_1,param_2);
  *(int **)((int)this + 0x41c) = piVar2;
  if (piVar1 != piVar2) {
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x30))();
    }
    if (*(int **)((int)this + 0x41c) != (int *)0x0) {
      (**(code **)(**(int **)((int)this + 0x41c) + 0x2c))();
    }
  }
  return *(undefined4 *)((int)this + 0x41c);
}

