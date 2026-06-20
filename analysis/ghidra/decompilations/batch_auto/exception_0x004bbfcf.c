
/* Library Function - Single Match
    public: __thiscall exception::exception(class exception const &)
   
   Library: Visual Studio 2003 Release */

exception * __thiscall exception::exception(exception *this,exception *param_1)

{
  int iVar1;
  size_t sVar2;
  char *_Dest;
  
  *(undefined ***)this = &PTR_Exception_ScalarDtor_004e9b24;
  iVar1 = *(int *)(param_1 + 8);
  *(int *)(this + 8) = iVar1;
  if (iVar1 == 0) {
    *(undefined4 *)(this + 4) = *(undefined4 *)(param_1 + 4);
  }
  else {
    sVar2 = _strlen(*(char **)(param_1 + 4));
    _Dest = _malloc(sVar2 + 1);
    *(char **)(this + 4) = _Dest;
    if (_Dest != (char *)0x0) {
      strcpy(_Dest,*(char **)(param_1 + 4));
    }
  }
  return this;
}

