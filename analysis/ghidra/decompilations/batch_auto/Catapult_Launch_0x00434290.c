
/* Catapult_Launch(catapult): Set catapult+0x10F0=1 (active), catapult+0x10F4=50 (0x32 launch
   timer). Called on E:CATAPULTBOTTOM collision. */

void __fastcall Catapult_Launch(int param_1)

{
  *(undefined1 *)(param_1 + 0x10f0) = 1;
  *(undefined4 *)(param_1 + 0x10f4) = 0x32;
  return;
}

