# "remove blade brothers (expert race)"

**CT Entry ID:** 167

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFA82:
db 90 90 90 90 90
Hamsterball.exe+CFA92:
db 90 90 90 90 90
Hamsterball.exe+CFAA2:
db 90 90 90 90 90
Hamsterball.exe+CFAAE:
db 90 90 90 90 90

[DISABLE]
Hamsterball.exe+CFA82:
db 41 43 54 49 56
Hamsterball.exe+CFA92:
db 41 43 54 49 56
Hamsterball.exe+CFAA2:
db 41 4C 45 52 54
Hamsterball.exe+CFAAE:
db 41 4C 45 52 54

```
