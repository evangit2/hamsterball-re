
void __fastcall Tourney_SaveTournament(int *param_1)

{
  (**(code **)(*param_1 + 0x40))();
  CRT_remove("DATA\\tournament.sav");
  (**(code **)(*(int *)param_1[0x22e] + 0x54))(&DAT_004d48a0);
  return;
}

