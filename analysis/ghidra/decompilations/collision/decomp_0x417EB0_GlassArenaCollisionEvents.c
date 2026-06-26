// GlassArenaCollisionEvents @ 0x417EB0
// Verified: vtable[0x1D] handler, created+decompiled June 2026


void __thiscall GlassArenaCollisionEvents(void *param_1,int *param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  bool bVar5;
  
  iVar1 = __strnicmp(*(char **)(param_3[1] + 0x864),"N:GLASS",7);
  iVar2 = 0xf;
  if (iVar1 == 0) {
    param_2[0x317] = 0xf;
  }
  pcVar4 = "DN:SINKPLATFORM";
  bVar5 = true;
  pcVar3 = *(char **)(param_3[1] + 0x864);
  do {
    pcVar4 = pcVar4 + 1;
    if (iVar2 == 0) break;
    iVar2 = iVar2 + -1;
    bVar5 = *pcVar3 == *pcVar4;
    pcVar3 = pcVar3 + 1;
  } while (bVar5);
  if (bVar5) {
    Scene_StartCountdown(*(void **)(*param_3 + 0x47c),(int)param_2);
  }
  CreateNoDizzy(param_1,param_2,param_3);
  return;
}

