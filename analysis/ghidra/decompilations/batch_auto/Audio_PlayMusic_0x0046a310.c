
void __thiscall Audio_PlayMusic(void *this,char *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  char unaff_BP;
  undefined4 *puVar4;
  
  iVar2 = AthenaList_NextIndex((int)this + 0xc);
  *(undefined4 *)((int)this + iVar2 * 4 + 0x14) = 0;
  if (*(int *)((int)this + 0x10) < 1) {
    puVar4 = (undefined4 *)0x0;
  }
  else {
    puVar4 = (undefined4 *)**(undefined4 **)((int)this + 0x418);
    *(undefined4 *)((int)this + iVar2 * 4 + 0x14) = 1;
  }
  if (puVar4 != (undefined4 *)0x0) {
    while (iVar3 = __stricmp((char *)*puVar4,param_1), iVar3 != 0) {
      iVar3 = *(int *)((int)this + iVar2 * 4 + 0x14);
      if (*(int *)((int)this + 0x10) <= iVar3) {
        return;
      }
      puVar4 = *(undefined4 **)(*(int *)((int)this + 0x418) + iVar3 * 4);
      *(int *)((int)this + iVar2 * 4 + 0x14) = iVar3 + 1;
      if (puVar4 == (undefined4 *)0x0) {
        return;
      }
    }
    uVar1 = puVar4[1];
    *(undefined4 *)((int)this + 0x528) = 0x3f800000;
    BASS_ChannelSetAttributes(*(undefined4 *)((int)this + 8),0xffffffff,100,0xffffff9b);
    BASS_MusicPlayEx(*(undefined4 *)((int)this + 8),uVar1,-(uint)(unaff_BP != '\0'),1);
  }
  return;
}

