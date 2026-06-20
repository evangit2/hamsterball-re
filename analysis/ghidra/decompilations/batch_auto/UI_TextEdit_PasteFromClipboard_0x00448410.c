
void __fastcall UI_TextEdit_PasteFromClipboard(int param_1)

{
  char cVar1;
  HANDLE hMem;
  char *pcVar2;
  undefined4 ****ppppuVar3;
  char *pcVar4;
  undefined4 ****ppppuVar5;
  uint uVar6;
  void *this;
  uint uVar7;
  undefined1 local_42c [4];
  undefined4 ***local_428 [4];
  uint local_418;
  uint local_414;
  void *local_410;
  char local_40c [1024];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cc38b;
  local_c = ExceptionList;
  this = (void *)(param_1 + 0x1120);
  if (*(int *)(param_1 + 0x1108) != param_1 + 0x1098) {
    this = local_410;
  }
  if (*(int *)(param_1 + 0x1108) == param_1 + 0x10b4) {
    this = (void *)(param_1 + 0x113c);
  }
  ExceptionList = &local_c;
  OpenClipboard((HWND)0x0);
  hMem = GetClipboardData(1);
  if ((hMem != (HANDLE)0x0) && (pcVar2 = GlobalLock(hMem), pcVar2 != (char *)0x0)) {
    pcVar4 = pcVar2;
    do {
      cVar1 = *pcVar4;
      pcVar4[(int)(local_40c + -(int)pcVar2)] = cVar1;
      pcVar4 = pcVar4 + 1;
    } while (cVar1 != '\0');
    GlobalUnlock(pcVar2);
    local_414 = 0xf;
    local_418 = 0;
    local_428[0] = (undefined4 ***)((uint)local_428[0] & 0xffffff00);
    pcVar2 = local_40c;
    local_4 = 0;
    do {
      cVar1 = *pcVar2;
      pcVar2 = pcVar2 + 1;
    } while (cVar1 != '\0');
    StdString_Assign(local_42c,(undefined4 *)local_40c,(int)pcVar2 - (int)(local_40c + 1));
    uVar7 = 0;
    ppppuVar5 = (undefined4 ****)local_428[0];
    uVar6 = local_414;
    if (local_418 != 0) {
      do {
        ppppuVar3 = ppppuVar5;
        if (uVar6 < 0x10) {
          ppppuVar3 = local_428;
        }
        if (*(char *)((int)ppppuVar3 + uVar7) < ' ') {
LAB_00448519:
          StdString_Erase(local_42c,uVar7,1);
          uVar7 = uVar7 - 1;
          ppppuVar5 = (undefined4 ****)local_428[0];
          uVar6 = local_414;
        }
        else {
          ppppuVar3 = ppppuVar5;
          if (uVar6 < 0x10) {
            ppppuVar3 = local_428;
          }
          if ('\x7f' < *(char *)((int)ppppuVar3 + uVar7)) goto LAB_00448519;
        }
        uVar7 = uVar7 + 1;
      } while (uVar7 < local_418);
    }
    StdString_Substr(this,local_42c,0,0xffffffff);
    StdString_TruncateToWidth(this);
    *(undefined4 *)(param_1 + 0x1158) = *(undefined4 *)((int)this + 0x14);
    if (0xf < local_414) {
      _free(local_428[0]);
    }
  }
  CloseClipboard();
  ExceptionList = local_c;
  return;
}

