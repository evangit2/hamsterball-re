# "de-activate catapults (tower race, tower arena and master race)"

**CT Entry ID:** 197

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF9E6:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF9E6:
db 43 41 54 41 50

```
