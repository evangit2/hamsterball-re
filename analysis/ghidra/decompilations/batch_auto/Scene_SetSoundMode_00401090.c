/*
 * Function: Scene_SetSoundMode
 * Address: 0x00401090
 * Signature: void __thiscall Scene_SetSoundMode(void *this, int param_1, char param_2)
 *
 * Description:
 * Sets the sound mode on the Scene's audio system (at this+0x154).
param_1: sound mode (1=stereo, 2=3D/hardware, 3=surround). If Scene+0x7D2 flag is set
(high-end audio), mode 2 is clamped to 3. Calls audio vtable[200/4=50] (Sound_SetMode).
If param_2 is non-zero, persists the mode to Scene+0x708.
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 * Ghidra decompilation - auto-batch 1
 */

void __thiscall Scene_SetSoundMode(void *this,int param_1,char param_2)
{
  int iVar1;
  iVar1 = param_1;
  if (*(char *)((int)this + 0x7d2) != '\0') {
    if (param_1 == 1) {
      (**(code **)(**(int **)((int)this + 0x154) + 200))(*(int **)((int)this + 0x154),0x16,1);
      goto LAB_004010ed;
    }
    if (param_1 != 2) {
      if (param_1 == 3) {
        (**(code **)(**(int **)((int)this + 0x154) + 200))(*(int **)((int)this + 0x154),0x16,2);
      }
      goto LAB_004010ed;
    }
    iVar1 = 3;
  }
  (**(code **)(**(int **)((int)this + 0x154) + 200))(*(int **)((int)this + 0x154),0x16,iVar1);
LAB_004010ed:
  if (param_2 != '\0') {
    *(int *)((int)this + 0x708) = param_1;
  }
  return;
}
