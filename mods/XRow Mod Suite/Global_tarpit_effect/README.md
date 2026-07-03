# "Global tarpit effect "

**CT Entry ID:** 43

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
Hamsterball.exe+CC30:
db 90 90

 
 
[DISABLE]
Hamsterball.exe+CC30:
db 75 71

```
