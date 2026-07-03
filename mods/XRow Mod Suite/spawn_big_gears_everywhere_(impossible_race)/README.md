# "spawn big gears everywhere (impossible race)"

**CT Entry ID:** 264

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+18142:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+18142:
db 0F 85 B3 00 00 00

```
