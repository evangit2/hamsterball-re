
void __thiscall App_FrameTick(void *this,undefined4 param_1)

{
  if (*(char *)((int)this + 0x159) == '\0') {
    (**(code **)(*(int *)this + 0x60))();
    MeshWorld_CallVtable34(*(void **)((int)this + 0x184),param_1);
    MusicDevice_MuteToggle((char)param_1);
  }
  return;
}

