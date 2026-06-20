
void __thiscall ShortArray_Push(void *this,undefined2 param_1)

{
  *(undefined2 *)(*(int *)((int)this + 0x10) + *(int *)((int)this + 4) * 2) = param_1;
  *(int *)((int)this + 8) = *(int *)((int)this + 8) + 1;
  *(int *)((int)this + 4) = *(int *)((int)this + 4) + 1;
  if (*(int *)((int)this + 4) == *(int *)((int)this + 0xc)) {
    *(undefined4 *)((int)this + 4) = 0;
  }
  return;
}

