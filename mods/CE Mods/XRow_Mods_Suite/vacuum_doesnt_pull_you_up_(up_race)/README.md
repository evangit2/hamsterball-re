# "vacuum doesn't pull you up (up race)"

**CT Entry ID:** 231

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFE28:
db 90 90 90 90


[DISABLE]
Hamsterball.exe+CFE28:
db 56 41 43 2D

```
