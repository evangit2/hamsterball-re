
void __thiscall SpriteAnim_SetRange(void *this,undefined4 param_1)

{
  *(undefined4 *)((int)this + 0x24) = *(undefined4 *)((int)this + 0x1c);
  *(undefined4 *)((int)this + 0x28) = param_1;
  return;
}

