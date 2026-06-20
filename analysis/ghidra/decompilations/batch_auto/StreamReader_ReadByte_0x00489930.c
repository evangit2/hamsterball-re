
/* WARNING: Removing unreachable block (ram,0x00489979) */

undefined1 __cdecl StreamReader_ReadByte(int *param_1)

{
  undefined4 *puVar1;
  int local_c;
  
  puVar1 = (undefined4 *)*param_1;
  local_c = 0;
  if (puVar1 != (undefined4 *)0x0) {
    local_c = puVar1[1] + *(int *)*puVar1;
  }
  AthenaList_IterateNext();
  return *(undefined1 *)(local_c + 4);
}

