// Function: App_CreateGraphics
// Address: 0x00429350
// Decompiled: 2026-06-21
// Source: GhidraMCP native tool (mcp_ghidra_mcp_batch_decompile)

void __fastcall App_CreateGraphics(int param_1)
{
  void *pvVar1;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_004cae8b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  
  // Allocate Graphics object (0x7DC = 2012 bytes)
  pvVar1 = operator_new(0x7dc);
  local_4 = 0;
  if (pvVar1 == (void *)0x0) {
    pvVar1 = (void *)0x0;
  }
  else {
    // Construct Graphics: ctor(obj, App, 1) — 1 = windowed mode flag
    pvVar1 = Graphics_ctor(pvVar1, param_1, 1);
  }
  // Store Graphics pointer at App+0x174
  *(void **)(param_1 + 0x174) = pvVar1;
  
  ExceptionList = local_c;
  return;
}
