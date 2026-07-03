# "disable shrinking in pipe (odd race)"

**CT Entry ID:** 222

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFBC0:
db 90 90 90


[DISABLE]
Hamsterball.exe+CFBC0:
db 45 3A 53

```
