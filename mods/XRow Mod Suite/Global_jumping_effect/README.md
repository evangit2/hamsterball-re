# "Global jumping effect"

**CT Entry ID:** 277

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+C8AE:
db 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+C8AE:
db 0F 85 81 00 00 00

```
