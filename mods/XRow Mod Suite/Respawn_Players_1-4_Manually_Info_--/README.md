# "Respawn Players 1-4 Manually                                      [] Info -->"

**CT Entry ID:** 355

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{

Press 1 to break Player 1
Press 2 to break Player 2
Press 3 to break Player 3
Press 4 to break Player 4

Numbers in Numpad wont work

}


[ENABLE]
alloc(newmem, 1000)
alloc(breakP1, 4)
alloc(breakP2, 4)
alloc(breakP3, 4)
alloc(breakP4, 4)
registersymbol(breakP1)
registersymbol(breakP2)
registersymbol(breakP3)
registersymbol(breakP4)

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
  jmp original

check_p1:
  cmp dword ptr [breakP1], 0
  je original
  mov dword ptr [breakP1], 0
  jmp do_break

check_p2:
  cmp dword ptr [breakP2], 0
  je original
  mov dword ptr [breakP2], 0
  jmp do_break

check_p3:
  cmp dword ptr [breakP3], 0
  je original
  mov dword ptr [breakP3], 0
  jmp do_break

check_p4:
  cmp dword ptr [breakP4], 0
  je original
  mov dword ptr [breakP4], 0

do_break:
  push ecx
  mov ecx, esi
  call 00405190
  pop ecx

original:
  mov eax, [esi+0x0c5c]
  jmp Hamsterball.exe+5E28

"Hamsterball.exe"+5E22:
  jmp newmem
  nop

[DISABLE]
"Hamsterball.exe"+5E22:
  db 8B 86 5C 0C 00 00
unregistersymbol(breakP1)
unregistersymbol(breakP2)
unregistersymbol(breakP3)
unregistersymbol(breakP4)
dealloc(newmem)
dealloc(breakP1)
dealloc(breakP2)
dealloc(breakP3)
dealloc(breakP4)

```
