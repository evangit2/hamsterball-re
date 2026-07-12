# "Global Controlled Mousetrap spawn"

**CT Entry ID:** 22

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{ Game   : Hamsterball.exe
  Version:
  Date   : 2026-05-27
  Author : XRow

  Mousetrap Spawn Limiter - Spawns limited Mousetraps
}

[ENABLE]

alloc(newmem,$1000)
alloc(MouseCount,4) // Not this, scroll down

MouseCount:
  dd 0

registersymbol(MouseCount)

label(code)
label(skip)
label(allow)

newmem:
  cmp [MouseCount],2 // Number of mousetraps to spawn
  jge skip
  inc [MouseCount]
  jmp allow

skip:
  jmp Hamsterball.exe+C09B // Jump destination of original JNE

allow:

code:
  jmp Hamsterball.exe+BFD8 // Spawn code right after JNE

"Hamsterball.exe"+BFD2:
  jmp newmem
  nop

[DISABLE]

"Hamsterball.exe"+BFD2:
  db 0F 85 C3 00 00 00

unregistersymbol(MouseCount)
dealloc(newmem)
dealloc(MouseCount)

```
