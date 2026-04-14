// RumbleBoard_Render (0x421910)
// The RumbleBoard HUD and timer rendering system
// Draws countdown timer, 4-player split HUD, tie-breaker indicator

void __thiscall RumbleBoard_Render(void *this, undefined4 param_1)
{
  // Create background scene object for timer display
  int screen_center = screen_width / 2;
  Scene_CreateObject4f(app->scene, timer_bg, screen_center - 88, 10.0, 180.0, 105.0);
  
  // Main countdown timer rendering (only if hasn't started yet: this+0x47c5 == 0)
  if (!timer_started) {
    // Timer color animation during last seconds
    if (time_remaining < 1100 && timer_expired) {
      // Red flashing (y=0, z=0)
      Matrix_Scale4x4(1.0, 0, 0, 1.0);
    }
    if (time_remaining < 600) {
      // Blue flashing
      Matrix_Scale4x4(1.0, 0, 0, 1.0);
    }
    
    // Draw centered time text (MM:SS format)
    char *time_str = AthenaString_Format(time_format);
    UI_DrawTextCentered(app->timer_font, time_str, y+38, x=0, scale=5,5);
    
    // Draw decimal sub-seconds ".N"
    char *frac_str = AthenaString_Format(".%.1d");
    UI_DrawTextShadow_Wrapper(app->shadow_font, frac_str, y, x=57, scale=5,5);
  }
  
  // Draw 4-player HUD quadrants
  // Top-left (0x4b, 0x4b)
  SceneObject_InitAtPosition(app->scene, 75, 75);
  UI_DrawTextCenteredAbsolute(app->timer_font, score_str, 75, 16, 5, 5);
  
  // Top-right (0x2d5, 0x4b) - only if P2 slot visible (app+0x677 == 0)
  if (!app->p2_hidden) {
    SceneObject_InitAtPosition(app->scene, 725, 75);
    UI_DrawTextCenteredAbsolute(app->timer_font, score_str, 725, 16, 5, 5);
  }
  
  // Bottom-left (0x4b, 0x20d) - only if P3 slot visible (app+0x717 == 0)
  if (!app->p3_hidden) {
    SceneObject_InitAtPosition(app->scene, 75, 525);
    UI_DrawTextCenteredAbsolute(app->timer_font, score_str, 75, 466, 5, 5);
  }
  
  // Bottom-right (0x2d5, 0x20d) - only if P4 slot visible (app+0x7b7 == 0)
  if (!app->p4_hidden) {
    SceneObject_InitAtPosition(app->scene, 725, 525);
    UI_DrawTextCenteredAbsolute(app->timer_font, score_str, 725, 466, 5, 5);
  }
  
  // Render arena objects via callback list (this+0x8b8 count, this+0xCC4 array)
  int idx = AthenaList_NextIndex(this + 0x8b8);
  this->render_flags[idx] = 0;
  for each object in object_list:
    this->render_flags[idx] = playing;
    object->vtable[2](param_1);  // render callback
    idx++;
  
  // Tie-breaker overlay
  if (timer_started && tie_breaker_active) {
    UI_DrawTextCenteredAbsolute(app->font, "TIE BREAKER!", 400, 40, 3, 3);
  }
}
