
void __thiscall UIList_HandleKeyNav(void *this,undefined4 param_1,int param_2)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar5 = param_2 - *(int *)((int)this + 0xccc);
  Scene_HandleInput(this,param_1,iVar5);
  if (*(int *)((int)this + 0x864) != 0) {
    iVar3 = AthenaList_NextIndex((int)this + 0x88c);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x894) = 0;
    if (*(int *)((int)this + 0x890) < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = **(int **)((int)this + 0xc98);
      *(undefined4 *)((int)this + iVar3 * 4 + 0x894) = 1;
    }
    while (iVar4 != 0) {
      if (*(int *)(iVar4 + 0x1c) == *(int *)((int)this + 0x864)) {
        *(int *)((int)this + 0xcc0) = iVar4;
      }
      iVar1 = *(int *)((int)this + iVar3 * 4 + 0x894);
      if (*(int *)((int)this + 0x890) <= iVar1) break;
      iVar4 = *(int *)(*(int *)((int)this + 0xc98) + iVar1 * 4);
      *(int *)((int)this + iVar3 * 4 + 0x894) = iVar1 + 1;
    }
  }
  if (*(char *)((int)this + 0xcd0) != '\0') {
    iVar5 = iVar5 + *(int *)((int)this + 0xccc);
    cVar2 = (**(code **)(**(int **)((int)this + 0xca4) + 4))(param_1,iVar5);
    if ((cVar2 != '\0') && (*(int *)((int)this + 0xccc) != *(int *)((int)this + 0xcc8))) {
      (**(code **)(**(int **)((int)this + 0x14) + 0x7c))((*(int **)((int)this + 0x14))[0x79]);
      *(undefined4 *)((int)this + 0x864) = *(undefined4 *)((int)this + 0xca4);
    }
    cVar2 = (**(code **)(**(int **)((int)this + 0xca8) + 4))(param_1,iVar5);
    if ((cVar2 != '\0') && (*(int *)((int)this + 0xccc) != *(int *)((int)this + 0xcc4))) {
      (**(code **)(**(int **)((int)this + 0x14) + 0x7c))((*(int **)((int)this + 0x14))[0x79]);
      *(undefined4 *)((int)this + 0x864) = *(undefined4 *)((int)this + 0xca8);
    }
  }
  return;
}

