
void __thiscall VertexDecl_CopyToBuffer(void *this,int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  
  if (*(int *)((int)this + 8) != 0) {
    puVar1 = (undefined4 *)(*(int *)((int)this + 8) + param_1);
    *puVar1 = *param_2;
    puVar1[1] = param_2[1];
    puVar1[2] = param_2[2];
  }
  return;
}

