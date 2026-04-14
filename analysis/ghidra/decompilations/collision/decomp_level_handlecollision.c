// Level_HandleCollision @ 0x0040DCD0
// Level-specific collision event dispatcher
// Strings at collider->type_name (collider+0x864) determine behavior:
//
// "E:CATAPULTBOTTOM" → Catapult_Launch if ball cooldown < 1
//   - Set ball cooldown to 1000
//   - Find matching catapult by object ID in scene catapult list (+0x43B8)
//   - Store ball pointer in catapult+0x10EC, then launch
//   - Play catapult sound
//
// "E:OPENSESAME" → Trapdoor_Open on first door
//   - Simple: find first door in door list (+0x4BEC) and open it
//
// "N:TRAPDOOR" → Trapdoor_Activate on matching doors
//   - Search door list (+0x47D0) for doors where trigger_id matches collider
//   - Activate all matching doors
//
// "E:BITE" → Set damage=25.0
//   - damage_timer = 0 (+0x43A8)
//   - damage_amount = 25.0 (0x41C80000) (+0x43A0)
//
// "E:MACETRIGGER" → Activate all maces
//   - Set mace active flag (+0x10F0) = 1 for all maces in list (+0x5000)
//
// "N:MACE" → Ball bounce callback if mace moving
//   - If mace matches collider AND speed != 80.0 (0x42A00000)
//   - AND not already bouncing (+0x10F4 == 0)
//   - Call ball->vtable[0x20] (BounceForce)
//
// Always ends with: CreateNoDizzy(this, ball, collider)

void __thiscall Level_HandleCollision(Scene *scene, Ball *ball, Collider *collider)
{
  const char *type = collider->sceneObject->type_name;  // +0x864
  
  if (stricmp(type, "E:CATAPULTBOTTOM") == 0 && ball->cooldown < 1) {
    ball->cooldown = 1000;
    // Find catapult matching this collider's ID
    for each (catapult in scene->catapult_list) {
      if (catapult->trigger_id == collider->id) {
        catapult->ball = ball;  // +0x10EC
        Catapult_Launch(catapult);
        Sound_PlayChannel(scene->app->catapult_sound);
      }
    }
  }
  
  if (stricmp(type, "E:OPENSESAME") == 0) {
    Trapdoor_Open(scene->door_list[0]);
  }
  
  if (stricmp(type, "N:TRAPDOOR") == 0) {
    for each (door in scene->door_list) {
      if (door->trigger_id == collider->id || door->alt_trigger_id == collider->id)
        Trapdoor_Activate(door);
    }
  }
  
  if (stricmp(type, "E:BITE") == 0) {
    scene->damage_timer = 0;        // +0x43A8
    scene->damage_amount = 25.0f;   // +0x43A0 (0x41C80000)
  }
  
  if (stricmp(type, "E:MACETRIGGER") == 0) {
    for each (mace in scene->mace_list)
      mace->active = 1;  // +0x10F0
  }
  
  if (stricmp(type, "N:MACE") == 0) {
    for each (mace in scene->mace_list) {
      if (mace->trigger_id == collider->id && mace->speed != 80.0f && !mace->already_hit)
        ball->vtable[0x20]();  // ApplyBounceForce
    }
  }
  
  CreateNoDizzy(scene, ball, collider);
}