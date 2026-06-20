
void __thiscall AthenaList_InsertAt(void *this,undefined4 param_1,int param_2)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  
  iVar3 = *(int *)((int)this + 4);
  if (iVar3 < param_2) {
    param_2 = iVar3;
  }
  puVar5 = *(undefined4 **)((int)this + 0x40c);
  *(int *)((int)this + 4) = iVar3 + 1;
  puVar1 = _malloc((iVar3 + 1) * 4);
  puVar2 = puVar1;
  iVar3 = 0;
  iVar4 = param_2;
  if (0 < param_2) {
    do {
      *puVar2 = *puVar5;
      puVar2 = puVar2 + 1;
      puVar5 = puVar5 + 1;
      iVar4 = iVar4 + -1;
      iVar3 = param_2;
    } while (iVar4 != 0);
  }
  *puVar2 = param_1;
  iVar3 = iVar3 + 1;
  if (iVar3 < *(int *)((int)this + 4)) {
    do {
      puVar2 = puVar2 + 1;
      *puVar2 = *puVar5;
      iVar3 = iVar3 + 1;
      puVar5 = puVar5 + 1;
    } while (iVar3 < *(int *)((int)this + 4));
  }
  if (*(void **)((int)this + 0x40c) != (void *)0x0) {
    _free(*(void **)((int)this + 0x40c));
  }
  *(undefined4 **)((int)this + 0x40c) = puVar1;
  return;
}

