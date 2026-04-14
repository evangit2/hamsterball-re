// Level_RenderObjects @ 0x0040B570
// Transparent pass renderer - called after opaque pass
// 
// Flow:
//   1. Graphics_BeginFrame(gfx, 0) - setup view/projection
//   2. level->vtable[0x4C]() - render level mesh (terrain geometry)
//   3. Graphics_BeginFrame(gfx, 0) - reset for objects
//   4. For each object in visible_list: vtable[0x0C]() (RenderTransparent)
//
// The visible_list was built in Level_UpdateAndRender and contains
// all objects from both primary and secondary ball lists, sorted
// by render priority.

void __thiscall Level_RenderObjects(void *this, void *gfx)
{
  Graphics_BeginFrame(gfx, 0);
  this->vtable[0x4C]();  // Render level mesh geometry
  
  Graphics_BeginFrame(gfx, 0);
  // Iterate visible_list, call vtable[0x0C] (RenderTransparent) for each
}