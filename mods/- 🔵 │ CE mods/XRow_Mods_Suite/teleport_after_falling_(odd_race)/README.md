# "teleport after falling (odd race)"

**CT Entry ID:** 270

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+EE79:
db 90 90


[DISABLE]
Hamsterball.exe+EE79:
db 75 67

```
