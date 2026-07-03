# "Global Controlled 8ball spawn"

**CT Entry ID:** 7

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{ Game   : Hamsterball.exe
  Version:
  Date   : 2026-05-25
  Author : XRow

  BadBall Spawn Limiter - Spawns limited BadBalls
}

[ENABLE]

alloc(newmem,$1000)
alloc(BadBallCount,4) // Not this, scroll down

BadBallCount:
  dd 0

registersymbol(BadBallCount)

label(code)
label(skip)
label(allow)

newmem:
  cmp [BadBallCount],2 // Number of badballs to spawn
  jge skip
  inc [BadBallCount]
  jmp allow

skip:
  jmp Hamsterball.exe+BEFE

allow:

code:
  jmp Hamsterball.exe+BD2D

"Hamsterball.exe"+BD27:
  jmp newmem
  nop

[DISABLE]

"Hamsterball.exe"+BD27:
  db 0F 85 D1 01 00 00

unregistersymbol(BadBallCount)
dealloc(newmem)
dealloc(BadBallCount)

```
