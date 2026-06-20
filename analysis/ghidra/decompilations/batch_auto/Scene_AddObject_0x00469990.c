
void __thiscall Scene_AddObject(void *this,int *param_1)

{
  uint uVar1;
  
  uVar1 = AthenaList_ContainsValue((void *)((int)this + 4),(int)param_1);
  if ((char)uVar1 == '\0') {
    AthenaList_Append((void *)((int)this + 4),(int)param_1);
    param_1[0xc] = (int)this;
    (**(code **)(**(int **)((int)this + 0x844) + 0x74))();
    (**(code **)(*param_1 + 0x38))(this);
  }
  return;
}

