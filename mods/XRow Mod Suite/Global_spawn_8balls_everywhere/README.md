# "Global spawn 8balls everywhere "

**CT Entry ID:** 46

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+BD27:
db 90 90 90 90 90 90

 
 
[DISABLE]
Hamsterball.exe+BD27:
db 0F 85 D1 01 00 00

```
