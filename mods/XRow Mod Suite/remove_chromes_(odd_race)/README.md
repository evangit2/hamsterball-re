# "remove chromes (odd race)"

**CT Entry ID:** 185

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFAE8:
db 90 90 90 90 90
Hamsterball.exe+CFAF4:
db 90 90 90 90 90
Hamsterball.exe+CFB00:
db 90 90 90 90 90

[DISABLE]
Hamsterball.exe+CFAE8:
db 4C 41 55 4E 43
Hamsterball.exe+CFAF4:
db 4C 41 55 4E 43
Hamsterball.exe+CFB00:
db 4C 41 55 4E 43

```
