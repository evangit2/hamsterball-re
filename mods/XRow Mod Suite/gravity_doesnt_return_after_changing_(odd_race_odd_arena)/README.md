# "gravity doesnt return after changing (odd race, odd arena)"

**CT Entry ID:** 228

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CFC00:
db 90 90 90 90 90


[DISABLE]
Hamsterball.exe+CFC00:
db 4E 4F 52 4D 41

```
