
void __fastcall PauseGame(int param_1)

{
  char cVar1;
  bool bVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  float10 fVar6;
  
  App_ReadDisplaySettings(param_1);
  RegKey_Open(*(int *)(param_1 + 0x54));
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"MouseSensitivity");
  if (cVar1 == '\0') {
    *(undefined4 *)(param_1 + 0x84c) = 0x3f000000;
  }
  else {
    fVar6 = Registry_ReadFloat(*(void **)(param_1 + 0x54),"MouseSensitivity");
    *(float *)(param_1 + 0x84c) = (float)fVar6;
  }
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"MirrorTournament");
  *(bool *)(param_1 + 0x850) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"DizzyRace");
  *(bool *)(param_1 + 0x851) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"TowerRace");
  *(bool *)(param_1 + 0x852) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"UpRace");
  *(bool *)(param_1 + 0x853) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ExpertRace");
  *(bool *)(param_1 + 0x854) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"OddRace");
  *(bool *)(param_1 + 0x855) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ToobRace");
  *(bool *)(param_1 + 0x856) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"WobblyRace");
  *(bool *)(param_1 + 0x857) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"SkyRace");
  *(bool *)(param_1 + 0x858) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"MasterRace");
  *(bool *)(param_1 + 0x859) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"DizzyArena");
  *(bool *)(param_1 + 0x85a) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"TowerArena");
  *(bool *)(param_1 + 0x85b) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"UpArena");
  *(bool *)(param_1 + 0x85c) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ExpertArena");
  *(bool *)(param_1 + 0x85d) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"OddArena");
  *(bool *)(param_1 + 0x85e) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ToobArena");
  *(bool *)(param_1 + 0x85f) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"WobblyArena");
  *(bool *)(param_1 + 0x860) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"SkyArena");
  *(bool *)(param_1 + 0x861) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"MasterArena");
  *(bool *)(param_1 + 0x862) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"NeonRace");
  *(bool *)(param_1 + 0x863) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"GlassRace");
  *(bool *)(param_1 + 0x864) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ImpossibleRace");
  *(bool *)(param_1 + 0x865) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"NeonArena");
  *(bool *)(param_1 + 0x866) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"GlassArena");
  *(bool *)(param_1 + 0x867) = bVar2;
  bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"ImpossibleArena");
  *(bool *)(param_1 + 0x868) = bVar2;
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"RightButtonPause");
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x238) = 1;
  }
  else {
    bVar2 = RegKey_ReadBool(*(void **)(param_1 + 0x54),"RightButtonPause");
    *(bool *)(param_1 + 0x238) = bVar2;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"BestTime");
  if (cVar1 == '\0') {
    puVar5 = (undefined4 *)(param_1 + 0x86c);
    for (iVar4 = 0x14; iVar4 != 0; iVar4 = iVar4 + -1) {
      *puVar5 = 0x5f5e09c;
      puVar5 = puVar5 + 1;
    }
  }
  else {
    RegKey_QueryValue(*(void **)(param_1 + 0x54),"BestTime",(LPBYTE)(param_1 + 0x86c),0x50);
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"Medals");
  if (cVar1 == '\0') {
    puVar5 = (undefined4 *)(param_1 + 0x8bc);
    for (iVar4 = 0x14; iVar4 != 0; iVar4 = iVar4 + -1) {
      *puVar5 = 0;
      puVar5 = puVar5 + 1;
    }
  }
  else {
    RegKey_QueryValue(*(void **)(param_1 + 0x54),"Medals",(LPBYTE)(param_1 + 0x8bc),0x50);
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"2PController1");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"2PController1");
    *(uint *)(param_1 + 0xb28) = uVar3;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"2PController2");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"2PController2");
    *(uint *)(param_1 + 0xb2c) = uVar3;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"2PController3");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"2PController3");
    *(uint *)(param_1 + 0xb30) = uVar3;
  }
  cVar1 = RegKey_ReadString(*(void **)(param_1 + 0x54),"2PController4");
  if (cVar1 != '\0') {
    uVar3 = RegKey_ReadDword(*(void **)(param_1 + 0x54),"2PController4");
    *(uint *)(param_1 + 0xb34) = uVar3;
  }
  RegKey_Close(*(int *)(param_1 + 0x54));
  return;
}

