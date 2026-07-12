# "spawn saw everywhere (toob race)"

**CT Entry ID:** 272

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+FBEA:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+FBEA:
db 0F 85 9D 00 00 00

```
