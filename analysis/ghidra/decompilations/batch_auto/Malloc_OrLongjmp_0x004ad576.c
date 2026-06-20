
void * __cdecl Malloc_OrLongjmp(int *param_1,size_t param_2)

{
  void *pvVar1;
  
  if ((param_1 != (int *)0x0) && (param_2 != 0)) {
    pvVar1 = _malloc(param_2);
    if (pvVar1 == (void *)0x0) {
      longjmp_with_cleanup(param_1,"Out of Memory");
    }
    return pvVar1;
  }
  return (void *)0x0;
}

