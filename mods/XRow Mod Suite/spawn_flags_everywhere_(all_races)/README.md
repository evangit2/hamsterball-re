# "spawn flags everywhere (all races)"

**CT Entry ID:** 261

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+C18A:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+C18A:
db 0F 85 93 00 00 00

```
