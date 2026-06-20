
void * __thiscall SpriteAnim_DeletingDtor(void *this,byte param_1)

{
  *(undefined ***)this = &PTR_SpriteAnim_DeletingDtor_004d9c48;
  if (*(char *)((int)this + 0xd) != '\0') {
    if (*(undefined4 **)((int)this + 8) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)((int)this + 8))(1);
    }
    *(undefined4 *)((int)this + 8) = 0;
  }
  if (*(char *)((int)this + 0xe) != '\0') {
    if (*(void **)((int)this + 0x10) != (void *)0x0) {
      _free(*(void **)((int)this + 0x10));
    }
    *(undefined4 *)((int)this + 0x10) = 0;
  }
  if ((param_1 & 1) != 0) {
    _free(this);
  }
  return this;
}

