# "Motionless 8balls"

**CT Entry ID:** 334

**Script Type:** Simple byte patches

## Script

```
[ENABLE]
// 8598: 0F 84 96 00 00 00 -> 0F 84 00 00 00 00 (je +0)
Hamsterball.exe+859A:
  db 00 00 00 00
// 85A6: 0F 85 88 00 00 00 -> 0F 85 00 00 00 00 (jne +0)
Hamsterball.exe+85A8:
  db 00 00 00 00
// 85B4: 75 7E -> 75 00 (jne +0)
Hamsterball.exe+85B5:
  db 00
// 85BE: 75 74 -> 75 00 (jne +0)
Hamsterball.exe+85BF:
  db 00
// 85CB: 74 67 -> 74 00 (je +0)
Hamsterball.exe+85CC:
  db 00
// 85D1: 74 61 -> 74 00 (je +0)
Hamsterball.exe+85D2:
  db 00

[DISABLE]
Hamsterball.exe+859A:
  db 96 00 00 00
Hamsterball.exe+85A8:
  db 88 00 00 00
Hamsterball.exe+85B5:
  db 7E
Hamsterball.exe+85BF:
  db 74
Hamsterball.exe+85CC:
  db 67
Hamsterball.exe+85D2:
  db 61

```
