
void CRT_SetFPUAffinity(void)

{
  __controlfp(0x10000,0x30000);
  return;
}

