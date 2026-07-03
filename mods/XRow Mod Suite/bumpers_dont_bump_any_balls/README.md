# "bumpers don't bump any balls"

**CT Entry ID:** 211

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFD36:
db 90 90


[DISABLE]
Hamsterball.exe+CFD36:
db 42 55

```
