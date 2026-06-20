
void __fastcall App_SaveAllConfig(int param_1)

{
  App_WriteDisplaySettings(param_1);
  RegKey_Open(*(int *)(param_1 + 0x54));
  RegKey_WriteDWORD(*(void **)(param_1 + 0x54),"MouseSensitivity",*(undefined4 *)(param_1 + 0x84c));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"MirrorTournament",*(BYTE *)(param_1 + 0x850));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"DizzyRace",*(BYTE *)(param_1 + 0x851));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"TowerRace",*(BYTE *)(param_1 + 0x852));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"UpRace",*(BYTE *)(param_1 + 0x853));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ExpertRace",*(BYTE *)(param_1 + 0x854));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"OddRace",*(BYTE *)(param_1 + 0x855));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ToobRace",*(BYTE *)(param_1 + 0x856));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"WobblyRace",*(BYTE *)(param_1 + 0x857));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"SkyRace",*(BYTE *)(param_1 + 0x858));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"MasterRace",*(BYTE *)(param_1 + 0x859));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"RightButtonPause",*(BYTE *)(param_1 + 0x238));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"DizzyArena",*(BYTE *)(param_1 + 0x85a));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"TowerArena",*(BYTE *)(param_1 + 0x85b));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"UpArena",*(BYTE *)(param_1 + 0x85c));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ExpertArena",*(BYTE *)(param_1 + 0x85d));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"NeonRace",*(BYTE *)(param_1 + 0x863));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"GlassRace",*(BYTE *)(param_1 + 0x864));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ImpossibleRace",*(BYTE *)(param_1 + 0x865));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"NeonArena",*(BYTE *)(param_1 + 0x866));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"GlassArena",*(BYTE *)(param_1 + 0x867));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ImpossibleArena",*(BYTE *)(param_1 + 0x868));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"OddArena",*(BYTE *)(param_1 + 0x85e));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"ToobArena",*(BYTE *)(param_1 + 0x85f));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"WobblyArena",*(BYTE *)(param_1 + 0x860));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"SkyArena",*(BYTE *)(param_1 + 0x861));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"MasterArena",*(BYTE *)(param_1 + 0x862));
  RegKey_WriteBool(*(void **)(param_1 + 0x54),"RightButtonPause",*(BYTE *)(param_1 + 0x238));
  Registry_SetValue(*(void **)(param_1 + 0x54),"BestTime",(BYTE *)(param_1 + 0x86c),0x50);
  Registry_SetValue(*(void **)(param_1 + 0x54),"Medals",(BYTE *)(param_1 + 0x8bc),0x50);
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"2PController1",*(undefined4 *)(param_1 + 0xb28));
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"2PController2",*(undefined4 *)(param_1 + 0xb2c));
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"2PController3",*(undefined4 *)(param_1 + 0xb30));
  RegKey_WriteDword(*(void **)(param_1 + 0x54),"2PController4",*(undefined4 *)(param_1 + 0xb34));
  RegKey_Close(*(int *)(param_1 + 0x54));
  return;
}

