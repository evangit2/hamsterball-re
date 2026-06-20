
void __thiscall App_SetFullScreen(void *this,char param_1)

{
  int cy;
  int cx;
  HWND hWnd;
  int X;
  int Y;
  tagRECT tStack_10;
  
  *(char *)((int)this + 0x158) = param_1;
  Graphics_Reset(*(void **)((int)this + 0x174),*(undefined4 *)((int)this + 8),
                 *(undefined4 *)((int)this + 0x15c),*(undefined4 *)((int)this + 0x160),param_1);
  (**(code **)(*(int *)this + 0x88))();
  if (param_1 == '\x01') {
    SetWindowLongA(*(HWND *)((int)this + 8),-0x10,-0x6d760000);
    cy = *(int *)((int)this + 0x160);
    cx = *(int *)((int)this + 0x15c);
    hWnd = *(HWND *)((int)this + 8);
    Y = 0;
    X = 0;
  }
  else {
    SetWindowLongA(*(HWND *)((int)this + 8),-0x10,-0x6d360000);
    Graphics_Reset(*(void **)((int)this + 0x174),*(undefined4 *)((int)this + 8),
                   *(undefined4 *)((int)this + 0x15c),*(undefined4 *)((int)this + 0x160),
                   *(char *)((int)this + 0x158));
    tStack_10.right = *(int *)((int)this + 0x15c);
    tStack_10.bottom = *(int *)((int)this + 0x160);
    tStack_10.left = 0;
    tStack_10.top = 0;
    AdjustWindowRect(&tStack_10,0x92ca0000,0);
    hWnd = *(HWND *)((int)this + 8);
    cy = tStack_10.bottom - tStack_10.top;
    cx = tStack_10.right - tStack_10.left;
    Y = 0x3f;
    X = 0x3f;
  }
  SetWindowPos(hWnd,(HWND)0xfffffffe,X,Y,cx,cy,0x40);
  (**(code **)(*(int *)this + 0x60))();
  return;
}

