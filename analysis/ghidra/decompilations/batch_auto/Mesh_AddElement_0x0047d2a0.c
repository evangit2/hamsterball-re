
void __thiscall Mesh_AddElement(void *this,float *param_1)

{
  int iVar1;
  
  iVar1 = Mesh_FindElement(this,param_1);
  if (iVar1 == -1) {
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + *(int *)((int)this + 0x20)) = *param_1;
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 4 + *(int *)((int)this + 0x20)) = param_1[1];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 8 + *(int *)((int)this + 0x20)) = param_1[2];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 0xc + *(int *)((int)this + 0x20)) = param_1[3];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 0x10 + *(int *)((int)this + 0x20)) = param_1[4];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 0x14 + *(int *)((int)this + 0x20)) = param_1[5];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 0x18 + *(int *)((int)this + 0x20)) = param_1[6];
    *(float *)(*(int *)((int)this + 0x1c) * 0x424 + 0x1c + *(int *)((int)this + 0x20)) = param_1[7];
    *(undefined4 *)(*(int *)((int)this + 0x1c) * 0x424 + 0x20 + *(int *)((int)this + 0x20)) = 0;
    *(int *)((int)this + 0x1c) = *(int *)((int)this + 0x1c) + 1;
  }
  return;
}

