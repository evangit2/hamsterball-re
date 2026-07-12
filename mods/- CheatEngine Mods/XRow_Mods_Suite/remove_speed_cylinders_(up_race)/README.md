# "remove speed cylinders (up race)"

**CT Entry ID:** 164

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFE4C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFE4C:
db 53 50 45 45 44

```
