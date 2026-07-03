# "Weaker mousetraps (x0 speed)"

**CT Entry ID:** 115

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF370:
db 00 00 A0 90 DF CC 4C


[DISABLE]
Hamsterball.exe+CF370:
db 00 00 A0 41 CD CC 4C

```
