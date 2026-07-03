# "grow hamster after shrink (odd race)"

**CT Entry ID:** 30

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+F04A:
db 90 90
Hamsterball.exe+F005:
db 90 90
 
 
[DISABLE]
Hamsterball.exe+F04A:
db 75 07
Hamsterball.exe+F005:
db 75 2A

```
