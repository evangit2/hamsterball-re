# "Global spawn tar signs everywhere "

**CT Entry ID:** 44

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+C2F4:
db 90 90 90 90 90 90

 
 
[DISABLE]
Hamsterball.exe+C2F4:
db 0F 85 E7 00 00 00

```
