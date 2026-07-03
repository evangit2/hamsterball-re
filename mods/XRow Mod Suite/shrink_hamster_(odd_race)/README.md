# "shrink hamster (odd race)"

**CT Entry ID:** 29

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+EF7C:
db 90 90
 
 
[DISABLE]
Hamsterball.exe+EF7C:
db 75 6E

```
