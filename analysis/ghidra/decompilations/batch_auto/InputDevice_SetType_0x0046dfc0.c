
void __thiscall InputDevice_SetType(void *this,undefined4 param_1)

{
  int iVar1;
  
  *(undefined4 *)((int)this + 8) = param_1;
  *(undefined4 *)((int)this + 0xc) = 0x3f800000;
  switch(param_1) {
  case 1:
    *(undefined4 *)((int)this + 0x10) = *(undefined4 *)(*(int *)((int)this + 4) + 0x434);
    *(char **)this = "Keyboard";
    break;
  case 2:
    *(undefined4 *)((int)this + 0x10) = *(undefined4 *)(*(int *)((int)this + 4) + 0x434);
    *(char **)this = "Mouse/Trackball";
    break;
  case 4:
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x424);
    *(int *)((int)this + 0x10) = iVar1;
    if (iVar1 != 0) {
      *(int *)this = *(int *)(*(int *)((int)this + 4) + 0x424) + 0xc;
    }
    break;
  case 5:
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x428);
    *(int *)((int)this + 0x10) = iVar1;
    if (iVar1 == 0) break;
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x428);
    goto LAB_0046e072;
  case 6:
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x42c);
    *(int *)((int)this + 0x10) = iVar1;
    if (iVar1 != 0) {
      *(int *)this = *(int *)(*(int *)((int)this + 4) + 0x42c) + 0xc;
    }
    break;
  case 7:
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x430);
    *(int *)((int)this + 0x10) = iVar1;
    if (iVar1 == 0) break;
    iVar1 = *(int *)(*(int *)((int)this + 4) + 0x430);
LAB_0046e072:
    *(int *)this = iVar1 + 0xc;
  }
  if (*(int *)((int)this + 0x10) == 0) {
    *(undefined4 *)((int)this + 8) = 0;
    *(char **)this = "Not Connected";
  }
  return;
}

