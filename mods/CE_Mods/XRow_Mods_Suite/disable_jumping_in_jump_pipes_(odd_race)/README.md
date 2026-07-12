# "disable jumping in jump pipes (odd race)"

**CT Entry ID:** 226

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFBDA:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFBDA:
db 4A 55 4D 50 53

```
