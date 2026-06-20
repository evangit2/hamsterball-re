
void __thiscall Vector_Resize(void *this,uint param_1)

{
  uint uVar1;
  int iVar2;
  
  iVar2 = *(int *)((int)this + 4);
  if (iVar2 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(int *)((int)this + 8) - iVar2 >> 2;
  }
  if (uVar1 < param_1) {
    if (iVar2 == 0) {
      iVar2 = 0;
    }
    else {
      iVar2 = *(int *)((int)this + 8) - iVar2 >> 2;
    }
    Vector_InsertN(this,*(undefined4 **)((int)this + 8),(undefined4 *)(param_1 - iVar2),
                   (undefined4 *)&stack0x00000008);
    return;
  }
  if ((iVar2 != 0) && (param_1 < (uint)((int)*(undefined4 **)((int)this + 8) - iVar2 >> 2))) {
    Vector_InsertRange(this,&param_1,(undefined4 *)(*(int *)((int)this + 4) + param_1 * 4),
                       *(undefined4 **)((int)this + 8));
  }
  return;
}

