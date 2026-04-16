---
name: github-commits
description: Git commit and push workflow for hamsterball-re repo and other projects. Handles common push failures and credential issues in sandbox environments.
triggers:
  - commit
  - push to github
  - git push hanging
---

# GitHub Commits & Pushes

## Quick Git Workflow
```bash
cd ~/hamsterball-re  # or project dir
git add -A
git status            # verify staged changes
git commit -m "descriptive message"
git push origin master
```

## Push Troubleshooting

### If `git push` hangs
1. Check remote: `git remote -v`
2. **HTTPS is preferred** — use `https://github.com/...` URLs, NOT `git@github.com:...` SSH
3. If SSH by mistake, fix: `git remote set-url origin https://github.com/USER/REPO.git`
4. If `gh auth git-credential` hangs, override:
   ```bash
   git config --unset credential.helper
   git config --global credential.helper 'store --file=/tmp/git-creds-store'
   # Then push normally
   ```
5. SSH keys often don't work in sandbox — SSH key at `~/.ssh/id_ed25519` exists but isn't registered with GitHub agent

### If `git push` fails with auth error
- The `gh` CLI stores tokens in `~/.config/gh/hosts.yml`
- Use HTTPS with stored credentials — it works reliably
- Never use `ssh://` or `git@github.com:` URLs in sandbox environments

## Commit Message Format
For the hamsterball-re project specifically:
```
Session N: X.X% (YYYY/3811) — brief summary of key renames/findings
```

## After Each Batch of Renames
1. Update `FUNCTION_MAP.md` header stats (documented count, percentage)
2. Append new function entries to `FUNCTION_MAP.md`
3. `git add -A && git commit -m "..." && git push origin master`

## Hamsterball-Specific
- Repo: `~/hamsterball-re/`
- GitHub: `https://github.com/evangit2/hamsterball-re.git`
- Branch: `master`
- Total functions: 3811
- Track progress: `(documented/3811)*100`%
- Docs in: `docs/FUNCTION_MAP.md`, `docs/RESEARCH_LOG.md`

## Common Pitfalls
- Don't use SSH URLs in sandbox — `git@github.com:` consistently fails with "Permission denied (publickey)"
- Don't let `gh auth git-credential` hang the push — it spawns a subprocess that blocks
- Don't forget `git add -A` before committing — Ghidra renames are in the project file, not tracked files
- Always push after committing to keep GitHub in sync