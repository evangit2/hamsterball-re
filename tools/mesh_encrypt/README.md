# MESHWORLD/MESH XOR Encryption Mod

Obfuscates `.MESH` and `.MESHWORLD` files so they can't be read as plain text in a hex editor.

## How it works

1. **Encrypt tool** (`encrypt_mesh.py`): XORs every byte of all `.mesh`/`.meshworld` files with key `119` (0x77)
2. **bass.dll proxy mod** (`mesh_xor_decrypt.c`): Hooks `CreateFileA` + `ReadFile` via IAT to decrypt on the fly

The game sees unencrypted data — the decryption is transparent.

## Key: 119 (0x77)

## Files

| File | Description |
|------|-------------|
| `encrypt_mesh.py` | Python script to XOR-encrypt all mesh/meshworld files in a directory |
| `mesh_xor_decrypt.c` | C source for bass.dll proxy that decrypts at runtime |
| `bass.dll` | Compiled proxy (MinGW cross-compiled) |

## Usage

### Step 1: Encrypt game files

```bash
python3 encrypt_mesh.py /path/to/game/Levels/
python3 encrypt_mesh.py /path/to/game/Meshes/
```

This modifies files **in place**. Back up your originals first!

### Step 2: Install the proxy DLL

1. Rename your original `bass.dll` to `bass_real.dll`
2. Copy the modded `bass.dll` into the game folder

### Build from source

```bash
i686-w64-mingw32-gcc -shared -o bass.dll mesh_xor_decrypt.c \
  -lwinmm -Wl,--enable-stdcall-fixup -O2 -static -static-libgcc \
  -Wl,--add-stdcall-alias
```

## How the DLL mod works

1. In `DllMain` (DLL_PROCESS_ATTACH): patches the game's IAT to intercept `CreateFileA`, `ReadFile`, and `CloseHandle`
2. `CreateFileA` hook: checks if filename ends in `.mesh` or `.meshworld` — if so, tracks the file handle
3. `ReadFile` hook: after reading, XOR-decrypts the buffer if the handle is tracked
4. `CloseHandle` hook: removes handle from tracking table

## Security note

The XOR key (119) is stored as a constant in the DLL binary. Anyone who opens the DLL in a disassembler can find it. This is obfuscation, not encryption — it stops casual browsing of mesh files, not determined reverse engineering.
