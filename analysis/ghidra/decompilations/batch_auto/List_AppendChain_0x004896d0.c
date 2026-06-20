
void __fastcall List_AppendChain(int param_1)

{
  int iVar1;
  int in_EAX;
  
  if (param_1 != 0) {
    iVar1 = *(int *)(param_1 + 0xc);
    while (iVar1 != 0) {
      param_1 = *(int *)(param_1 + 0xc);
      iVar1 = *(int *)(param_1 + 0xc);
    }
    *(int *)(param_1 + 0xc) = in_EAX;
    if (in_EAX == 0) {
      return;
    }
    for (iVar1 = *(int *)(in_EAX + 0xc); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0xc)) {
    }
  }
  return;
}

