
void __cdecl
Mesh_RemoveEdgeFromHash(uint param_1,uint param_2,uint param_3,int param_4,uint param_5)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  
  puVar1 = (uint *)(param_4 + (param_1 % param_5) * 4);
  puVar4 = (uint *)*puVar1;
  puVar3 = (uint *)0x0;
  while( true ) {
    puVar2 = puVar4;
    if (puVar2 == (uint *)0x0) {
      return;
    }
    if (((puVar2[3] == param_3) && (puVar2[1] == param_2)) && (*puVar2 == param_1)) break;
    puVar4 = (uint *)puVar2[4];
    puVar3 = puVar2;
  }
  if (puVar3 != (uint *)0x0) {
    puVar3[4] = puVar2[4];
    return;
  }
  *puVar1 = puVar2[4];
  return;
}

