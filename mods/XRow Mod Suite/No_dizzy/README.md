# "No dizzy"

**CT Entry ID:** 17

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+75C9:
db 90 90 90 90 90 90
 
 
[DISABLE]
Hamsterball.exe+75C9:
db FF 86 EC 02 00 00

```
