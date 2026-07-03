# "spawn tipper floors everywhere (dizzy race)"

**CT Entry ID:** 275

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+A629:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+A629:
db 0F 85 D1 00 00 00

```
