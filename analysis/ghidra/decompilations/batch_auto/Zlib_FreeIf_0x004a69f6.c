
void __cdecl Zlib_FreeIf(int param_1,void *param_2)

{
  if ((param_1 != 0) && (param_2 != (void *)0x0)) {
    _free(param_2);
  }
  return;
}

