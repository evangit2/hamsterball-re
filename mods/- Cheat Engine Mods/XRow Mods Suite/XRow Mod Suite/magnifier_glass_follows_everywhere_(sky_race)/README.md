# "magnifier glass follows everywhere (sky race)"

**CT Entry ID:** 33

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+10DC6:
db 90 90
 
[DISABLE]
Hamsterball.exe+10DC6:
db 75 2F

```
