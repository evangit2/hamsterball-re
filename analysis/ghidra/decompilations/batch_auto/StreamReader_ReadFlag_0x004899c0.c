
/* WARNING: Removing unreachable block (ram,0x00489a09) */

byte __cdecl StreamReader_ReadFlag(int *param_1)

{
  undefined4 *puVar1;
  int local_c;
  
  puVar1 = (undefined4 *)*param_1;
  local_c = 0;
  if (puVar1 != (undefined4 *)0x0) {
    local_c = puVar1[1] + *(int *)*puVar1;
  }
  AthenaList_IterateNext();
  return *(byte *)(local_c + 5) & 1;
}

