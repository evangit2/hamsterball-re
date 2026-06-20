
/* Library Function - Single Match
    public: __thiscall exception::exception(char const * const &)
   
   Library: Visual Studio 2003 Release */

exception * __thiscall exception::exception(exception *this,char **param_1)

{
  size_t sVar1;
  char *_Dest;
  
  *(undefined ***)this = &PTR_Exception_ScalarDtor_004e9b24;
  sVar1 = _strlen(*param_1);
  _Dest = _malloc(sVar1 + 1);
  *(char **)(this + 4) = _Dest;
  if (_Dest != (char *)0x0) {
    strcpy(_Dest,*param_1);
  }
  *(undefined4 *)(this + 8) = 1;
  return this;
}

