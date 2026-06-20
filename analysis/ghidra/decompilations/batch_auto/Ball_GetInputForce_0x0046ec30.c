
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* Ball_GetInputForce(this, output[2]): Convert input to 2D force vector. Mode 1=keyboard (DIK codes
   at +0x50C-518), Mode 2=mouse (cursor offset from center), Mode 4-7=joystick (axes at +0x10C/110).
   output[0]=forceX, output[1]=forceY. See decomp_ball_input_audio.c */

void __thiscall Ball_GetInputForce(void *this,float *param_1)

{
  float fVar1;
  int iVar2;
  float fVar3;
  int Y;
  int iVar4;
  int X;
  float local_10;
  tagPOINT local_c;
  undefined4 local_4;
  
  local_10 = 0.0;
  fVar3 = _DAT_004cf368;
  switch(*(undefined4 *)((int)this + 8)) {
  case 1:
    iVar2 = *(int *)(*(int *)((int)this + 4) + 0x434);
    if ((*(byte *)(*(int *)(iVar2 + 0x514) + 0xc + iVar2) & 0x80) != 0) {
      fVar3 = _DAT_004d0250;
    }
    if ((*(byte *)(*(int *)(iVar2 + 0x518) + 0xc + iVar2) & 0x80) != 0) {
      fVar3 = _DAT_004cf310;
    }
    if ((*(byte *)(*(int *)(iVar2 + 0x50c) + 0xc + iVar2) & 0x80) != 0) {
      local_10 = -1.0;
    }
    if ((*(byte *)(*(int *)(iVar2 + 0x510) + 0xc + iVar2) & 0x80) != 0) {
      local_10 = 1.0;
    }
    break;
  case 2:
    GetCursorPos(&local_c);
    iVar2 = *(int *)(*(int *)((int)this + 4) + 4);
    X = *(int *)(iVar2 + 0x15c) / 2;
    iVar4 = local_c.x - X;
    Y = *(int *)(iVar2 + 0x160) / 2;
    local_10 = (float)(local_c.y - Y);
    fVar3 = (float)iVar4;
    if (*(char *)(iVar2 + 0x15a) != '\0') {
      SetCursorPos(X,Y);
      fVar3 = (float)iVar4;
    }
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    iVar2 = *(int *)((int)this + 0x10);
    if (iVar2 != 0) {
      local_c.x = (LONG)(float)(*(int *)(iVar2 + 0x10c) / 100);
      local_c.y = (LONG)(float)(*(int *)(iVar2 + 0x110) / 100);
      local_4 = 0;
      Vec3_NormalizeAndScale(&local_c,1.0);
      local_10 = (float)local_c.y;
      fVar3 = (float)local_c.x;
    }
  }
  fVar1 = *(float *)((int)this + 0xc);
  *param_1 = fVar1 * fVar3;
  param_1[1] = fVar1 * local_10;
  return;
}

