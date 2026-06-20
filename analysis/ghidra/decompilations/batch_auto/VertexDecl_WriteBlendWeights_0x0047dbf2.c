
void __thiscall VertexDecl_WriteBlendWeights(void *this,int param_1,undefined4 param_2)

{
  if (((*(byte *)((int)this + 1) & 0x10) != 0) && (*(int *)((int)this + 0x38) != 0)) {
    *(undefined4 *)(param_1 + 8 + *(int *)((int)this + 0x38) * 4) = param_2;
  }
  return;
}

