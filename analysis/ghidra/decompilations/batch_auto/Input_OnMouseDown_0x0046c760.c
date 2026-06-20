
void __thiscall Input_OnMouseDown(void *this,undefined4 param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  ulonglong uVar4;
  
  piVar2 = (int *)UIWidget_HitTest(*(void **)((int)this + 0x184),*(int *)((int)this + 0x1b8),
                                   *(int *)((int)this + 0x1bc));
  if (piVar2 != (int *)0x0) {
    iVar1 = *piVar2;
    uVar4 = __ftol2();
    uVar3 = (undefined4)uVar4;
    uVar4 = __ftol2();
    (**(code **)(iVar1 + 0x1c))((int)uVar4,uVar3,param_1);
  }
  return;
}

