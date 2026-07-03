# "Follow camera"

**CT Entry ID:** 15

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+C761:
db 90 90 90 90 90 90
Hamsterball.exe+F226:
db 90 90 90 90 90 90 90

 
 
[DISABLE]
Hamsterball.exe+C761:
db 88 85 68 07 00 00
Hamsterball.exe+F226:
db C6 86 68 07 00 00 00

```
