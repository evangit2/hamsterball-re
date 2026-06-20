
undefined4 __cdecl PNG_SetTransformFiller(int param_1)

{
  if (*(char *)(param_1 + 0x113) != '\0') {
    *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) | 2;
    return 7;
  }
  return 1;
}

