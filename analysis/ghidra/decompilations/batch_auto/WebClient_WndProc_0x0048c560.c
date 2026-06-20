
LRESULT WebClient_WndProc(HWND param_1,UINT param_2,WPARAM param_3,uint param_4)

{
  undefined2 uVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  LRESULT LVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uStack_20;
  char *pcVar9;
  
  piVar2 = (int *)GetWindowLongA(param_1,-0x15);
  if (param_2 == 0x113) {
    if (param_3 != 1) {
      if (param_3 == 2) {
        uStack_20 = 0x48c710;
        iVar3 = Ordinal_16();
        if (iVar3 == -1) {
          uStack_20 = 0x48c71b;
          iVar3 = Ordinal_111();
          if (iVar3 == -1) {
            SoundDevice_ReleaseWindow((int)piVar2);
            piVar2[1] = 3;
            AthenaString_AssignCStr(piVar2 + 2,"Could not receive server response");
            (**(code **)(*piVar2 + 4))();
            return 1;
          }
          if (iVar3 == 0x2733) {
            uStack_20 = 0x48c737;
            SetTimer(param_1,2,0xfa,(TIMERPROC)0x0);
          }
        }
        else {
          uStack_20 = 1;
          SoundDevice_ReleaseWindow((int)piVar2);
          piVar2[1] = 1;
          uStack_20 = 0x48c77f;
          (**(code **)(*piVar2 + 4))();
        }
      }
      goto LAB_0048c7a5;
    }
    SoundDevice_ReleaseWindow((int)piVar2);
    piVar2[1] = 4;
    pcVar9 = "Time-out";
  }
  else {
    if (param_2 != 0x7e8) goto LAB_0048c7a5;
    piVar2[0x117] = 0;
    if (param_4 >> 0x10 != 0) {
      piVar2[1] = 3;
      AthenaString_AssignCStr(piVar2 + 2,"Host was not found");
      (**(code **)(*piVar2 + 4))();
      goto LAB_0048c7a5;
    }
    Ordinal_12();
    iVar3 = Ordinal_11();
    *(undefined2 *)(piVar2 + 0x119) = 2;
    uStack_20 = 0x48c5ec;
    uVar1 = Ordinal_9();
    *(undefined2 *)((int)piVar2 + 0x466) = uVar1;
    piVar2[0x11a] = iVar3;
    piVar2[0x11b] = 0;
    uStack_20 = 0;
    piVar2[0x11c] = 0;
    iVar3 = Ordinal_23(2,1);
    piVar2[0x118] = iVar3;
    iVar3 = Ordinal_4(iVar3,piVar2 + 0x119,0x10);
    if (iVar3 < 0) {
      piVar2[1] = 3;
      AthenaString_AssignCStr(piVar2 + 2,"Could not connect to server");
      (**(code **)(*piVar2 + 4))();
      goto LAB_0048c7a5;
    }
    uStack_20 = 1;
    Ordinal_10(piVar2[0x118],0x8004667e,&uStack_20);
    iVar3 = piVar2[10];
    uVar8 = 0;
    iVar4 = StdString_RecalcLen((int)(piVar2 + 9));
    iVar3 = Ordinal_19(piVar2[0x118],iVar3,iVar4,uVar8);
    if (-1 < iVar3) {
      StdString_FreeBuffer((int)(piVar2 + 2));
      StdString_Reserve(piVar2 + 2,piVar2[0x11f] + 1);
      iVar3 = piVar2[0x11f];
      puVar7 = (undefined4 *)piVar2[3];
      for (uVar6 = iVar3 + 1U >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
        *puVar7 = 0;
        puVar7 = puVar7 + 1;
      }
      for (uVar6 = iVar3 + 1U & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *(undefined1 *)puVar7 = 0;
        puVar7 = (undefined4 *)((int)puVar7 + 1);
      }
      uStack_20 = 0x48c6df;
      SetTimer(param_1,2,1,(TIMERPROC)0x0);
      goto LAB_0048c7a5;
    }
    SoundDevice_ReleaseWindow((int)piVar2);
    piVar2[1] = 3;
    pcVar9 = "Could not send URL request";
  }
  AthenaString_AssignCStr(piVar2 + 2,pcVar9);
  (**(code **)(*piVar2 + 4))();
LAB_0048c7a5:
  uStack_20 = 0x48c7bb;
  LVar5 = DefWindowProcA(param_1,param_2,param_3,param_4);
  return LVar5;
}

