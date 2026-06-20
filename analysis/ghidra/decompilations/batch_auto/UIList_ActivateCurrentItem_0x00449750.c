
void __thiscall
UIList_ActivateCurrentItem(void *this,undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  ulonglong uVar3;
  undefined4 uVar4;
  
  Gadget_Activate(this,param_1,param_2,param_3);
  if (param_3 != 0) {
    return;
  }
  iVar1 = *(int *)((int)this + 0x864);
  if (iVar1 == *(int *)((int)this + 0xca4)) {
    uVar4 = 0x28a;
  }
  else {
    if (iVar1 != *(int *)((int)this + 0xca8)) {
      iVar2 = *(int *)((int)this + 0xcc0);
      if (iVar2 == 0) {
        return;
      }
      if (iVar1 != *(int *)(iVar2 + 0x1c)) {
        return;
      }
      (**(code **)(*(int *)this + 0x48))(*(undefined4 *)(iVar2 + 4));
      return;
    }
    uVar4 = 0x32;
  }
  iVar1 = *(int *)this;
  uVar3 = __ftol2();
  (**(code **)(iVar1 + 0xc))(100 - (int)uVar3,uVar4);
  return;
}

