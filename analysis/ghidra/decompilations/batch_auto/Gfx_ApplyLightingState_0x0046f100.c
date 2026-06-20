
void __thiscall Gfx_ApplyLightingState(void *this,int param_1)

{
  char cVar1;
  int *piVar2;
  undefined4 uVar3;
  
  cVar1 = *(char *)(param_1 + 0x1c);
  if (*(char *)((int)this + 0x700) != cVar1) {
    *(char *)((int)this + 0x700) = cVar1;
    piVar2 = *(int **)((int)this + 0x154);
    *(int *)((int)this + 0x7c8) = *(int *)((int)this + 0x7c8) + 1;
    if (cVar1 == '\x01') {
      (**(code **)(*piVar2 + 200))(piVar2,0x1b,1);
      uVar3 = 4;
    }
    else {
      (**(code **)(*piVar2 + 200))(piVar2,0x1b,0);
      uVar3 = 2;
    }
    (**(code **)(**(int **)((int)this + 0x154) + 0xfc))(*(int **)((int)this + 0x154),0,4,uVar3);
  }
  cVar1 = *(char *)(param_1 + 0x1e);
  if (*(char *)((int)this + 5) != cVar1) {
    piVar2 = *(int **)((int)this + 0x154);
    if (cVar1 == '\0') {
      (**(code **)(*piVar2 + 0xfc))(piVar2,0,0x10,2);
      uVar3 = 2;
    }
    else {
      (**(code **)(*piVar2 + 0xfc))(piVar2,0,0x10,1);
      uVar3 = 1;
    }
    (**(code **)(**(int **)((int)this + 0x154) + 0xfc))(*(int **)((int)this + 0x154),0,0x11,uVar3);
    *(char *)((int)this + 5) = cVar1;
  }
  (**(code **)(**(int **)((int)this + 0x154) + 0xf4))
            (*(int **)((int)this + 0x154),0,*(undefined4 *)(param_1 + 4));
  (**(code **)(**(int **)((int)this + 0x154) + 200))(*(int **)((int)this + 0x154),0x39,0);
  return;
}

