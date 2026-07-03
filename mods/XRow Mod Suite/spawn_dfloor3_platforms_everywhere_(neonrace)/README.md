# "spawn dfloor3 platforms everywhere (neonrace) "

**CT Entry ID:** 38

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+16AD1:
db 90 90

 
 
[DISABLE]
Hamsterball.exe+16AD1:
db 75 70

```
