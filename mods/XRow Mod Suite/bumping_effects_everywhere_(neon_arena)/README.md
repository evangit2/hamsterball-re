# "bumping effects everywhere (neon arena)"

**CT Entry ID:** 288

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+174BC:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+174BC:
db 0F 85 38 01 00 00

```
