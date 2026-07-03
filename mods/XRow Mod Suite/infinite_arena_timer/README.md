# "infinite arena timer"

**CT Entry ID:** 312

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
[ENABLE]

aobscanmodule(INJECT,Hamsterball.exe,89 8F AC 47 00 00) // should be unique
alloc(newmem,$1000)

label(code)
label(return)

newmem:

code:
  add [edi+000047AC],#1
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
dealloc(newmem)

```
