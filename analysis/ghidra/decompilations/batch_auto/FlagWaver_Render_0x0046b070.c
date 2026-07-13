
/* FlagWaver_Render(this, heightY): Render flag wave. UpdateVertices, plane test for ball,
   alpha blend ON, ApplyMaterialAndDraw, indexed or non-indexed DrawPrimitiveUP, sound trigger. See
   decomp_water_collision.c */

void __thiscall FlagWaver_Render(void *this,float param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  short sVar5;
  undefined4 *unaff_ESI;
  
  FlagWaver_UpdateVertices((int)this);
  Graphics_BeginFrame(*(void **)((int)this + 4),(int)param_1);
  if (*(float *)((int)this + 0x10) <= *(float *)((int)this + 0x14)) {
    param_1 = *(float *)((int)this + 0x14);
  }
  else {
    param_1 = *(float *)((int)this + 0x10);
  }
  iVar1 = *(int *)((int)this + 4);
  sVar5 = Ball_TestPlaneIntersection(*(void **)(iVar1 + 0x748),0.0,0.0,0.0,param_1);
  if ((char)sVar5 != '\0') {
    iVar2 = *(int *)(iVar1 + 0x708);
    (**(code **)(**(int **)(iVar1 + 0x154) + 200))(*(int **)(iVar1 + 0x154),0x16,1);
    *(undefined4 *)(iVar1 + 0x708) = 1;
    Ball_InitRenderState(*(void **)((int)this + 4));
    if (unaff_ESI == (undefined4 *)0x0) {
      unaff_ESI = (undefined4 *)((int)this + 0x30);
    }
    Graphics_ApplyMaterialAndDraw(*(void **)((int)this + 4),unaff_ESI);
    iVar1 = *(int *)((int)this + 0x84);
    if (iVar1 != 0) {
      iVar3 = *(int *)((int)this + 4);
      piVar4 = *(int **)(iVar3 + 0x154);
      (**(code **)(*piVar4 + 0x14c))(piVar4,0,iVar1);
      *(int *)(iVar3 + 8) = iVar1;
      piVar4 = *(int **)(*(int *)((int)this + 4) + 0x154);
      (**(code **)(*piVar4 + 0x118))(piVar4,5,0,*(int *)((int)this + 0x88) + -2);
      *(int *)(*(int *)((int)this + 4) + 0x7cc) = *(int *)(*(int *)((int)this + 4) + 0x7cc) + 1;
      Scene_SetSoundMode(*(void **)((int)this + 4),iVar2,'\x01');
      return;
    }
    piVar4 = *(int **)(*(int *)((int)this + 4) + 0x154);
    (**(code **)(*piVar4 + 0x120))
              (piVar4,5,*(int *)((int)this + 8) << 1,*(undefined4 *)((int)this + 0xc),0x20);
    Scene_SetSoundMode(*(void **)((int)this + 4),iVar2,'\x01');
  }
  return;
}

