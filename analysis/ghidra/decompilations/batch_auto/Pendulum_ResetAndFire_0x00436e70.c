
void __thiscall Pendulum_ResetAndFire(void *this,char param_1)

{
  undefined4 *puVar1;
  undefined4 local_c [3];
  
  *(undefined1 *)((int)this + 0x10fc) = 1;
  puVar1 = (undefined4 *)((int)this + 0x10e8);
  if (param_1 != '\0') {
    if (puVar1 != local_c) {
      *puVar1 = 0xc0a00000;
      *(undefined4 *)((int)this + 0x10ec) = 0x41a00000;
      *(undefined4 *)((int)this + 0x10f0) = 0;
    }
    *(undefined4 *)((int)this + 0x10f8) = 0xc0a00000;
    return;
  }
  if (puVar1 != local_c) {
    *puVar1 = 0x40a00000;
    *(undefined4 *)((int)this + 0x10ec) = 0x41a00000;
    *(undefined4 *)((int)this + 0x10f0) = 0;
  }
  *(undefined4 *)((int)this + 0x10f8) = 0x40a00000;
  return;
}

