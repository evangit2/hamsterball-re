# "Manually shatter/break ball entities                             [] Info -->"

**CT Entry ID:** 42

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{

Press 1 to shatter Player 1
Press 2 to shatter Player 2
Press 3 to shatter Player 3
Press 4 to shatter Player 4
Press 5 to shatter Badballs

Numbers in Numpad wont work

}


[ENABLE]
alloc(newmem, 1000)
alloc(splitP1, 4)
alloc(splitP2, 4)
alloc(splitP3, 4)
alloc(splitP4, 4)
alloc(splitBad, 4)
registersymbol(splitP1)
registersymbol(splitP2)
registersymbol(splitP3)
registersymbol(splitP4)
registersymbol(splitBad)

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
  cmp dword ptr [splitP1], 0
  je original
  mov dword ptr [splitP1], 0
  jmp do_split

check_p2:
  cmp dword ptr [splitP2], 0
  je original
  mov dword ptr [splitP2], 0
  jmp do_split

check_p3:
  cmp dword ptr [splitP3], 0
  je original
  mov dword ptr [splitP3], 0
  jmp do_split

check_p4:
  cmp dword ptr [splitP4], 0
  je original
  mov dword ptr [splitP4], 0
  jmp do_split

check_bad:
  cmp dword ptr [splitBad], 0
  je original
  mov dword ptr [splitBad], 0

do_split:
  push ecx
  mov ecx, esi
  call 00409480
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
unregistersymbol(splitP1)
unregistersymbol(splitP2)
unregistersymbol(splitP3)
unregistersymbol(splitP4)
unregistersymbol(splitBad)
dealloc(newmem)
dealloc(splitP1)
dealloc(splitP2)
dealloc(splitP3)
dealloc(splitP4)
dealloc(splitBad)

```
