
void __thiscall FUN_004499d0(void *this,int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  
  if (param_1 == 0x410) {
    iVar1 = AthenaList_FindByValue((void *)((int)this + 0x88c),*(int *)((int)this + 0xcc0));
    if (iVar1 == -1) {
      iVar1 = 0;
    }
    else {
      do {
        iVar1 = iVar1 + 1;
        iVar2 = AthenaList_GetSize((int)this + 0x88c);
        if (iVar1 == iVar2) {
          iVar1 = 0;
        }
        if (iVar1 < 0) {
          piVar3 = (int *)0x0;
        }
        else if (iVar1 < *(int *)((int)this + 0x890)) {
          piVar3 = *(int **)(*(int *)((int)this + 0xc98) + iVar1 * 4);
        }
        else {
          piVar3 = (int *)0x0;
        }
      } while (*piVar3 == 0);
    }
    if ((-1 < iVar1) && (iVar1 < *(int *)((int)this + 0x890))) {
      *(undefined4 *)((int)this + 0xcc0) = *(undefined4 *)(*(int *)((int)this + 0xc98) + iVar1 * 4);
      return;
    }
    *(undefined4 *)((int)this + 0xcc0) = 0;
    return;
  }
  if (param_1 != 0x40e) {
    if (param_1 == 0x40d) {
      uVar4 = 0xffffffff;
    }
    else {
      if (param_1 != 0x40f) {
        return;
      }
      uVar4 = 1;
    }
    (**(code **)(*(int *)this + 0x4c))(*(undefined4 *)(*(int *)((int)this + 0xcc0) + 4),uVar4);
    return;
  }
  iVar1 = AthenaList_FindByValue((void *)((int)this + 0x88c),*(int *)((int)this + 0xcc0));
  if (iVar1 == -1) {
    iVar1 = 0;
  }
  else {
    do {
      iVar1 = iVar1 + -1;
      if (iVar1 < 0) {
        iVar1 = AthenaList_GetSize((int)this + 0x88c);
        iVar1 = iVar1 + -1;
      }
      if (iVar1 < 0) {
        piVar3 = (int *)0x0;
      }
      else if (iVar1 < *(int *)((int)this + 0x890)) {
        piVar3 = *(int **)(*(int *)((int)this + 0xc98) + iVar1 * 4);
      }
      else {
        piVar3 = (int *)0x0;
      }
    } while (*piVar3 == 0);
  }
  if ((-1 < iVar1) && (iVar1 < *(int *)((int)this + 0x890))) {
    *(undefined4 *)((int)this + 0xcc0) = *(undefined4 *)(*(int *)((int)this + 0xc98) + iVar1 * 4);
    return;
  }
  *(undefined4 *)((int)this + 0xcc0) = 0;
  return;
}

