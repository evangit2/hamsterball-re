
void __thiscall Scene_HandleInput(void *this,undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  char cVar2;
  int iVar3;
  int *piVar4;
  undefined4 local_4;
  
  local_4 = *(undefined4 *)((int)this + 0x870);
  piVar4 = (int *)0x0;
  *(undefined4 *)((int)this + 0x864) = 0;
  iVar3 = AthenaList_NextIndex((int)this + 0x44c);
  *(undefined4 *)((int)this + iVar3 * 4 + 0x454) = 0;
  if (0 < *(int *)((int)this + 0x450)) {
    piVar4 = (int *)**(undefined4 **)((int)this + 0x858);
    *(undefined4 *)((int)this + iVar3 * 4 + 0x454) = 1;
  }
  while (piVar4 != (int *)0x0) {
    if ((*(char *)((int)piVar4 + 0x16) != '\0') &&
       (cVar2 = (**(code **)(*piVar4 + 4))(param_1,param_2), cVar2 != '\0')) {
      if ((char)piVar4[5] != '\0') {
        local_4 = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1e4);
      }
      if (*(char *)((int)piVar4 + 0x15) != '\0') {
        local_4 = *(undefined4 *)(*(int *)((int)this + 0x14) + 0x1e8);
      }
      *(int **)((int)this + 0x864) = piVar4;
    }
    iVar1 = *(int *)((int)this + iVar3 * 4 + 0x454);
    if (*(int *)((int)this + 0x450) <= iVar1) break;
    piVar4 = *(int **)(*(int *)((int)this + 0x858) + iVar1 * 4);
    *(int *)((int)this + iVar3 * 4 + 0x454) = iVar1 + 1;
  }
  (**(code **)(**(int **)((int)this + 0x14) + 0x7c))(local_4);
  return;
}

