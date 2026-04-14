// Graphics_RenderScene @ 0x00454BC0
// Top-level 3D scene render: lights, view matrix, render callbacks, state restore
//
// Flow:
//   1. Graphics_SetupLights(gfx)
//   2. Save current view/world/projection matrices to local stack
//   3. Gfx_SetViewMatrix(gfx, projection_matrix) with D3DTS VieW=3
//   4. D3DDevice->SetTransform(D3DTS_VIEW, saved_view) via vtable[0x94]
//   5. Copy projection to scene, Matrix_ComputeFrustum(frustum+0x748)
//   6. D3DDevice->SetTransform(D3DTS_PROJECTION, proj) (0x100=D3DTS_PROJECTION)
//   7. Copy view back to scene, Matrix_ComputeFrustum again
//   8. Iterate 8 render callback slots (this+0x710 array):
//      - If slot != NULL: call vtable[0x0C]() (pre-render callback)
//   9. Restore D3D render states:
//      - D3DRS_CULLMODE (0x8B) = saved value
//      - D3DRS_LIGHTING (0x1C) = saved value
//   10. Graphics_SetViewportZ(near, far)
//   11. D3DRS_ZENABLE (0x22) = saved value
//   12. Call scene->vtable[0x88]() (post-render-1)
//   13. Call scene->vtable[0x78]() (post-render-2)

void Graphics_RenderScene(Graphics *gfx)
{
  Graphics_SetupLights(gfx);
  
  // Save matrices
  Matrix view = gfx->view_matrix;       // +0x224
  Matrix projection = gfx->projection;  // +0x264  
  Matrix world = gfx->world_matrix;     // +0x2A4
  
  // Setup view transform
  Gfx_SetViewMatrix(gfx, projection);
  D3DDevice->SetTransform(D3DTS_VIEW, &view);  // vtable[0x94]
  
  // Setup projection
  memcpy(gfx->world_matrix, saved_world, 64);
  Matrix_ComputeFrustum(gfx->frustum);  // +0x748
  D3DDevice->SetTransform(D3DTS_PROJECTION, &projection);  // vtable[0x94], 0x100
  
  // Copy back view
  memcpy(gfx->view_matrix, &view, 64);
  Matrix_ComputeFrustum(gfx->frustum);
  
  // Render callbacks (8 slots: lights, shadows, etc.)
  for (int i = 0; i < 8; i++) {
    if (gfx->render_callbacks[i])  // +0x710
      gfx->render_callbacks[i]->vtable[0x0C]();
  }
  
  // Restore render states
  D3DDevice->SetRenderState(D3DRS_CULLMODE, gfx->cull_mode);    // +0x730
  D3DDevice->SetRenderState(D3DRS_LIGHTING, gfx->lighting_on);    // +0x734
  Graphics_SetViewportZ(gfx, gfx->viewport_near, gfx->viewport_far);  // +0x73C, +0x740
  D3DDevice->SetRenderState(D3DRS_ZENABLE, gfx->z_enable);       // +0x738
  
  // Post-render
  gfx->scene_vtable[0x88]();
  gfx->scene_vtable[0x78]();
}