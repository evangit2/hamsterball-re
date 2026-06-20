
void __thiscall MeshWorld_RemoveObject(void *this,int *param_1)

{
  undefined4 uVar1;
  
  (**(code **)(*(int *)((int)this + 0x428) + 0xc))(param_1);
  if (*(int **)((int)this + 0x41c) == param_1) {
    *(undefined4 *)((int)this + 0x41c) = 0;
  }
  if (*(int **)((int)this + 0x424) == param_1) {
    uVar1 = (**(code **)(*(int *)((int)this + 0x428) + 8))();
    *(undefined4 *)((int)this + 0x424) = uVar1;
  }
  thunk_Gfx_SetRenderState((void *)((int)this + 4),(int)param_1);
  param_1[0xc] = 0;
  (**(code **)(**(int **)((int)this + 0x844) + 0x74))();
  (**(code **)(*param_1 + 0x3c))(this);
  return;
}

