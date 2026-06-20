
void __fastcall CRT_SharedMemCloseIfValid(int *param_1)

{
  if (*param_1 != -1) {
    CRT_SharedMemClose(param_1);
    return;
  }
  return;
}

