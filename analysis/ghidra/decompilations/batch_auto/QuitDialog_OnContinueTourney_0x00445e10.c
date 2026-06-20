
void __fastcall QuitDialog_OnContinueTourney(int *param_1)

{
  int *this;
  
  (**(code **)(*param_1 + 0x40))();
  this = (int *)param_1[0x22e];
  Tourney_AdvanceRound(this);
  Tourney_SetCurrentLevel(this,*(int *)(*(int *)(param_1[0x21e] + 0x220) + 8) + -1);
  *(undefined4 *)(param_1[0x21e] + 0x5e4) = 0;
  return;
}

