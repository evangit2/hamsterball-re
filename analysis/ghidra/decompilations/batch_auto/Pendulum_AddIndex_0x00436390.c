
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall Pendulum_AddIndex(void *this,int param_1)

{
  void *this_00;
  float fVar1;
  int iVar2;
  uint uVar3;
  
  this_00 = (void *)((int)this + 0x2c);
  iVar2 = AthenaList_GetSize((int)this_00);
  if (iVar2 == 0) {
    iVar2 = RNG_Rand(&PTR_OBJ_VTABLE,2,'\0');
    if (iVar2 == 0) {
      fVar1 = *(float *)((int)this + 0x28) + _DAT_004d0418;
    }
    else {
      fVar1 = *(float *)((int)this + 0x28) - _DAT_004d0418;
    }
    *(float *)((int)this + 0x28) = fVar1;
  }
  uVar3 = AthenaList_ContainsValue(this_00,param_1);
  if ((char)uVar3 == '\0') {
    AthenaList_Append(this_00,param_1);
  }
  return;
}

