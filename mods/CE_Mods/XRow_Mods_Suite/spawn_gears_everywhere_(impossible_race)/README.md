# "spawn gears everywhere (impossible race)"

**CT Entry ID:** 265

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+1809A:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+1809A:
db 0F 85 8C 00 00 00

```
