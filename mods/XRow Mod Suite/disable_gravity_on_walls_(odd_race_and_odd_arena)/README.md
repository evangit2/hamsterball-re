# "disable gravity on walls (odd race and odd arena)"

**CT Entry ID:** 209

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFC12:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFC12:
db 47 52 41 56 49

```
