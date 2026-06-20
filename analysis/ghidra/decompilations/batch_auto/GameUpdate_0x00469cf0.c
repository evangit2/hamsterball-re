
void __fastcall GameUpdate(int param_1)

{
  void *this;
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  
  this = (void *)(param_1 + 4);
  iVar2 = AthenaList_NextIndex((int)this);
  *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 8) < 1) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = (int *)**(undefined4 **)(param_1 + 0x410);
    *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 1;
  }
  while (piVar4 != (int *)0x0) {
    if (((char)piVar4[0xb] == '\0') && ((char)piVar4[0x21d] == '\0')) {
      *(int *)(*(int *)(param_1 + 0x844) + 0x20c) = piVar4[0x21a];
      (**(code **)(*piVar4 + 4))();
    }
    iVar1 = *(int *)(param_1 + 0xc + iVar2 * 4);
    if (*(int *)(param_1 + 8) <= iVar1) break;
    piVar4 = *(int **)(*(int *)(param_1 + 0x410) + iVar1 * 4);
    *(int *)(param_1 + 0xc + iVar2 * 4) = iVar1 + 1;
  }
  iVar2 = AthenaList_NextIndex((int)this);
  *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 0;
  if (*(int *)(param_1 + 8) < 1) {
    piVar4 = (int *)0x0;
  }
  else {
    piVar4 = (int *)**(undefined4 **)(param_1 + 0x410);
    *(undefined4 *)(param_1 + 0xc + iVar2 * 4) = 1;
  }
  while( true ) {
    if (piVar4 == (int *)0x0) {
      return;
    }
    if ((char)piVar4[0xb] != '\0') {
      *(int *)(*(int *)(param_1 + 0x844) + 0x20c) = piVar4[0x21a];
      *(char **)(*(int *)(param_1 + 0x844) + 0x210) = "Remove Object";
      if (*(int **)(param_1 + 0x41c) == piVar4) {
        (**(code **)(**(int **)(param_1 + 0x844) + 0x68))(0xffffd8f1,0xffffd8f1);
        (**(code **)(*piVar4 + 0x30))();
        *(undefined4 *)(param_1 + 0x41c) = 0;
      }
      if (*(int **)(param_1 + 0x420) == piVar4) {
        *(undefined4 *)(param_1 + 0x420) = 0;
      }
      if ((int *)(*(int **)(param_1 + 0x844))[0x62] == piVar4) {
        (**(code **)(**(int **)(param_1 + 0x844) + 0x60))();
      }
      (**(code **)(*(int *)(param_1 + 0x428) + 0xc))(piVar4);
      if (*(int **)(param_1 + 0x41c) == piVar4) {
        *(undefined4 *)(param_1 + 0x41c) = 0;
      }
      if (*(int **)(param_1 + 0x424) == piVar4) {
        uVar3 = (**(code **)(*(int *)(param_1 + 0x428) + 8))();
        *(undefined4 *)(param_1 + 0x424) = uVar3;
      }
      thunk_Gfx_SetRenderState(this,(int)piVar4);
      piVar4[0xc] = 0;
      (**(code **)(**(int **)(param_1 + 0x844) + 0x74))();
      (**(code **)(*piVar4 + 0x3c))(param_1);
      (**(code **)*piVar4)(1);
      *(char **)(*(int *)(param_1 + 0x844) + 0x210) = "Update";
    }
    iVar1 = *(int *)(param_1 + 0xc + iVar2 * 4);
    if (*(int *)(param_1 + 8) <= iVar1) break;
    piVar4 = *(int **)(*(int *)(param_1 + 0x410) + iVar1 * 4);
    *(int *)(param_1 + 0xc + iVar2 * 4) = iVar1 + 1;
  }
  return;
}

