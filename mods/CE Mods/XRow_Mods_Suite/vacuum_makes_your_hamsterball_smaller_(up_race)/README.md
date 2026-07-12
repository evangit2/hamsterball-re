# "vacuum makes your hamsterball smaller (up race)"

**CT Entry ID:** 204

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFE7A:
db 90 90 90 90 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFE7A:
db 56 41 43 50 4F 50 4F 55 54

```
