// TournamentManager (0x433ac0)
// Main menu command dispatcher for the title screen
// param_1 = command string from button click

void __thiscall TournamentManager(void *this, char *cmd)
{
  App *app = *(App **)(this + 0x878);
  app->flags0x234 = 0;
  if (cmd == NULL) return;
  
  if (strcmp(cmd, "BACK")) {
    self->close();
    MainMenu_ctor(new(0xcdc), app);
    app->main_menu = result;
    Scene_AddObject(app->scene, app->main_menu);
  }
  
  if (strcmp(cmd, "LOCKED")) {
    // Mirror Tournament locked message
    OkayDialog_ctor(new(0x8a8), app,
      "NOT UNLOCKED!",
      "THE MIRROR TOURNAMENT ISN'T UNLOCKED YET! "
      "TO UNLOCK THE MIRROR TOURNAMENT, YOU NEED TO WIN A TOURNAMENT "
      "AT NORMAL OR FRENZIED DIFFICULTY!",
      600);
  }
  
  if (strcmp(cmd, "1PT")) {    // 1-Player Tournament
    app->is_tournament = 1;
    app->is_mirror = 0;
    if (save_not_found || !Game_SetInProgress(app)) {
      self->close();
      DifficultyMenu_ctor(new(0xcdc), app);  // New tournament
    } else {
      TourneyContinueDialog_Ctor(new(0x8bc), app, this);  // Resume?
    }
  }
  
  if (strcmp(cmd, "1PMT")) {   // 1-Player Mirror Tournament
    app->is_tournament = 1;
    app->is_mirror = 1;       // Mirror mode!
    // Same flow as 1PT but with mirror flag set
    if (save_not_found) {
      self->close();
      DifficultyMenu_ctor(new(0xcdc), app);
    } else {
      TourneyContinueDialog_Ctor(new(0x8bc), app, this);
    }
  }
  
  if (strcmp(cmd, "1PP")) {    // 1-Player Practice
    app->is_tournament = 0;
    self->close();
    PracticeMenu_ctor(new(0xd18), app);
  }
  
  if (strcmp(cmd, "1PTT")) {   // 1-Player Time Trial
    app->is_tournament = 0;
    self->close();
    TimeTrialMenu_ctor(new(0xd18), app);
  }
  
  if (strcmp(cmd, "PARTY")) {   // Multiplayer Party
    self->close();
    MPMenu_ctor(new(0xcdc), app);
  }
}
