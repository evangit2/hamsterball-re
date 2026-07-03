# "deactivate bonus-time button (up race)"

**CT Entry ID:** 229

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFE40:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFE40:
db 54 49 4D 45 42

```
