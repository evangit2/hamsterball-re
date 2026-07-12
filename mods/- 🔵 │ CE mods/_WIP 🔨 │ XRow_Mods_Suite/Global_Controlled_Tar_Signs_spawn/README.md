# "Global Controlled Tar Signs spawn"

**CT Entry ID:** 25

**Script Type:** Code cave / complex

**Uses alloc:** Yes

**Uses registersymbol:** Yes

## Script

```
{ Game   : Hamsterball.exe
  Version:
  Date   : 2026-05-27
  Author : XRow

  Tar Sign Spawn Limiter - Spawns limited Tar Signs
}

[ENABLE]

alloc(newmem,$1000)
alloc(TarCount,4) // Not this, scroll down

TarCount:
  dd 0

registersymbol(TarCount)

label(code)
label(skip)
label(allow)

newmem:
  cmp [TarCount],3 // Number of tar signs to spawn
  jge skip
  inc [TarCount]
  jmp allow

skip:
  jmp Hamsterball.exe+C3E1 // Jump destination of original JNE

allow:

code:
  jmp Hamsterball.exe+C2FA // Spawn code right after JNE

"Hamsterball.exe"+C2F4:
  jmp newmem
  nop

[DISABLE]

"Hamsterball.exe"+C2F4:
  db 0F 85 E7 00 00 00

unregistersymbol(TarCount)
dealloc(newmem)
dealloc(TarCount)

```
