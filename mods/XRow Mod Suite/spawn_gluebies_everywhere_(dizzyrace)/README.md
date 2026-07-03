# "spawn gluebies everywhere (dizzyrace) "

**CT Entry ID:** 47

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+A7A2:
db 90 90 90 90 90 90

 
 
[DISABLE]
Hamsterball.exe+A7A2:
db 0F 85 91 00 00 00

```
