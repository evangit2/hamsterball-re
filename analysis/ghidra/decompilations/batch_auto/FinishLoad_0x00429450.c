
void __fastcall FinishLoad(void *param_1)

{
  int *piVar1;
  int iVar2;
  
  *(char **)((int)param_1 + 0x208) = "FinishLoad(1)";
  piVar1 = (int *)((int)param_1 + 0x2cc);
  iVar2 = 2;
  do {
    if (piVar1[-1] != 0) {
      *(undefined1 *)(piVar1[-1] + 0x1e) = 1;
    }
    if (*piVar1 != 0) {
      *(undefined1 *)(*piVar1 + 0x1e) = 1;
    }
    if (piVar1[1] != 0) {
      *(undefined1 *)(piVar1[1] + 0x1e) = 1;
    }
    if (piVar1[2] != 0) {
      *(undefined1 *)(piVar1[2] + 0x1e) = 1;
    }
    if (piVar1[3] != 0) {
      *(undefined1 *)(piVar1[3] + 0x1e) = 1;
    }
    if (piVar1[4] != 0) {
      *(undefined1 *)(piVar1[4] + 0x1e) = 1;
    }
    if (piVar1[5] != 0) {
      *(undefined1 *)(piVar1[5] + 0x1e) = 1;
    }
    if (piVar1[6] != 0) {
      *(undefined1 *)(piVar1[6] + 0x1e) = 1;
    }
    if (piVar1[7] != 0) {
      *(undefined1 *)(piVar1[7] + 0x1e) = 1;
    }
    if (piVar1[8] != 0) {
      *(undefined1 *)(piVar1[8] + 0x1e) = 1;
    }
    piVar1 = piVar1 + 10;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  *(char **)((int)param_1 + 0x208) = "FinishLoad(2)";
  App_ShowResults(param_1,'\x01');
  *(char **)((int)param_1 + 0x208) = "FinishLoad(3)";
  App_ShowMainMenu((int)param_1);
  *(char **)((int)param_1 + 0x208) = "FinishLoad(4)";
  App_WritePlayCount((int)param_1);
  *(char **)((int)param_1 + 0x208) = "FinishLoad(OK)";
  return;
}

