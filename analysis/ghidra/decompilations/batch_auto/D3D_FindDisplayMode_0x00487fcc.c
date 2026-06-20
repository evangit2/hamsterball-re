
int * __cdecl D3D_FindDisplayMode(int param_1)

{
  int *piVar1;
  
  piVar1 = &DAT_004db5a8;
  while( true ) {
    if (PTR_DAT_004f7fd0 <= piVar1) {
      return (int *)&DAT_004db580;
    }
    if (param_1 == *piVar1) break;
    piVar1 = piVar1 + 9;
  }
  return piVar1;
}

