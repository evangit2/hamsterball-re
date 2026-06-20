
void __thiscall App_ParseGUID(void *this,LPCSTR param_1)

{
  char cVar1;
  LPCSTR pCVar2;
  LPCSTR pCVar3;
  HRESULT HVar4;
  int iVar5;
  WCHAR *pWVar6;
  char local_308 [256];
  WCHAR local_208 [258];
  
  pWVar6 = local_208;
  for (iVar5 = 0x80; iVar5 != 0; iVar5 = iVar5 + -1) {
    pWVar6[0] = L'\0';
    pWVar6[1] = L'\0';
    pWVar6 = pWVar6 + 2;
  }
  pCVar2 = param_1;
  do {
    cVar1 = *pCVar2;
    pCVar2 = pCVar2 + 1;
  } while (cVar1 != '\0');
  pCVar3 = param_1;
  do {
    cVar1 = *pCVar3;
    pCVar3 = pCVar3 + 1;
  } while (cVar1 != '\0');
  MultiByteToWideChar(0,0,param_1,(int)pCVar3 - (int)(param_1 + 1),local_208,
                      (int)(pCVar2 + (1 - (int)(param_1 + 1))));
  HVar4 = CLSIDFromString(local_208,(LPCLSID)((int)this + 0x44));
  if (HVar4 != 0) {
    AthenaString_SprintfToBuffer(local_308,(byte *)"Could not generate GUID : %s");
    MessageBoxA((HWND)0x0,local_308,"GUID Error",0);
  }
  return;
}

