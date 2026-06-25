# hbremap — Hamsterball RE Knowledge Map

Interactive mind map of all Hamsterball reverse engineering documentation, mods, and tools.

Built with [Cosma](https://cosma.arthurperret.fr/) — an open-source document graph visualization tool.

## Live Site

**https://evangit2.github.io/hamsterball-re/hbremap/**

## Features

- **Graph visualization** — 183 documents rendered as an interactive network of color-coded nodes
- **Full-text search** — Press `Ctrl+K` (or click the 🔍 button) to search across all documents
  - Searches titles, tags, hex addresses, function names, and full body text
  - Results show highlighted snippets with context
  - Each result links directly to the source file on GitHub
- **Document cards** — Click any node to view the full markdown content rendered as an index card
- **Type filtering** — Filter the graph by document category (objects, physics, mods, tools, etc.)
- **GitHub source links** — Every document has a "View source on GitHub" link at the top
- **Wikilinks** — Cross-references between documents are clickable links in the graph

## Document Categories

| Type | Color | Count | Description |
|------|-------|-------|-------------|
| docs | Blue | 11 | Top-level system docs |
| objects | Orange | 17 | Game struct/object analysis |
| physics | Green | 13 | Physics & collision system |
| modding | Purple | 7 | Modding guides & references |
| rendering | Red | 7 | D3D8 rendering pipeline |
| gameplay | Yellow | 7 | Game state & scoring |
| meshworld | Teal | 7 | MESHWORLD binary format |
| audio | Gray | 2 | Audio system docs |
| input | Brown | 2 | Input/control system |
| decompilation | Lavender | 8 | Key decompilation analysis |
| mods | Violet | 35 | DLL mod documentation |
| tools | Dark Green | 11 | Tool documentation |
| analysis | Pink | 5 | Analysis & verification |
| playbook | Tan | 16 | RE playbook chapters |
| agent-knowledge | Sage | 17 | Agent knowledge pack |
| project | Brown | 12 | Project meta docs |
| ui | Pink | 3 | UI/menu system docs |
| skill | Sky Blue | 1 | Main RE skill |
| plans | Gray | 1 | Reimpl plans |
| root | Black | 1 | Project README |

## How to Rebuild

```bash
npm install @graphlab-fr/cosma --global
python3 hbremap/build_cosmoscope.py
```

The build script:
1. Collects all `.md` files from `docs/`, `mods/`, `tools/`, `analysis/`, `skills/gaming/hamsterball-re/`, and `README.md`
2. Adds YAML frontmatter (id, title, type, tags) to each file
3. Extracts hex addresses and function names as tags
4. Inserts wikilinks between cross-referencing documents
5. Adds GitHub source links to each document
6. Runs `cosma modelize` to generate the cosmoscope
7. Injects a custom full-text search overlay into the HTML

## Auto-Deploy

GitHub Actions workflow (`.github/workflows/hbremap-pages.yml`) automatically rebuilds and deploys on every push to `main` that touches any documentation file.
