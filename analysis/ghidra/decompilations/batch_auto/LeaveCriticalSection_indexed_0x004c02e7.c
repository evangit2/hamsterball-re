
void __cdecl LeaveCriticalSection_indexed(int param_1)

{
  LeaveCriticalSection((LPCRITICAL_SECTION)(&DAT_004fc988)[param_1 * 2]);
  return;
}

