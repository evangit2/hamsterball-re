
/* Library Function - Multiple Matches With Different Base Names
    __fread_lk
    _fread
   
   Library: Visual Studio 2003 Release */

size_t __cdecl FID_conflict___fread_lk(void *_DstBuf,size_t _ElementSize,size_t _Count,FILE *_File)

{
  void *pvVar1;
  int iVar2;
  undefined1 *puVar3;
  void *pvVar4;
  void *_Size;
  void *local_8;
  
  pvVar4 = (void *)(_ElementSize * _Count);
  if (pvVar4 == (void *)0x0) {
    _Count = 0;
  }
  else {
    puVar3 = _DstBuf;
    _DstBuf = pvVar4;
    if ((_File->_flag & 0x10c) == 0) {
      local_8 = (void *)0x1000;
    }
    else {
      local_8 = (void *)_File->_bufsiz;
    }
    do {
      if (((_File->_flag & 0x10c) == 0) || (pvVar1 = (void *)_File->_cnt, pvVar1 == (void *)0x0)) {
        if (_DstBuf < local_8) {
          iVar2 = __filbuf(_File);
          if (iVar2 == -1) goto LAB_004bcd49;
          *puVar3 = (char)iVar2;
          local_8 = (void *)_File->_bufsiz;
          puVar3 = puVar3 + 1;
          _DstBuf = (void *)((int)_DstBuf + -1);
        }
        else {
          pvVar1 = _DstBuf;
          if (local_8 != (void *)0x0) {
            pvVar1 = (void *)((int)_DstBuf - (uint)_DstBuf % (uint)local_8);
          }
          iVar2 = __read(_File->_file,puVar3,(uint)pvVar1);
          if (iVar2 == 0) {
            _File->_flag = _File->_flag | 0x10;
LAB_004bcd49:
            return (uint)((int)pvVar4 - (int)_DstBuf) / _ElementSize;
          }
          if (iVar2 == -1) {
            _File->_flag = _File->_flag | 0x20;
            goto LAB_004bcd49;
          }
          _DstBuf = (void *)((int)_DstBuf - iVar2);
          puVar3 = puVar3 + iVar2;
        }
      }
      else {
        _Size = _DstBuf;
        if (pvVar1 <= _DstBuf) {
          _Size = pvVar1;
        }
        _memcpy(puVar3,_File->_ptr,(size_t)_Size);
        _DstBuf = (void *)((int)_DstBuf - (int)_Size);
        _File->_cnt = _File->_cnt - (int)_Size;
        _File->_ptr = _File->_ptr + (int)_Size;
        puVar3 = puVar3 + (int)_Size;
      }
    } while (_DstBuf != (void *)0x0);
  }
  return _Count;
}

