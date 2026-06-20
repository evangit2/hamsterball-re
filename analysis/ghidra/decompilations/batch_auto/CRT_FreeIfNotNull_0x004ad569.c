
void __cdecl CRT_FreeIfNotNull(void *param_1)

{
  if (param_1 != (void *)0x0) {
    _free(param_1);
    return;
  }
  return;
}

