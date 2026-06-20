/*
 * Function: Scene_Render
 * Address: 0x0041a2e0
 * Signature: Scene_Render(...)
 *
 * Patterns: vtable dispatch, audio, rendering, camera, scene. Calls: BG, Opaque, Transparent, Overlay, PostFX, Scene_Render, AthenaList_GetSize, Graphics_SetViewport. Offsets: 19, Lines: 80
 *
 * Decompiled from Hamsterball.exe (Athena Engine, PE32 i386)
 */

/* Scene_Render: 1P/2P split screen. Per viewport:
   SetCamera->BG(vtable60)->Opaque(64)->Transparent(68)->Overlay(70)->PostFX(6C). Player list at
   +0x362C. See docs/SCENE_SYSTEM_DECOMP.md */

void __thiscall Scene_Render(void *this,int *param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  void *pvVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  ulonglong uVar7;
  
  iVar5 = (int)this + 0x362c;
  iVar4 = AthenaList_GetSize(iVar5);
  if (iVar4 == 0) {
    Graphics_SetViewport(param_1,0,0);
    (**(code **)(*param_1 + 4))(*(undefined4 *)((int)this + 0x87c));
    (**(code **)(*(int *)this + 0x60))(param_1);
    (**(code **)(*(int *)this + 100))(param_1);
    (**(code **)(*(int *)this + 0x68))(param_1);
    Graphics_SetViewport(param_1,0,0);
    (**(code **)(*(int *)this + 0x70))(param_1);
    (**(code **)(*(int *)this + 0x6c))(param_1);
  }
  iVar4 = AthenaList_GetSize(iVar5);
  if (iVar4 == 1) {
    Graphics_SetViewport(param_1,0,0);
    if ((0 < *(int *)((int)this + 0x3630)) && (**(int **)((int)this + 0x3a38) != 0)) {
      iVar4 = *(int *)(**(int **)((int)this + 0x3a38) + 0x10);
      *(int *)((int)this + 0x29d0) = iVar4;
      uVar1 = *(undefined4 *)(iVar4 + 0x168);
      uVar2 = *(undefined4 *)(iVar4 + 0x16c);
      iVar6 = *(int *)(*(int *)((int)this + 0x878) + 0x178);
      *(undefined4 *)(iVar6 + 0x854) = *(undefined4 *)(iVar4 + 0x164);
      *(undefined4 *)(iVar6 + 0x858) = uVar1;
      *(undefined4 *)(iVar6 + 0x85c) = uVar2;
      Scene_SetCamera(this,*(void **)((int)this + 0x29d0),'\x01');
    }
    (**(code **)(*(int *)this + 0x60))(param_1);
    (**(code **)(*(int *)this + 100))(param_1);
    (**(code **)(*(int *)this + 0x68))(param_1);
    Graphics_SetViewport(param_1,0,0);
    (**(code **)(*(int *)this + 0x70))(param_1);
    (**(code **)(*(int *)this + 0x6c))(param_1);
  }
  iVar4 = AthenaList_GetSize(iVar5);
  if (iVar4 == 2) {
    iVar5 = AthenaList_NextIndex(iVar5);
    *(undefined4 *)((int)this + iVar5 * 4 + 0x3634) = 0;
    if (*(int *)((int)this + 0x3630) < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = **(int **)((int)this + 0x3a38);
      *(undefined4 *)((int)this + iVar5 * 4 + 0x3634) = 1;
    }
    while (iVar4 != 0) {
      uVar7 = __ftol2();
      iVar6 = (int)uVar7;
      uVar7 = __ftol2();
      Graphics_SetViewport(param_1,(int)uVar7,iVar6);
      pvVar3 = *(void **)(iVar4 + 0x10);
      *(void **)((int)this + 0x29d0) = pvVar3;
      Scene_SetCamera(this,pvVar3,'\x01');
      (**(code **)(*(int *)this + 0x60))(param_1);
      (**(code **)(*(int *)this + 100))(param_1);
      (**(code **)(*(int *)this + 0x68))(param_1);
      iVar6 = *(int *)((int)this + iVar5 * 4 + 0x3634);
      if (*(int *)((int)this + 0x3630) <= iVar6) break;
      iVar4 = *(int *)(*(int *)((int)this + 0x3a38) + iVar6 * 4);
      *(int *)((int)this + iVar5 * 4 + 0x3634) = iVar6 + 1;
    }
    Graphics_SetViewport(param_1,0,0);
    (**(code **)(*(int *)this + 0x70))(param_1);
    (**(code **)(*(int *)this + 0x6c))(param_1);
  }
  return;
}
