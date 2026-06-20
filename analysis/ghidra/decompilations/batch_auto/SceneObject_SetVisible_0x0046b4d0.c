
void __thiscall SceneObject_SetVisible(void *this,undefined1 param_1)

{
  *(undefined1 *)((int)this + 0x88) = param_1;
  (**(code **)(*(int *)this + 0xc))();
  return;
}

