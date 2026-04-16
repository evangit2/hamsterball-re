---
name: hamsterball-git-sync
description: Dual-repo sync for hamsterball-re - public (no binaries) and private (with binaries). Push workflow, binary purging, and remote management.
---

# Hamsterball RE Git Sync Skill

## Dual-Repo Setup

- **Public repo** (`origin`): `https://github.com/evangit2/hamsterball-re.git` — NO copyrighted binaries (exe, dll, zip, installer)
- **Private repo** (`priv`): `https://github.com/evangit2/hamsterball-re-priv.git` — EVERYTHING including binaries and builds

## .gitignore (public repo)

The `.gitignore` in the public repo MUST always exclude:
- `originals/installed/extracted/Hamsterball.exe`
- `originals/installed/extracted/bass.dll`
- `originals/installed/extracted/unins000.exe`
- `originals/installed/extracted/unins000.dat`
- `originals/installed/extracted/Music/Music.mo3`
- `originals/installed/Hamsterball.zip`
- `originals/installer/`
- `original/hamsterball.zip`
- `releases/` (built exe)
- Ghidra project files
- Build dirs, SDK downloads

Game ASSETS (levels, textures, sounds, meshes, XML, fonts) are OK in the public repo.

## Pushing Workflow

### Normal push (code/docs changes only):
```bash
cd ~/hamsterball-re
git add -A
git commit -m "descriptive message"
git push origin master
git push priv master
```

### Pushing to private with binaries included:
```bash
cd ~/hamsterball-re
# Force-add the gitignored binaries temporarily
git add -f originals/installed/extracted/Hamsterball.exe \
         originals/installed/extracted/bass.dll \
         originals/installed/extracted/unins000.exe \
         originals/installed/extracted/unins000.dat \
         originals/installed/extracted/Music/Music.mo3 \
         originals/installed/Hamsterball.zip \
         originals/installer/setup_hamsterball.exe \
         originals/installer/share_download.zip \
         original/hamsterball.zip \
         releases/
git commit -m "private: add binaries/builds"
# Push to PRIVATE ONLY
git push priv master
# Now RESET master back to public-only commit (without binaries)
git reset --hard HEAD~1
# Verify public remote is clean
git push origin master  # should be up-to-date
```

### After building Windows exe:
```bash
cd ~/hamsterball-re/reimpl
bash build/build_win64.sh
# Output: build-win64/hamsterball.exe (~15MB static PE32+)

# For private repo, force-add the built exe:
git add -f reimpl/build-win64/hamsterball.exe
git commit -m "private: Windows exe build"
git push priv master
git reset --hard HEAD~1
```

The `reimpl/build-win64/` directory should also contain game asset subdirs
(Levels/, Textures/, Sounds/, Meshes/, Fonts/, Data/) copied from
`originals/installed/extracted/` for Wine testing.

## Purging Copyrighted Files from Public History

If copyrighted binaries ever end up in public repo history:
```bash
pip3 install --user --break-system-packages git-filter-repo
export PATH=$PATH:~/.local/bin

# Create purge_paths.txt with one path per line:
# originals/installed/extracted/Hamsterball.exe
# originals/installed/extracted/bass.dll
# ... etc

git filter-repo --invert-paths --paths-from-file purge_paths.txt --force
# This rewrites ALL history - force push required:
git push --force origin master
```

## Remotes

```
origin  https://github.com/evangit2/hamsterball-re.git (PUBLIC)
priv    https://github.com/evangit2/hamsterball-re-priv.git (PRIVATE)
```

## Key Rules

1. NEVER push copyrighted binaries (exe, dll, zip, installer) to `origin` (public)
2. ALWAYS push to BOTH `origin` AND `priv` for normal code/docs
3. For binary pushes: commit → push to `priv` → `git reset --hard HEAD~1` → continue
4. After `git filter-repo`, remotes are removed — re-add with `git remote add`
5. Branch is `master` (not `main`) — filter-repo may reset it