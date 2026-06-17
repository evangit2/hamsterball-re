# 01 - Bootstrap a Hamsterball RE Project

This guide assumes you have access to the `hamsterball-re` repo and the original game assets.

## 1. Workspace Layout

```
hamsterball-re/
├── originals/              # NEVER modify these
│   ├── installer/
│   └── installed/
│       └── extracted/
│           └── Hamsterball.exe
├── analysis/
│   ├── ghidra/
│   │   ├── HamsterballProject/
│   │   ├── scripts/
│   │   ├── structs/
│   │   └── renames_backup.json
│   ├── code_ref/
│   └── screenshots/
├── docs/                   # Human-readable findings
├── reimpl/                 # Reimplementation source
└── tools/                  # Custom RE tooling
```

## 2. Hash Everything First

Before analysis, record integrity baselines:

```bash
cd ~/hamsterball-re
python3 - <<'PY'
import hashlib, json, os
root = "originals/installed/extracted"
hashes = {}
for dirpath, _, files in os.walk(root):
    for f in files:
        p = os.path.join(dirpath, f)
        with open(p, "rb") as fh:
            data = fh.read()
            hashes[os.path.relpath(p, root)] = {
                "md5": hashlib.md5(data).hexdigest(),
                "sha256": hashlib.sha256(data).hexdigest(),
                "size": len(data)
            }
with open("docs/FILE_HASHES.json", "w") as out:
    json.dump(hashes, out, indent=2)
print("hashed", len(hashes), "files")
PY
```

Known good MD5 for `Hamsterball.exe`: `7d25019366b8d7f55906325bd630d7fe`.

## 3. Initial Binary Reconnaissance

```bash
# File type and PE layout
file Hamsterball.exe
objdump -x Hamsterball.exe | head -80

# String sweeps
strings Hamsterball.exe | grep -iE '\.(dll|mesh|world|xml|wav|png|bmp|mo3)'
strings Hamsterball.exe | grep -iE 'direct3d|d3d|dinput|dsound|bass'
strings Hamsterball.exe | grep -iE 'menu|level|race|score|hamster'
```

Key findings to expect:
- PE32 i386, Visual C++ 2003 era
- Imports from `D3D8.dll`, `DINPUT8.dll`, `DSOUND.dll`, `BASS.dll`
- Window class name `AthenaWindow` at ~0x4D9374
- Engine assets under `levels\`, `data\`, `fonts\`

## 4. PE Analysis With Python

```bash
python3 - <<'PY'
import pefile
pe = pefile.PE("originals/installed/extracted/Hamsterball.exe")
print(f"Image base: 0x{pe.OPTIONAL_HEADER.ImageBase:08X}")
print(f"Entry point RVA: 0x{pe.OPTIONAL_HEADER.AddressOfEntryPoint:08X}")
print("Imports:")
for entry in pe.DIRECTORY_ENTRY_IMPORT:
    print(f"  {entry.dll.decode()}: {len(entry.imports)} imports")
PY
```

## 5. Asset Discovery

Common asset directories and files:

```
levels\          # .MESH and .MESHWORLD level files
data\            # HS.CFG save, RaceData.xml, Jukebox.xml, etc.
fonts\           # .description + .png font data
meshes\          # ball, hamster, hazards
sounds\          # .wav SFX
music\           # .mo3 tracker music (BASS)
```

## 6. First Wine Run (Optional)

Never background Wine. Instead:

```bash
cd originals/installed/extracted
WINEDEBUG=+loaddll timeout 30 wine Hamsterball.exe 2>&1 | tee /tmp/wine_first_run.log
```

Look for missing DLLs in the log.
