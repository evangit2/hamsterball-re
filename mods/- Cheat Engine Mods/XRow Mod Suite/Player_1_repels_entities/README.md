# "Player 1 repels entities"

**CT Entry ID:** 408

**Script Type:** Code cave / complex

**Uses alloc:** Yes

## Script

```
[ENABLE]
alloc(newmem, 1000)
alloc(player1X, 4)
alloc(player1Z, 4)

player1X:
  dd (float)0.0
player1Z:
  dd (float)0.0

newmem:
  push eax
  push ecx

  mov eax, [esi+0x18]
  cmp eax, 0
  je save_p1_pos
  cmp eax, 1
  je push_from_p1
  cmp eax, 2
  je push_from_p1
  cmp eax, 3
  je push_from_p1
  cmp eax, -1
  je push_from_p1
  jmp done

save_p1_pos:
  mov eax, [esi+0x164]
  mov [player1X], eax
  mov eax, [esi+0x16C]
  mov [player1Z], eax
  jmp done

push_from_p1:
  mov eax, [esi+0x1A4]
  test eax, eax
  jz done

  // Direction away from player 1
  fld dword ptr [esi+0x164]
  fsub dword ptr [player1X]
  fmul dword ptr [repelForce]
  fadd dword ptr [eax+0xCA4]
  fstp dword ptr [eax+0xCA4]

  fld dword ptr [esi+0x16C]
  fsub dword ptr [player1Z]
  fmul dword ptr [repelForce]
  fadd dword ptr [eax+0xCAC]
  fstp dword ptr [eax+0xCAC]

done:
  pop ecx
  pop eax
  mov [esi+0x168], eax
  jmp 00407C72

repelForce:
  dd (float)0.0007

00407C6C:
  jmp newmem
  nop

[DISABLE]
00407C6C:
db 89 86 68 01 00 00
dealloc(newmem)
dealloc(player1X)
dealloc(player1Z)

```
