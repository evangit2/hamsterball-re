
void __thiscall IndexList_SetRange(void *this,int param_1,int param_2)

{
  *(int *)((int)this + 0x410) = param_2 - param_1;
  return;
}

