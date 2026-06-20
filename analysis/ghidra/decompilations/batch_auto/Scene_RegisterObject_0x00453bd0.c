
void __thiscall Scene_RegisterObject(void *this,int param_1,int *param_2)

{
  param_2[0x23] = param_1;
  (**(code **)(*param_2 + 0xc))();
  *(int **)((int)this + param_1 * 4 + 0x710) = param_2;
  return;
}

