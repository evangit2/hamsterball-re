
void __thiscall TipperVisual_Attach(void *this,int param_1)

{
  if ((*(char *)((int)this + 0x431) != '\0') &&
     (*(undefined4 **)((int)this + 0x434) != (undefined4 *)0x0)) {
    (**(code **)**(undefined4 **)((int)this + 0x434))(1);
  }
  *(undefined1 *)((int)this + 0x431) = 0;
  *(undefined4 *)((int)this + 0x434) = *(undefined4 *)(param_1 + 0x434);
  return;
}

