// Arena_HandleCollision @ 0x0040E6A0
// Arena (RumbleBoard) collision event dispatcher
// Handles all multiplayer arena specific collision events:
//
// "E:CALLHAMMER" → CreateBonkPopup (if tournament mode, app+0x23C != 0)
//   - Creates a hammer/bonk visual effect
//
// "E:HAMMERCHASE" → Hammer_ChaseStart (if tournament mode)
//   - AI hammer starts chasing player
//
// "E:ALERTSAW1" / "E:ALERTSAW2" → Saw_AlertActivate
//   - Pre-activate saw blade 1 or 2 (visual warning)
//
// "E:ACTIVATESAW1" / "E:ACTIVATESAW2" → Saw_Activate
//   - Full activate saw blade 1 or 2
//
// "E:ALERTJUDGES" → Judge_Reset for all judges
//   - Reset all judge objects in judge list (+0x4BBC)
//
// "E:SCORE<time>" → ScoreDisplay_SetTime with parsed time
//   - strnicmp "E:SCORE" + atol(suffix) for time value
//   - Apply to all score displays in list (+0x4BBC)
//
// "E:JUMP" → Ball jump pad (if jump_cooldown < 1)
//   - Play 3D jump sound at ball position
//   - Set ball jump_cooldown = 10 (+0x1F7)
//   - Set ball vertical velocity = 0.008 (0x3B03126F) (+0xA7)
//   - Enable vertical velocity flag (+0xA8 = 1)
//   - Set general cooldown = 10 (+0x202)
//   - Ball_RecordBest(ball, 200) - record proximity best
//
// "E:BELL<suffix>" → Bell_Activate
//   - Activate bell object (+0x4FD4)
//   - If no race (not playing, not demo): add 500 bonus time
//   - Create ScoreObject with "EXTRA TIME:" text
//   - Timer_Decrement + append to scene score list (+0x8B8)
//
// Always ends with: CreateNoDizzy(scene, ball, collider)

void __thiscall Arena_HandleCollision(Scene *scene, Ball *ball, Collider *collider)
{
  const char *type = collider->sceneObject->type_name;
  
  if (stricmp(type, "E:CALLHAMMER") == 0 && scene->app->is_tournament)
    CreateBonkPopup(scene->hammer_obj);  // +0x436C
  
  if (stricmp(type, "E:HAMMERCHASE") == 0 && scene->app->is_tournament)
    Hammer_ChaseStart(scene->hammer_obj);
  
  if (stricmp(type, "E:ALERTSAW1") == 0 && scene->app->is_tournament)
    Saw_AlertActivate(scene->saw1_obj);  // +0x4370
  if (stricmp(type, "E:ALERTSAW2") == 0 && scene->app->is_tournament)
    Saw_AlertActivate(scene->saw2_obj);  // +0x4374
  
  if (stricmp(type, "E:ACTIVATESAW1") == 0 && scene->app->is_tournament)
    Saw_Activate(scene->saw1_obj);
  if (stricmp(type, "E:ACTIVATESAW2") == 0 && scene->app->is_tournament)
    Saw_Activate(scene->saw2_obj);
  
  if (stricmp(type, "E:ALERTJUDGES") == 0) {
    for each (judge in scene->judge_list)
      Judge_Reset(judge);
  }
  
  if (strnicmp(type, "E:SCORE", 7) == 0) {
    long time = atol(type + 7);
    for each (display in scene->score_display_list)
      ScoreDisplay_SetTime(display, time);
  }
  
  if (stricmp(type, "E:JUMP") == 0 && ball->jump_cooldown < 1) {
    Sound_Play3D(app->jump_sound, ball->pos.x, ball->pos.y, ball->pos.z);
    ball->jump_cooldown = 10;       // +0x1F7
    ball->vert_velocity = 0.008f;   // +0xA7 (0x3B03126F)
    ball->vert_velocity_on = 1;     // +0xA8
    ball->cooldown = 10;            // +0x202
    Ball_RecordBest(ball, 200);
  }
  
  if (strnicmp(type, "E:BELL", 6) == 0) {
    Bell_Activate(scene->bell_obj);  // +0x4FD4
    if (!scene->app->is_playing && !scene->app->is_demo) {
      scene->app->bonus_time[player] += 500;
      ScoreObject *score = ScoreObject_ctor(new(0x30), scene,
        &scene->app->timer[player], "EXTRA TIME:");
      Timer_Decrement(score);
      AthenaList_Append(scene->score_list, score);  // +0x8B8
    }
  }
  
  CreateNoDizzy(scene, ball, collider);
}