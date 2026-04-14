// Scene_SpawnBallsAndObjects @ 0x0041C5B0 (originally decompiled as 0x41C5B0)
// Level startup: spawn player balls and all interactive objects
//
// Flow:
//   1. For each start entry in ball_list:
//      - Lookup "START%d-%d" in hash table (level position data)
//      - For tournament modes: also lookup alternate start positions
//      - For 1-player specific race types (5/b/c/e): random start between "START2-1"/"START2-2"
//      - Check for "START-DEBUG" override
//      - Ball_ctor2(0xC60 bytes) at found position
//      - Ball_Init: vtable[4]() - sets up initial state
//      - Ball_SetTrajectory: set initial velocity params from level data
//      - Configure ball: player_index, radius=26.0, max_speed=5.0, gravity=0.5
//      - Speed scale = 0.1 (0x3DCCCCCD)
//      - Gravity scale = 1.00625 (0x3F866666)
//      - Append ball to scene's ball list
//      - Store ball pointer in start entry
//
//   2. Scan for SAFESPOT/SAFEPOS entries in meshworld string table
//      - Append safe spots to scene's safe spot list (+0x546)
//
//   3. If tournament or demo mode (scene+0x21E flags):
//      - CreateBadBall() - AI opponent ball
//      - CreateMouseTrap() - arena hazard
//
//   4. Create level objects:
//      - CreateSecretObjects() - hidden/bonus items
//      - Scene_CreateFlags() - race checkpoint flags
//      - Scene_CreateSigns() - directional sign objects
//      - Scene_CreateDynamicObjects() - moving/animated objects
//
// Key ball properties set:
//   pos = START position from hash table lookup
//   radius = 26.0
//   max_speed = 5.0 (=5000 units, see max_speed at +0x188)
//   gravity = 0.5
//   speed_scale = 0.1 (3DCCCCCDh)
//   gravity_multiplier = 1.00625 (3F866666h)
//   is_falling = 0
//   active_flag (+0x769) = 1

void Scene_SpawnBallsAndObjects(Scene *scene)
{
  // Set ball count in app state
  int num_starts = AthenaList_GetSize(scene->ball_list);
  app->num_balls = num_starts;
  
  // For each start entry
  for each (start_entry in scene->ball_list) {
    // Lookup "START<startnum>-<playeridx>" position
    Vec3 pos = HashTable_Lookup(scene->hashtable, format("START%d-%d", ...));
    
    // Tournament alternate starts
    if (app->is_tournament) {
      pos = HashTable_Lookup(scene->hashtable, format("START%d-%d", ...));
    }
    
    // 1-player random start for certain race types
    if (num_starts == 1 && race_type in {5,11,12,14}) {
      if (RNG_Rand(2) == 0)
        pos = Lookup("START2-1");
      else
        pos = Lookup("START2-2");
    }
    
    // Debug start override
    if (num_starts == 1 && Lookup("START-DEBUG") != default)
      pos = Lookup("START-DEBUG");
    
    // Create ball
    Ball *ball = Ball_ctor2(new(0xC60), scene);
    ball->vtable[4]();  // Init
    Ball_SetTrajectory(ball, ...);
    ball->pos = pos;
    ball->player_index = start_entry->player_index;
    ball->gravity = 0.5f;
    ball->speed_scale = 0.1f;
    ball->radius = 26.0f;
    ball->gravity_mult = 1.00625f;
    ball->max_speed = 5.0f;
    ball->is_falling = false;
    ball->active = true;
    
    AthenaList_Append(scene->balls, ball);
    start_entry->ball = ball;
  }
  
  // Safe spots
  for each (string in meshworld_stringtable) {
    if (strnicmp(string, "SAFESPOT", 8) || strnicmp(string, "SAFEPOS", 7))
      AthenaList_Append(scene->safe_spots, string);
  }
  
  // Tournament/demo objects
  if (app->is_tournament || app->is_demo) {
    CreateBadBall(scene);
    CreateMouseTrap(scene);
  }
  
  // Level objects
  CreateSecretObjects(scene);
  Scene_CreateFlags(scene);
  Scene_CreateSigns(scene);
  Scene_CreateDynamicObjects(scene);
}