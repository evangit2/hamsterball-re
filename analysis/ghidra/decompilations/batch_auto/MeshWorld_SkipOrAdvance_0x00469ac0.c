
void __thiscall MeshWorld_SkipOrAdvance(void *this,int param_1,char param_2)

{
  undefined4 uVar1;
  undefined4 unaff_retaddr;
  
  if (param_2 != '\0') {
    (**(code **)(*(int *)((int)this + 0x428) + 4))(*(undefined4 *)((int)this + 0x424));
    *(undefined4 *)((int)this + 0x424) = unaff_retaddr;
    return;
  }
  if (*(int *)((int)this + 0x424) == param_1) {
    uVar1 = (**(code **)(*(int *)((int)this + 0x428) + 8))();
    *(undefined4 *)((int)this + 0x424) = uVar1;
  }
  return;
}

