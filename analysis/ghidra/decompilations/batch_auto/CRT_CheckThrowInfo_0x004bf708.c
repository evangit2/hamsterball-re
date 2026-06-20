
int CRT_CheckThrowInfo(int *param_1)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = (int *)*param_1;
  if (((*piVar1 == -0x1f928c9d) && (piVar1[4] == 3)) && (piVar1[5] == 0x19930520)) {
                    /* WARNING: Subroutine does not return */
    terminate();
  }
  if ((DAT_00535328 != (_func_int *)0x0) && (iVar2 = _ValidateExecute(DAT_00535328), iVar2 != 0)) {
    iVar2 = (*DAT_00535328)(param_1);
    return iVar2;
  }
  return 0;
}

