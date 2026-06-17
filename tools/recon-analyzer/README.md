# recon-analyzer

A tiny, program-agnostic first-pass analyzer for compiled binaries.

## Usage

```bash
python3 recon-analyzer.py /path/to/binary
python3 recon-analyzer.py /path/to/binary --out report.md
python3 recon-analyzer.py /path/to/dir --recursive --out report.md
python3 recon-analyzer.py /path/to/binary --format json
```

## What it does

- Hashes the target (MD5, SHA256)
- Computes entropy
- Runs `strings`
- Categorizes strings (paths, DLLs, APIs, file types, interesting keywords)
- Summarizes PE structure (if `pefile` is installed)
- Emits Markdown or JSON report

## Requirements

- Python 3.10+
- `strings` command available
- Optional: `pefile` (`pip install pefile`)

## Example

```bash
python3 recon-analyzer.py ~/hamsterball-re/originals/installed/extracted/Hamsterball.exe --out /tmp/hb_recon.md
```
