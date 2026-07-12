# "Entities Jump Boosts                                                       [] Info -->"

**CT Entry ID:** 12

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{

Press 1 to jump boost Player 1
Press 2 to jump boost Player 2
Press 3 to jump boost Player 3
Press 4 to jump boost Player 4
Press 5 to jump boost Badballs

Numbers in Numpad wont work

}


[ENABLE]
alloc(newmem, 1000)
alloc(jumpP1, 4)
alloc(jumpP2, 4)
alloc(jumpP3, 4)
alloc(jumpP4, 4)
alloc(jumpBad, 4)
alloc(JumpForce, 4)
registersymbol(jumpP1)
registersymbol(jumpP2)
registersymbol(jumpP3)
registersymbol(jumpP4)
registersymbol(jumpBad)

JumpForce:
  dd (float)20.0   // Jump force / height

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
  cmp dword ptr [jumpP1], 0
  je original
  mov dword ptr [jumpP1], 0
  jmp do_jump

check_p2:
  cmp dword ptr [jumpP2], 0
  je original
  mov dword ptr [jumpP2], 0
  jmp do_jump

check_p3:
  cmp dword ptr [jumpP3], 0
  je original
  mov dword ptr [jumpP3], 0
  jmp do_jump

check_p4:
  cmp dword ptr [jumpP4], 0
  je original
  mov dword ptr [jumpP4], 0
  jmp do_jump

check_bad:
  cmp dword ptr [jumpBad], 0
  je original
  mov dword ptr [jumpBad], 0

do_jump:
  push eax
  mov eax, [esi+0x1A4]
  test eax, eax
  jz skip_jump
  push edx
  mov edx, [JumpForce]
  mov [eax+0xCA8], edx   // Y velocity
  pop edx
skip_jump:
  pop eax

original:
  mov eax, [esi+0x0c5c]
  jmp Hamsterball.exe+5E28

"Hamsterball.exe"+5E22:
  jmp newmem
  nop

[DISABLE]
"Hamsterball.exe"+5E22:
  db 8B 86 5C 0C 00 00
unregistersymbol(jumpP1)
unregistersymbol(jumpP2)
unregistersymbol(jumpP3)
unregistersymbol(jumpP4)
unregistersymbol(jumpBad)
dealloc(newmem)
dealloc(jumpP1)
dealloc(jumpP2)
dealloc(jumpP3)
dealloc(jumpP4)
dealloc(jumpBad)
dealloc(JumpForce)

```
