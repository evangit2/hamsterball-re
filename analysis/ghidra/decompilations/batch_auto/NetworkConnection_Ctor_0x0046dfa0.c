
void __thiscall NetworkConnection_Ctor(void *this,undefined4 param_1)

{
  *(undefined4 *)((int)this + 4) = param_1;
  *(undefined4 *)((int)this + 8) = 0;
  *(char **)this = "Not Connected";
  *(undefined4 *)((int)this + 0xc) = 0x3f800000;
  return;
}

