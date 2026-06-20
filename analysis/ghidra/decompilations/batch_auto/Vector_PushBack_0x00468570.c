
void __thiscall Vector_PushBack(void *this,undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  
  iVar1 = *(int *)((int)this + 4);
  if ((iVar1 != 0) &&
     ((uint)(*(int *)((int)this + 8) - iVar1 >> 2) < (uint)(*(int *)((int)this + 0xc) - iVar1 >> 2))
     ) {
    puVar2 = *(undefined4 **)((int)this + 8);
    Array_FillDWords(puVar2,1,param_1);
    *(undefined4 **)((int)this + 8) = puVar2 + 1;
    return;
  }
  Vector_InsertN(this,*(undefined4 **)((int)this + 8),(undefined4 *)0x1,param_1);
  return;
}

