
void __fastcall App_StartRace(int param_1)

{
  int iVar1;
  int *piVar2;
  char unaff_SI;
  
  Scene_UpdateChildren(*(int *)(param_1 + 0x178));
  iVar1 = *(int *)(param_1 + 0x174);
  *(undefined1 *)(iVar1 + 0x7d2) = 0;
  Gfx_SetCullMode(iVar1);
  iVar1 = *(int *)(param_1 + 0x174);
  piVar2 = *(int **)(iVar1 + 0x154);
  if (*(char *)(iVar1 + 0x7d2) == '\0') {
    (**(code **)(*piVar2 + 200))(piVar2,0x16,3);
  }
  else {
    (**(code **)(*piVar2 + 200))(piVar2,0x16,2);
  }
  *(undefined4 *)(iVar1 + 0x708) = 3;
  if (*(int **)(param_1 + 0x21c) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x21c) + 0x40))();
    *(undefined4 *)(param_1 + 0x21c) = 0;
  }
  if (*(int **)(param_1 + 0x228) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x228) + 0x40))();
    *(undefined4 *)(param_1 + 0x228) = 0;
  }
  if (*(int **)(param_1 + 0x224) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x224) + 0x40))();
    *(undefined4 *)(param_1 + 0x224) = 0;
  }
  if (*(undefined4 **)(param_1 + 0x220) != (undefined4 *)0x0) {
    (**(code **)**(undefined4 **)(param_1 + 0x220))(1);
    *(undefined4 *)(param_1 + 0x220) = 0;
  }
  if ((unaff_SI != '\0') && (*(void **)(param_1 + 0x534) != (void *)0x0)) {
    MusicPlayer_SetTempoScale(*(void **)(param_1 + 0x534),1.0);
  }
  if (*(void **)(param_1 + 0x53c) != (void *)0x0) {
    MusicPlayer_SetTempoScale(*(void **)(param_1 + 0x53c),0.5);
    return;
  }
  return;
}

