
void __cdecl Media_InitParser(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  
  puVar1 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,0,0x5c);
  *(undefined4 **)(param_1 + 400) = puVar1;
  *puVar1 = Media_ResetContext;
  *(undefined1 **)(*(int *)(param_1 + 400) + 4) = &LAB_004a3faa;
  *(code **)(*(int *)(param_1 + 400) + 8) = Image_ProcessRestartMarker;
  *(code **)(*(int *)(param_1 + 400) + 0xc) = MIDI_ReadChunkData;
  iVar2 = 0x10;
  do {
    *(code **)(iVar2 + *(int *)(param_1 + 400)) = MIDI_ReadChunkData;
    iVar2 = iVar2 + 4;
  } while (iVar2 < 0x50);
  *(code **)(*(int *)(param_1 + 400) + 0x10) = Image_ParseJFIF;
  *(code **)(*(int *)(param_1 + 400) + 0x48) = Image_ParseAdobeMarker;
  Media_ResetContext(param_1);
  return;
}

