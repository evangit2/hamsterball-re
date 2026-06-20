
undefined4 __thiscall D3D_GetAdapterMode(void *this,uint param_1)

{
  undefined4 uVar1;
  
  if (param_1 < *(uint *)((int)this + 0x3c)) {
    uVar1 = *(undefined4 *)
             (&DAT_004db348 + (*(uint *)this >> ((char)param_1 * '\x02' + 0x10U & 0x1f) & 3) * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

