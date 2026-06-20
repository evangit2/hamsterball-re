
void CRT_LeaveCritSec12(void)

{
  LeaveCriticalSection_indexed(0xc);
  return;
}

