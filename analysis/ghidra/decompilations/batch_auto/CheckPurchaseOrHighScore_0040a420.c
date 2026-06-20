/*
 * Function: CheckPurchaseOrHighScore
 * Address: 0x0040a420
 * Signature: void __fastcall CheckPurchaseOrHighScore(void *param_1)
 *
 * Patterns: allocates, vtable dispatch, SEH frame, scene. Calls: CheckPurchaseOrHighScore, __ftol2, operator_new, ConfirmMenu_ctor, Game_SetInProgress, ScoreDisplay_CtorB, HighScoreEntry_ctor, Scene_AddObject. Offsets: 7, Lines: 57
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __fastcall CheckPurchaseOrHighScore(void *param_1)

{
  char cVar1;
  int iVar2;
  void *pvVar3;
  int *piVar4;
  ulonglong uVar5;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004c9ae1;
  pvStack_c = ExceptionList;
  iVar2 = **(int **)(*(int *)((int)param_1 + 0x878) + 0x230);
  ExceptionList = &pvStack_c;
  uVar5 = __ftol2();
  iVar2 = (**(code **)(iVar2 + 0xc))((int)uVar5);
  if ((iVar2 == 0) || (*(char *)(*(int *)(*(int *)((int)param_1 + 0x878) + 0x220) + 0x94) != '\0'))
  {
    pvVar3 = operator_new(0xcfc);
    puStack_8 = (undefined1 *)0x2;
    if (pvVar3 != (void *)0x0) {
      piVar4 = ConfirmMenu_ctor(pvVar3,*(int *)((int)param_1 + 0x878),'\x01');
      goto LAB_0040a518;
    }
  }
  else {
    cVar1 = Game_SetInProgress(*(int *)((int)param_1 + 0x878));
    if (cVar1 == '\0') {
      pvVar3 = operator_new(0x115c);
      puStack_8 = (undefined1 *)0x1;
      if (pvVar3 != (void *)0x0) {
        piVar4 = ScoreDisplay_CtorB(pvVar3,*(int *)((int)param_1 + 0x878),
                                    (byte *)
                                    "BUY HAMSTERBALL AND YOU CAN SAVE YOUR HIGH SCORES!  YOU CAN USE THE QUICK AND EASY BUY LINK BELOW, AND YOUR CURRENT SCORE WILL BE SAVED!"
                                   );
        goto LAB_0040a518;
      }
    }
    else {
      pvVar3 = operator_new(0x904);
      puStack_8 = (undefined1 *)0x0;
      if (pvVar3 != (void *)0x0) {
        piVar4 = HighScoreEntry_ctor(pvVar3,*(int *)((int)param_1 + 0x878),
                                     *(int *)((int)param_1 + 0x878) + 0x5cc);
        goto LAB_0040a518;
      }
    }
  }
  piVar4 = (int *)0x0;
LAB_0040a518:
  puStack_8 = (undefined1 *)0xffffffff;
  Scene_AddObject(*(void **)(*(int *)((int)param_1 + 0x878) + 0x184),piVar4);
  ExceptionList = param_1;
  return;
}
