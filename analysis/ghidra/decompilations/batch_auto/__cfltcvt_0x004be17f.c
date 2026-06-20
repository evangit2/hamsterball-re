
/* Library Function - Single Match
    __cfltcvt
   
   Library: Visual Studio 2003 Release */

errno_t __cdecl
__cfltcvt(double *arg,char *buffer,size_t sizeInBytes,int format,int precision,int caps)

{
  undefined1 *puVar1;
  errno_t eVar2;
  
  if ((sizeInBytes == 0x65) || (sizeInBytes == 0x45)) {
    eVar2 = CRT_cvtToScientific((undefined4 *)arg,(int)buffer,format,precision);
  }
  else {
    if (sizeInBytes == 0x66) {
      puVar1 = CRT_cftof((undefined4 *)arg,buffer,format);
      return (errno_t)puVar1;
    }
    eVar2 = CRT_cftoe((undefined4 *)arg,buffer,format,precision);
  }
  return eVar2;
}

