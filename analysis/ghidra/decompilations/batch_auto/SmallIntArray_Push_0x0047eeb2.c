
void __thiscall SmallIntArray_Push(void *this,undefined4 param_1)

{
  *(undefined4 *)(*(int *)((int)this + 0x10) + *(int *)((int)this + 4) * 4) = param_1;
  *(int *)((int)this + 8) = *(int *)((int)this + 8) + 1;
  *(int *)((int)this + 4) = *(int *)((int)this + 4) + 1;
  if (*(int *)((int)this + 4) == *(int *)((int)this + 0xc)) {
    *(undefined4 *)((int)this + 4) = 0;
  }
  return;
}

