// Function: MusicPlayer_Render
// Address: 0x00426150
// Decompiled: 2026-06-21
// Source: GhidraMCP native tool (mcp_ghidra_mcp_batch_decompile)

void __thiscall MusicPlayer_Render(void *this,void *param_1)
{
  int iVar1;
  byte *pbVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 in_stack_ffffffcc;
  int *in_stack_ffffffd0;
  undefined4 in_stack_ffffffd4;
  undefined4 in_stack_ffffffd8;
  undefined4 uVar8;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_004cac58;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  
  // Clear screen (800x600)
  Matrix_Scale4x4(&stack0xffffffcc,0,0,0x3e800000,0x3f800000);
  Graphics_DrawScreenRect(param_1,0,0,800,600);
  
  // State-based animation: param_1+5 is a state byte (0 or 1)
  if (*(char *)((int)param_1 + 5) != '\0') {
    in_stack_ffffffd0 = *(int **)((int)param_1 + 0x154);
    in_stack_ffffffd8 = 0x10;
    in_stack_ffffffd4 = 0;
    (**(code **)(*in_stack_ffffffd0 + 0xfc))();
    in_stack_ffffffcc = 2;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    *(undefined1 *)((int)param_1 + 5) = 0;
  }
  
  // Draw 4 quadrant backgrounds (400x300 each = full 800x600 screen)
  uVar8 = 0x4261d8;
  Gfx_SetAlphaBlendState((int)param_1);
  
  // Top-left: (0,0) to (400,300)
  Matrix_Scale4x4(&stack0xffffffcc,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x418),&PTR_LAB_004cf584,
    0.0,0.0,400.0,300.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  // Top-right: (400,0) to (400,300)
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x41c),&PTR_LAB_004cf584,
    400.0,0.0,400.0,300.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  // Bottom-left: (0,300) to (400,300)
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x420),&PTR_LAB_004cf584,
    0.0,300.0,400.0,300.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  // Bottom-right: (400,300) to (400,300)
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x424),&PTR_LAB_004cf584,
    400.0,300.0,400.0,300.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  // Two album art / icon areas (256x256 each)
  // Left icon at (130, 45)
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x348),&PTR_LAB_004cf584,
    130.0,45.0,256.0,256.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  // Right icon at (385, 48)
  Scene_CreateObject4f(*(void **)(*(int *)((int)this + 0x878) + 0x34c),&PTR_LAB_004cf584,
    385.0,48.0,256.0,256.0,in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  // Transition to next state
  uVar8 = 0x4263da;
  App_CompleteRace((int)param_1);
  if (*(char *)((int)param_1 + 5) != '\x01') {
    in_stack_ffffffd0 = *(int **)((int)param_1 + 0x154);
    uVar8 = 1;
    in_stack_ffffffd8 = 0x10;
    in_stack_ffffffd4 = 0;
    (**(code **)(*in_stack_ffffffd0 + 0xfc))();
    in_stack_ffffffcc = 1;
    (**(code **)(**(int **)((int)param_1 + 0x154) + 0xfc))();
    *(undefined1 *)((int)param_1 + 5) = 1;
  }
  
  // Draw track title text at (525, 265) — "Now Playing" style display
  uVar7 = 0x3f800000;
  uVar6 = 0;
  uVar5 = 0;
  uVar4 = 0;
  uVar3 = 0x42642a;
  Matrix_Scale4x4(&stack0xffffffcc,0,0,0,0x3f800000);
  uStack_4 = 0;
  Matrix_Scale4x4(&stack0xffffffb8,0,0x3f800000,0,0x3f800000);
  iVar1 = *(int *)((int)this + 0x878);
  uStack_4 = 0xffffffff;
  if (*(uint *)(iVar1 + 0x3c) < 0x10) {
    pbVar2 = (byte *)(iVar1 + 0x28);
  }
  else {
    pbVar2 = *(byte **)(iVar1 + 0x28);
  }
  UI_DrawTextShadow(*(void **)(iVar1 + 800),pbVar2,0x20d,0x109,3,3,uVar3,uVar4,uVar5,uVar6,uVar7,
    in_stack_ffffffcc,in_stack_ffffffd0,in_stack_ffffffd4,in_stack_ffffffd8,uVar8);
  
  ExceptionList = pvStack_c;
  return;
}
