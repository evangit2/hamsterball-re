
void __fastcall LoaderGadget_Tick(int *param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  if ((char)param_1[0xb59] != '\0') {
    iVar2 = param_1[0x21e];
    param_1[0xb58] = param_1[0xb58] + 1;
    *(undefined1 *)(param_1 + 0xb59) = 0;
    piVar1 = param_1 + 0x21f;
    if (((-1 < iVar2) && (iVar2 < param_1[0x220])) &&
       (iVar2 = *(int *)(param_1[0x322] + iVar2 * 4), iVar2 != 0)) {
      (**(code **)(*param_1 + 0x6c))(iVar2);
    }
    param_1[0x21e] = param_1[0x21e] + 1;
    iVar2 = AthenaList_GetSize((int)piVar1);
    if (iVar2 != 0) {
      iVar2 = param_1[0x21e];
      iVar3 = AthenaList_GetSize((int)piVar1);
      param_1[0xb57] = (int)((float)iVar2 / (float)iVar3);
    }
    iVar2 = AthenaList_GetSize((int)piVar1);
    if (param_1[0x21e] == iVar2) {
                    /* WARNING: Could not recover jumptable at 0x00474ad6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*param_1 + 0x68))();
      return;
    }
  }
  return;
}

