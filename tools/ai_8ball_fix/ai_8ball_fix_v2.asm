[ENABLE]
// ai_8ball_fix_v2.ct - Cheat Engine assembly script
// Fixes motionless grounded 8balls in Hamsterball
// 
// ROOT CAUSE: Ball_AI_ChaseNearest (vtable[4], 0x408390) has Gate 0 at
// 0x4083AE: if ball+0xC74==0 AND App+0x237==0, return immediately.
// Practice mode: App+0x237=0. Fresh CreateBadBall'd 8balls have ball+0xC74=0.
// Result: Ball_Update + targeting + SetVelocity ALL SKIPPED → motionless.
//
// FIX: NOP the 6-byte JZ at 0x4083AE (0F 84 76 04 00 00 → 90×6)

aobscanmodule(aob_ball_ai_gate, Hamsterball.exe, 0F 84 76 04 00 00)
registersymbol(aob_ball_ai_gate)

aob_ball_ai_gate:
  nop
  nop
  nop
  nop
  nop
  nop

[DISABLE]
// Restore: JZ 0x40882A (0F 84 76 04 00 00)
aob_ball_ai_gate:
  db 0F 84 76 04 00 00

unregistersymbol(aob_ball_ai_gate)
