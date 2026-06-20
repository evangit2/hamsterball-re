/*
 * Function: Ball_SetSpeed
 * Address: 0x004029c0
 * Signature: void __thiscall Ball_SetSpeed(void *this,float param_1)
 *
 * Patterns: ball. Calls: Ball_SetSpeed. Offsets: 7, Lines: 13
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

void __thiscall Ball_SetSpeed(void *this,float param_1)

{
  float local_c [3];
  
  *(float *)((int)this + 0xc64) = param_1;
  if ((float *)((int)this + 0xc98) != local_c) {
    *(float *)((int)this + 0xc98) = param_1 * *(float *)((int)this + 0xc8c);
    *(float *)((int)this + 0xc9c) = param_1 * *(float *)((int)this + 0xc90);
    *(float *)((int)this + 0xca0) = param_1 * *(float *)((int)this + 0xc94);
    return;
  }
  return;
}
