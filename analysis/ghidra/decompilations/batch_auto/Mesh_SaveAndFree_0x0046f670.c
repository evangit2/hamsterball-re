
void __fastcall Mesh_SaveAndFree(undefined4 *param_1)

{
  int *_Buf;
  undefined4 *puVar1;
  int iVar2;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((((char *)*param_1 != (char *)0x0) && (*(char *)(param_1 + 1) == '\0')) &&
     (*(char *)(param_1[0xb] + 0x7d1) != '\0')) {
    iVar2 = CRT_OpenFileExclusive((char *)*param_1,0x180);
    __close(iVar2);
    iVar2 = FID_conflict___open((char *)*param_1,0x8002);
    local_8 = 0xbeef;
    local_4 = *(undefined4 *)(*(int *)(param_1[0xb] + 0x5c) + 0x40);
    __write(iVar2,&local_8,4);
    __write(iVar2,&local_4,4);
    __write(iVar2,param_1 + 7,4);
    __write(iVar2,(void *)param_1[3],param_1[7] << 5);
    _Buf = param_1 + 8;
    __write(iVar2,_Buf,4);
    __write(iVar2,(void *)param_1[4],*_Buf << 1);
    __write(iVar2,(void *)param_1[6],(*_Buf / 3) * 4);
    __close(iVar2);
  }
  _free((void *)*param_1);
  *param_1 = 0;
  _free((void *)param_1[6]);
  param_1[6] = 0;
  _free((void *)param_1[3]);
  param_1[3] = 0;
  _free((void *)param_1[4]);
  puVar1 = (undefined4 *)param_1[5];
  param_1[4] = 0;
  if (puVar1 != (undefined4 *)0x0) {
    if (puVar1[-1] != 0) {
      (**(code **)*puVar1)(3);
      param_1[5] = 0;
      return;
    }
    _free(puVar1 + -1);
  }
  param_1[5] = 0;
  return;
}

