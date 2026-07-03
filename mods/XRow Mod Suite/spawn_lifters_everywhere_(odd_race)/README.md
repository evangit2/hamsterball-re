# "spawn lifters everywhere (odd race)"

**CT Entry ID:** 296

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+EC74:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+EC74:
db 0F 85 87 00 00 00

```
