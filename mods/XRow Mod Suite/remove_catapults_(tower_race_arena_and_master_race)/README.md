# "remove catapults (tower race, arena and master race)"

**CT Entry ID:** 176

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CF99C:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CF99C:
db 43 41 54 41 50

```
