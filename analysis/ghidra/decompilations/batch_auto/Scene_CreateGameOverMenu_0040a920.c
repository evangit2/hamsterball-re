/*
 * Function: Scene_CreateGameOverMenu
 * Address: 0x0040a920
 * Signature: void __thiscall Scene_CreateGameOverMenu(void *this,char param_1)
 *
 * Patterns: allocates, SEH frame, scene. Calls: Scene_CreateGameOverMenu, operator_new, PauseMenu_Ctor, QuitRaceMenu, QuitRace, PauseArenaMenu_ctor, Scene_AddObject. Offsets: 7, Lines: 58
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Scene_CreateGameOverMenu(void *this,char param_1)

{
  int iVar1;
  void *pvVar2;
  int *piVar3;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004c953c;
  local_c = ExceptionList;
  if (param_1 == '\0') goto LAB_0040aa30;
  if (*(char *)(*(int *)((int)this + 0x878) + 0x237) == '\0') {
    iVar1 = *(int *)(*(int *)((int)this + 0x878) + 0x220);
    if (*(char *)(iVar1 + 0x11) == '\0') {
      if (*(char *)(iVar1 + 0x10) == '\0') {
        ExceptionList = &local_c;
        pvVar2 = operator_new(0xcdc);
        local_4 = 3;
        if (pvVar2 == (void *)0x0) goto LAB_0040aa14;
        piVar3 = PauseMenu_Ctor(pvVar2,*(int *)((int)this + 0x878));
      }
      else {
        ExceptionList = &local_c;
        pvVar2 = operator_new(0xcdc);
        local_4 = 2;
        if (pvVar2 == (void *)0x0) {
LAB_0040aa14:
          piVar3 = (int *)0x0;
        }
        else {
          piVar3 = QuitRaceMenu(pvVar2,*(int *)((int)this + 0x878));
        }
      }
    }
    else {
      ExceptionList = &local_c;
      pvVar2 = operator_new(0xcdc);
      local_4 = 1;
      if (pvVar2 == (void *)0x0) goto LAB_0040aa14;
      piVar3 = QuitRace(pvVar2,*(int *)((int)this + 0x878));
    }
  }
  else {
    ExceptionList = &local_c;
    pvVar2 = operator_new(0xcdc);
    local_4 = 0;
    if (pvVar2 == (void *)0x0) goto LAB_0040aa14;
    piVar3 = PauseArenaMenu_ctor(pvVar2,*(int *)((int)this + 0x878));
  }
  local_4 = 0xffffffff;
  Scene_AddObject(*(void **)(*(int *)((int)this + 0x878) + 0x184),piVar3);
LAB_0040aa30:
  *(undefined1 *)((int)this + 0x874) = 1;
  ExceptionList = local_c;
  return;
}
