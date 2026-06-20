
void __thiscall Gadget_Activate(void *this,undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  ulonglong uVar3;
  
  if (*(int **)((int)this + 0x864) != (int *)0x0) {
    iVar1 = **(int **)((int)this + 0x864);
    uVar3 = __ftol2();
    uVar2 = (undefined4)uVar3;
    uVar3 = __ftol2();
    (**(code **)(iVar1 + 8))((int)uVar3,uVar2,param_3);
  }
  return;
}

