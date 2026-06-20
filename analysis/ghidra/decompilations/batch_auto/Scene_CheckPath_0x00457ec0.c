
/* Scene_CheckPath: Ring topology pathfinder on 360-cell (0x167) circular grid. Used in Ball_Update
   for angular collision/track snapping. Returns 1=forward reachable, -1=backward reachable,
   0=unreachable. */

undefined4 Scene_CheckPath(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = 0;
  iVar2 = param_1;
  while( true ) {
    if (param_1 == param_2) {
      return 1;
    }
    if (iVar2 == param_2) break;
    iVar1 = param_1 + 1;
    if (0x167 < iVar1) {
      iVar1 = param_1 + -0x167;
    }
    iVar3 = iVar2 + -1;
    if (iVar3 < 0) {
      iVar3 = iVar2 + 0x167;
    }
    iVar4 = iVar4 + 1;
    param_1 = iVar1;
    iVar2 = iVar3;
    if (0x167 < iVar4) {
      return 0;
    }
  }
  return 0xffffffff;
}

