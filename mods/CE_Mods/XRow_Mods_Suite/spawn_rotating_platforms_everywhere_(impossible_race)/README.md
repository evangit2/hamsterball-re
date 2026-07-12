# "spawn rotating platforms everywhere (impossible race)"

**CT Entry ID:** 262

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+18211:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+18211:
db 0F 85 92 00 00 00

```
