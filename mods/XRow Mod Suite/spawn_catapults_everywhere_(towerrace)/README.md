# "spawn catapults everywhere (towerrace)"

**CT Entry ID:** 41

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+D7F8:
db 90 90

 
 
[DISABLE]
Hamsterball.exe+D7F8:
db 75 76

```
