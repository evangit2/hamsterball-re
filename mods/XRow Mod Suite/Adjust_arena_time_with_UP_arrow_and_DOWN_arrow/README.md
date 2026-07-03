# "Adjust arena time with UP arrow and DOWN arrow "

**CT Entry ID:** 50

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

aobscanmodule(INJECT,Hamsterball.exe,89 8F AC 47 00 00)
alloc(newmem,$1000)
alloc(timerAdd,4)
alloc(timerSub,4)
registersymbol(timerAdd)
registersymbol(timerSub)

label(code)
label(return)

timerAdd:
  dd 0
timerSub:
  dd 0

newmem:
  cmp dword ptr [timerAdd], 1
  jne check_sub
  mov dword ptr [timerAdd], 0
  add [edi+000047AC], #1000

check_sub:
  cmp dword ptr [timerSub], 1
  jne code
  mov dword ptr [timerSub], 0
  sub [edi+000047AC], #1000

code:
  sub [edi+000047AC],#1
  jmp return

INJECT:
  jmp newmem
  nop
return:
registersymbol(INJECT)

[DISABLE]

INJECT:
  db 89 8F AC 47 00 00

unregistersymbol(INJECT)
unregistersymbol(timerAdd)
unregistersymbol(timerSub)
dealloc(newmem)
dealloc(timerAdd)
dealloc(timerSub)

```
