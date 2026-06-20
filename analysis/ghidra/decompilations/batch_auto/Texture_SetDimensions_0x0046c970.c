
void __thiscall Texture_SetDimensions(void *this,int param_1,int param_2)

{
  *(int *)((int)this + 0x1f0) = param_1;
  *(int *)((int)this + 500) = param_2;
  if ((*(int *)((int)this + 0x15c) == -1) && (*(int *)((int)this + 0x160) == -1)) {
    *(undefined4 *)((int)this + 0x1f8) = 0x3f800000;
    *(undefined4 *)((int)this + 0x1fc) = 0x3f800000;
    return;
  }
  *(float *)((int)this + 0x1f8) = (float)*(int *)((int)this + 0x15c) / (float)param_1;
  *(float *)((int)this + 0x1fc) = (float)*(int *)((int)this + 0x160) / (float)param_2;
  return;
}

