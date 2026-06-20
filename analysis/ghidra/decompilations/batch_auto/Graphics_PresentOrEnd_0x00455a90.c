
void __thiscall Graphics_PresentOrEnd(void *this,char param_1)

{
  int iVar1;
  tagRECT local_20;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  if (param_1 == '\x01') {
    iVar1 = (**(code **)(**(int **)((int)this + 0x154) + 0x3c))
                      (*(int **)((int)this + 0x154),0,0,0,0);
    if (iVar1 != -0x7789f798) goto LAB_00455b70;
    (**(code **)(**(int **)((int)this + 0x154) + 0x38))
              (*(int **)((int)this + 0x154),*(undefined4 *)((int)this + 400));
  }
  else {
    GetClientRect(*(HWND *)((int)this + 0x6c),&local_20);
    local_10 = *(int *)((int)this + 0x70) / 2 - (local_20.right - local_20.left) / 2;
    local_c = *(int *)((int)this + 0x74) / 2 - (local_20.bottom - local_20.top) / 2;
    local_8 = local_10 + (local_20.right - local_20.left);
    local_4 = local_c + (local_20.bottom - local_20.top);
    iVar1 = (**(code **)(**(int **)((int)this + 0x154) + 0x3c))
                      (*(int **)((int)this + 0x154),&local_10,0,0,0);
    if (iVar1 != -0x7789f798) goto LAB_00455b70;
    (**(code **)(**(int **)((int)this + 0x154) + 0x38))
              (*(int **)((int)this + 0x154),*(undefined4 *)((int)this + 400));
  }
  Graphics_RenderScene(this);
LAB_00455b70:
  *(int *)((int)this + 0x7c4) = *(int *)((int)this + 0x7c4) + 1;
  return;
}

