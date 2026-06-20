
void __thiscall Graphics_SetScaleAndPosition(void *this,int param_1,int param_2)

{
  undefined1 auStack_1c [20];
  
  Matrix_Scale4x4(auStack_1c,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
  Sprite_DrawRect(this,(float)param_1,(float)param_2);
  return;
}

