/*
 * Function: SinkPlatformArenaCollisionEvents
 * Address: 0x00413bd0
 * Signature: void __thiscall SinkPlatformArenaCollisionEvents(void *this,int *param_1,int *param_2)
 *
 * Patterns: collision, scene. Calls: SinkPlatformArenaCollisionEvents, Scene_StartCountdown, DispatchCollisionEvents. Offsets: 2, Lines: 24
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall SinkPlatformArenaCollisionEvents(void *this,int *param_1,int *param_2)

{
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  bool bVar4;
  
  pcVar3 = "DN:SINKPLATFORM";
  iVar1 = 0xf;
  bVar4 = true;
  pcVar2 = *(char **)(param_2[1] + 0x864);
  do {
    pcVar3 = pcVar3 + 1;
    if (iVar1 == 0) break;
    iVar1 = iVar1 + -1;
    bVar4 = *pcVar2 == *pcVar3;
    pcVar2 = pcVar2 + 1;
  } while (bVar4);
  if (bVar4) {
    Scene_StartCountdown(*(void **)(*param_2 + 0x47c),(int)param_1);
  }
  DispatchCollisionEvents(this,param_1,param_2);
  return;
}
