# "Disable size-breaking collisions"

**CT Entry ID:** 31

**Script Type:** Code cave / complex

## Script

```
{

This disables the size-breaking function where a giant
ball entity collides with a smaller one, then the small
one will break/shatter on collision with the giant entity

}

[ENABLE]

"Hamsterball.exe"+6FEA:
  jmp "Hamsterball.exe"+7094

"Hamsterball.exe"+70AD:
  jmp "Hamsterball.exe"+7154

[DISABLE]

"Hamsterball.exe"+6FEA:
  db 0F 85 A4 00 00 00

"Hamsterball.exe"+70AD:
  db 0F 8A A1 00 00 00

```
