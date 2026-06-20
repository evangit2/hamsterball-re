
void __thiscall Graphics_BeginFrame(void *this,int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  if (param_1 == 0) {
    param_1 = (int)this + 0x74c;
  }
  (**(code **)(**(int **)((int)this + 0x154) + 0x94))
            (*(int **)((int)this + 0x154),0x100,(undefined4 *)(param_1 + 4));
  puVar2 = (undefined4 *)(param_1 + 4);
  puVar3 = (undefined4 *)((int)this + 0x224);
  for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  Matrix_ComputeFrustum(*(int *)((int)this + 0x748));
  return;
}

