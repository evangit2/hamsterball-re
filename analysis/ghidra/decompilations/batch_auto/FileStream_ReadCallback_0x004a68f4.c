
void __cdecl FileStream_ReadCallback(int *param_1,undefined4 param_2,undefined4 param_3)

{
  if ((code *)param_1[0x14] != (code *)0x0) {
    (*(code *)param_1[0x14])(param_1,param_2,param_3);
    return;
  }
  longjmp_with_cleanup(param_1,"Call to NULL read function");
  return;
}

