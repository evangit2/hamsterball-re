# "Teleport Entities in the middle of Arenas                      [] Info -->"

**CT Entry ID:** 347

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{

Press T to teleport Players 1-4
Press Y to teleport Badballs

}


[ENABLE]
alloc(newmem, 1000)
alloc(tpPlayer1, 4)
alloc(tpPlayer2, 4)
alloc(tpPlayer3, 4)
alloc(tpPlayer4, 4)
alloc(tpBadBall, 4)
registersymbol(tpPlayer1)
registersymbol(tpPlayer2)
registersymbol(tpPlayer3)
registersymbol(tpPlayer4)
registersymbol(tpBadBall)

newmem:
  mov eax, [esi+0x18]

  cmp eax, 0
  je check_p1
  cmp eax, 1
  je check_p2
  cmp eax, 2
  je check_p3
  cmp eax, 3
  je check_p4
  cmp eax, -1
  je check_bad
  jmp original

check_p1:
  cmp dword ptr [tpPlayer1], 0
  je original
  mov dword ptr [tpPlayer1], 0
  jmp do_teleport

check_p2:
  cmp dword ptr [tpPlayer2], 0
  je original
  mov dword ptr [tpPlayer2], 0
  jmp do_teleport

check_p3:
  cmp dword ptr [tpPlayer3], 0
  je original
  mov dword ptr [tpPlayer3], 0
  jmp do_teleport

check_p4:
  cmp dword ptr [tpPlayer4], 0
  je original
  mov dword ptr [tpPlayer4], 0
  jmp do_teleport

check_bad:
  cmp dword ptr [tpBadBall], 0
  je original
  mov dword ptr [tpBadBall], 0

do_teleport:
  mov dword ptr [esi+0x164], 0
  mov dword ptr [esi+0x168], 0x41A00000
  mov dword ptr [esi+0x16C], 0
  mov dword ptr [esi+0x170], 0
  mov dword ptr [esi+0x174], 0
  mov dword ptr [esi+0x178], 0

  // Zero physics body velocity to prevent break on teleport
  push eax
  mov eax, [esi+0x1A4]
  test eax, eax
  jz skip_phys
  mov dword ptr [eax+0xCA4], 0
  mov dword ptr [eax+0xCA8], 0
  mov dword ptr [eax+0xCAC], 0
skip_phys:
  pop eax

  // Clear break-related flags
  mov byte ptr [esi+0x2E8], 0
  mov byte ptr [esi+0x2E9], 0
  mov dword ptr [esi+0x2EC], 0
  mov byte ptr [esi+0xBA], 0
  mov dword ptr [esi+0x300], 0

original:
  mov eax, [esi+0x0c5c]
  jmp Hamsterball.exe+5E28

"Hamsterball.exe"+5E22:
  jmp newmem
  nop

[DISABLE]
"Hamsterball.exe"+5E22:
  db 8B 86 5C 0C 00 00
unregistersymbol(tpPlayer1)
unregistersymbol(tpPlayer2)
unregistersymbol(tpPlayer3)
unregistersymbol(tpPlayer4)
unregistersymbol(tpBadBall)
dealloc(newmem)
dealloc(tpPlayer1)
dealloc(tpPlayer2)
dealloc(tpPlayer3)
dealloc(tpPlayer4)
dealloc(tpBadBall)

```
