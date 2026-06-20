
void __thiscall Pendulum_PlayCollisionSound(void *this,int param_1)

{
  void *this_00;
  int iVar1;
  uint uVar2;
  
  this_00 = (void *)((int)this + 0x10f0);
  iVar1 = AthenaList_GetSize((int)this_00);
  if (iVar1 == 0) {
    Sound_Play3D(*(void **)(*(int *)(*(int *)((int)this + 0x10d0) + 0x878) + 0x508),
                 *(float *)((int)this + 0x10d4),*(float *)((int)this + 0x10d8),
                 *(float *)((int)this + 0x10dc));
    *(undefined4 *)((int)this + 0x1508) = 0;
  }
  uVar2 = AthenaList_ContainsValue(this_00,param_1);
  if ((char)uVar2 == '\0') {
    AthenaList_Append(this_00,param_1);
  }
  *(undefined4 *)(param_1 + 0x808) = 10;
  return;
}

