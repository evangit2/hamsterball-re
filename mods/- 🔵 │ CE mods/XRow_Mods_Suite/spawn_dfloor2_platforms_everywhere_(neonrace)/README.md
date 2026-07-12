# "spawn dfloor2 platforms everywhere (neonrace) "

**CT Entry ID:** 37

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+16A4D:
db 90 90

 
 
[DISABLE]
Hamsterball.exe+16A4D:
db 75 6C

```
