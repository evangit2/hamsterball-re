#!/usr/bin/env python3
"""
Build a Cosma cosmoscope from all Hamsterball RE project markdown files.

Pipeline:
1. Copy all project .md files to hbremap/content/
2. Add YAML frontmatter (id, title, type, tags) to each file
3. Insert [[wikilinks]] between cross-referencing documents
4. Run cosma modelize to generate the cosmoscope HTML
"""

import os
import re
import shutil
import subprocess
import hashlib
import json
from pathlib import Path
from collections import defaultdict

REPO_ROOT = Path(__file__).parent.parent
CONTENT_DIR = Path(__file__).parent / "content"
CONFIG_PATH = Path(__file__).parent / "config.yml"
OUTPUT_HTML = Path(__file__).parent / "index.html"

# Directories to include (relative to repo root)
INCLUDE_DIRS = [
    "docs",
    "mods",
    "tools",
    "analysis",
    "skills/gaming/hamsterball-re",
]
INCLUDE_FILES = ["README.md"]

# Directories to exclude within included dirs
EXCLUDE_PATTERNS = [
    "mingw-libs",
    "node_modules",
    ".venv",
    ".git",
    "raptisoft-exporter/node_modules",
]

# Record type → color mapping
RECORD_TYPES = {
    "docs":           {"fill": "#4a9eff", "stroke": "#1a5cb8"},
    "objects":        {"fill": "#f5a623", "stroke": "#c47d00"},
    "physics":        {"fill": "#7ed321", "stroke": "#4a8a0c"},
    "modding":        {"fill": "#bd10e0", "stroke": "#8a0a9e"},
    "rendering":      {"fill": "#d0021b", "stroke": "#9a0014"},
    "gameplay":       {"fill": "#f8e71c", "stroke": "#c4b80c"},
    "meshworld":      {"fill": "#50e3c2", "stroke": "#2da88c"},
    "audio":          {"fill": "#9b9b9b", "stroke": "#6b6b6b"},
    "input":          {"fill": "#b8651b", "stroke": "#8a4910"},
    "mods":           {"fill": "#9013fe", "stroke": "#6a00d4"},
    "tools":          {"fill": "#417505", "stroke": "#2a4d03"},
    "analysis":       {"fill": "#e85d75", "stroke": "#b03d50"},
    "skill":          {"fill": "#5ac8fa", "stroke": "#3498db"},
    "project":        {"fill": "#8b572a", "stroke": "#5e3b1c"},
    "decompilation":  {"fill": "#c7b1e3", "stroke": "#9a7fc4"},
    "ui":             {"fill": "#f0a0d0", "stroke": "#c47da8"},
    "playbook":       {"fill": "#d4a76a", "stroke": "#a07a44"},
    "agent-knowledge": {"fill": "#6c9f7e", "stroke": "#42705a"},
    "plans":          {"fill": "#a0a0a0", "stroke": "#707070"},
    "root":           {"fill": "#000000", "stroke": "#333333"},
}

def get_type_for_file(filepath: str) -> str:
    """Determine record type based on file path."""
    parts = filepath.lower().split("/")
    if "physics" in filepath: return "physics"
    if "objects" in filepath: return "objects"
    if "meshworld" in filepath: return "meshworld"
    if "modding" in filepath: return "modding"
    if "rendering" in filepath: return "rendering"
    if "gameplay" in filepath: return "gameplay"
    if "audio" in filepath: return "audio"
    if "input" in filepath: return "input"
    if "decompilation" in filepath: return "decompilation"
    if "reverse-engineering-playbook" in filepath: return "playbook"
    if "agent-knowledge" in filepath: return "agent-knowledge"
    if "ui" in filepath: return "ui"
    if "project" in filepath: return "project"
    if "plans" in filepath: return "plans"
    if filepath.startswith("mods/"): return "mods"
    if filepath.startswith("tools/"): return "tools"
    if filepath.startswith("analysis/"): return "analysis"
    if "skill" in filepath.lower(): return "skill"
    if filepath == "README.md": return "root"
    return "docs"

def shorten_title(title: str) -> str:
    """Intelligently shorten a title for graph display."""
    # Remove common prefixes
    for prefix in ["Hamsterball — ", "Hamsterball - ", "Hamsterball ", "The "]:
        if title.startswith(prefix):
            title = title[len(prefix):]
            break
    
    # Cut at subtitle separators — take the main part
    for sep in [" — ", " - ", ": "]:
        if sep in title:
            main = title.split(sep)[0].strip()
            # Only use the shorter part if it's still meaningful
            if len(main) >= 5:
                title = main
                break
    
    # Remove common suffixes
    for suffix in [" Complete Reverse Engineering", " Complete Modder's Reference",
                   " Comprehensive Analysis", " Design Document", " — Complete RE",
                   " (Expert + Tower Races)", " (Expert Race)", " (Session 2928)"]:
        if title.endswith(suffix):
            title = title[:-len(suffix)]
            break
    
    # If still too long, cap at 30 chars
    if len(title) > 30:
        title = title[:30]
    
    return title.strip()

def extract_title(filepath: str, content: str) -> str:
    """Extract title from first H1 heading, or fall back to filename."""
    # Try H1
    match = re.search(r'^#\s+(.+)$', content, re.MULTILINE)
    if match:
        return shorten_title(match.group(1).strip())
    # Try filename without extension
    return Path(filepath).stem.replace("_", " ").replace("-", " ")

def generate_id(filepath: str) -> str:
    """Generate a unique 14-digit numeric ID from filepath hash."""
    h = hashlib.md5(filepath.encode()).hexdigest()
    # Take first 14 hex chars, convert to numeric
    num = int(h[:14], 16)
    # Ensure it's 14 digits
    return str(num)[:14].zfill(14)

def extract_tags(content: str) -> list:
    """Extract tags from content: hex addresses, function names, key terms."""
    tags = set()
    
    # Hex addresses (0x...)
    for m in re.finditer(r'\b(0x[0-9A-Fa-f]{6,8})\b', content):
        tags.add(m.group(1).lower())
    
    # Function-like names (Ball_Update, Scene_SetCamera, etc.)
    for m in re.finditer(r'\b([A-Z][a-z]+_[A-Z][a-z]+(?:_[A-Z][a-z]+)*)\b', content):
        tags.add(m.group(1))
    
    # Limit to top 15 tags per file
    return sorted(list(tags))[:15]

def extract_headings(content: str) -> list:
    """Extract section headings for structure."""
    headings = []
    for m in re.finditer(r'^#{1,4}\s+(.+)$', content, re.MULTILINE):
        headings.append(m.group(1).strip())
    return headings

GITHUB_BASE = "https://github.com/evangit2/hamsterball-re/blob/master"

def get_github_url(rel_path: str) -> str:
    """Get GitHub URL for a file in the repo."""
    encoded = rel_path.replace(" ", "%20")
    return f"{GITHUB_BASE}/{encoded}"

def should_exclude(filepath: str) -> bool:
    """Check if file should be excluded."""
    for pattern in EXCLUDE_PATTERNS:
        if pattern in filepath:
            return True
    return False

def collect_files() -> list:
    """Collect all project markdown files."""
    all_files = []
    
    # Include specific directories
    for inc_dir in INCLUDE_DIRS:
        full_dir = REPO_ROOT / inc_dir
        if not full_dir.exists():
            continue
        for md_file in full_dir.rglob("*.md"):
            rel_path = str(md_file.relative_to(REPO_ROOT))
            if should_exclude(rel_path):
                continue
            all_files.append(rel_path)
    
    # Include specific root files
    for inc_file in INCLUDE_FILES:
        full_file = REPO_ROOT / inc_file
        if full_file.exists():
            all_files.append(inc_file)
    
    return sorted(all_files)

def read_file_content(rel_path: str) -> str:
    """Read file content."""
    full_path = REPO_ROOT / rel_path
    with open(full_path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()

def strip_frontmatter(content: str) -> str:
    """Remove existing YAML frontmatter."""
    if content.startswith("---"):
        end = content.find("---", 3)
        if end != -1:
            return content[end+3:].lstrip("\n")
    return content

def add_frontmatter(content: str, record_id: str, title: str, 
                    record_type: str, tags: list, github_url: str) -> str:
    """Add YAML frontmatter to content, plus a GitHub source link at top."""
    body = strip_frontmatter(content)
    
    # Add GitHub source link at top of body
    github_link = f"\n> 📂 [View source on GitHub]({github_url})\n\n---\n\n"
    
    frontmatter = "---\n"
    escaped_title = title.replace('"', "'")
    frontmatter += f'title: "{escaped_title}"\n'
    frontmatter += f"id: {record_id}\n"
    frontmatter += f"types:\n- {record_type}\n"
    if tags:
        frontmatter += "tags:\n"
        for tag in tags[:15]:
            frontmatter += f"- \"{tag}\"\n"
    frontmatter += "---\n\n"
    
    return frontmatter + github_link + body

def build_wikilink_map(files_info: dict) -> dict:
    """Build a map of (title/filename → record_id) for wikilink insertion.
    Only map by exact title — never by filename stems which are too generic."""
    link_map = {}
    for rel_path, info in files_info.items():
        title = info["title"].lower()
        # Only map titles that are specific enough (>8 chars, not generic)
        if len(title) < 8:
            continue
        # Skip generic titles that would cause false matches
        GENERIC = {
            "readme", "index", "todo", "overview", "introduction",
            "summary", "notes", "references", "changelog"
        }
        if title in GENERIC:
            continue
        link_map[title] = info["id"]
    return link_map

def insert_wikilinks(content: str, link_map: dict, own_id: str) -> str:
    """Insert [[wikilinks]] for cross-references to other documents.
    Conservative: only link exact title matches in prose, not in code/headings/links."""
    
    lines = content.split("\n")
    in_code = False
    result = []
    
    for line in lines:
        stripped = line.strip()
        
        # Track code blocks
        if stripped.startswith("```"):
            in_code = not in_code
            result.append(line)
            continue
        
        if in_code:
            result.append(line)
            continue
        
        # Don't modify headings, frontmatter, or lines with URLs
        if stripped.startswith("---") or stripped.startswith("#") or stripped.startswith("http"):
            result.append(line)
            continue
        
        modified = line
        
        for title, rid in sorted(link_map.items(), key=lambda x: -len(x[0])):
            if rid == own_id:
                continue
            
            # Exact phrase match, word-boundary, case-insensitive
            # Only match the exact title as a phrase
            pattern = r'(?<!\w)(' + re.escape(title) + r')(?!\w)'
            replacement = f'[[{rid}|{title}]]'
            
            # Don't replace if already wikilinked or inside markdown link syntax
            if f'[[{rid}' not in modified and f']({title}' not in modified:
                modified = re.sub(pattern, replacement, modified, flags=re.IGNORECASE, count=1)
        
        result.append(modified)
    
    return "\n".join(result)

def compute_related_docs(files_info: dict) -> dict:
    """For each doc, find related docs by shared tags and same type.
    Returns map of rel_path → list of (other_path, score) tuples."""
    related = {}
    
    # Build tag → docs index
    tag_to_docs = defaultdict(list)
    for rel_path, info in files_info.items():
        for tag in info["tags"]:
            tag_to_docs[tag].append(rel_path)
    
    for rel_path, info in files_info.items():
        scores = defaultdict(int)
        my_tags = set(info["tags"])
        my_type = info["type"]
        
        # Score by shared tags
        for tag in my_tags:
            for other_path in tag_to_docs[tag]:
                if other_path == rel_path:
                    continue
                # Weight hex address matches heavily (same function/offset = strongly related)
                weight = 5 if tag.startswith("0x") else 1
                scores[other_path] += weight
        
        # Bonus for same type
        for other_path, other_info in files_info.items():
            if other_path == rel_path:
                continue
            if other_info["type"] == my_type:
                scores[other_path] += 1
        
        # Sort by score, take top 2
        top = sorted(scores.items(), key=lambda x: -x[1])[:2]
        related_list = [(p, s) for p, s in top if s >= 5]
        # Ensure every doc has at least 1 link (take strongest match even if score < 5)
        if not related_list and top:
            related_list = [top[0]]
        related[rel_path] = related_list
    
    return related

def add_related_links(content: str, own_id: str, related: list, files_info: dict) -> str:
    """Append a 'Related Documents' section with wikilinks at the bottom."""
    if not related:
        return content
    
    links_section = "\n\n---\n\n## 🔗 Related Documents\n\n"
    for other_path, score in related:
        other_info = files_info[other_path]
        title = other_info["title"]
        rid = other_info["id"]
        links_section += f"- [[{rid}|{title}]]\n"
    
    return content + links_section

def write_config_yml():
    """Write Cosma configuration file."""
    
    record_types_yaml = ""
    for rtype, colors in RECORD_TYPES.items():
        record_types_yaml += f"  {rtype}:\n"
        record_types_yaml += f"    fill: \"{colors['fill']}\"\n"
        record_types_yaml += f"    stroke: \"{colors['stroke']}\"\n"
    
    config = f"""# Cosma configuration for Hamsterball RE Mind Map
select_origin: directory
files_origin: {CONTENT_DIR}
images_origin: {Path(__file__).parent / "images"}
export_target: {Path(__file__).parent}
history: false
focus_max: 3
record_types:
{record_types_yaml}
link_types:
  undefined:
    stroke: dash
    color: "#a0a0a0"
graph_background_color: "#1a1a2e"
graph_text_color: "#e0e0e0"
graph_link_color: "#555555"
graph_highlight_color: "#ff6a6a"
graph_text_size: 8
graph_arrows: true
node_size_method: degree
node_size: 5
node_size_max: 10
node_size_min: 3
attraction_force: 50
attraction_distance_max: 1200
attraction_vertical: 0.0
attraction_horizontal: 0.0
generate_id: always
link_context: tooltip
hide_id_from_record_header: true
title: "Hamsterball RE — Knowledge Map"
author: "rsks & contributors"
description: "Interactive mind map of all Hamsterball reverse engineering documentation, mods, and tools"
keywords:
  - hamsterball
  - reverse engineering
  - ghidra
  - game modding
lang: en
"""
    with open(CONFIG_PATH, "w") as f:
        f.write(config)


def inject_search_overlay(html: str, files_info: dict) -> str:
    """Inject a powerful full-text search overlay into the cosmoscope HTML.
    
    Features:
    - Ctrl+K / Cmd+K to open
    - Full-text search across all document content
    - Highlights matched snippets with context
    - Searches hex addresses, function names, titles, tags, and body text
    - Results clickable to navigate to the record in the cosmoscope
    - Each result has a 'View on GitHub' link
    """
    
    import json as jsonmod
    
    # Build search index
    search_index = []
    for rel_path, info in files_info.items():
        content = info["original_content"]
        # Strip frontmatter and code blocks for indexing
        clean = strip_frontmatter(content)
        clean = re.sub(r'```[\s\S]*?```', ' ', clean)  # Remove code blocks
        clean = re.sub(r'`[^`]+`', ' ', clean)  # Remove inline code
        clean = re.sub(r'!\[.*?\]\(.*?\)', ' ', clean)  # Remove images
        clean = re.sub(r'\[([^\]]+)\]\([^\)]+\)', r'\1', clean)  # Keep link text
        clean = re.sub(r'[#*|>]', ' ', clean)  # Remove markdown symbols
        clean = re.sub(r'\s+', ' ', clean).strip()
        
        search_index.append({
            "id": info["id"],
            "title": info["title"],
            "type": info["type"],
            "tags": info["tags"],
            "github_url": info["github_url"],
            "path": rel_path,
            "content": clean[:2000],  # First 2K chars for snippet extraction
        })
    
    index_json = jsonmod.dumps(search_index)
    
    search_overlay = f"""
<style>
#hbremap-search-overlay {{
  position: fixed; top: 0; left: 0; width: 100vw; height: 100vh;
  background: rgba(0,0,0,0.7); backdrop-filter: blur(4px);
  z-index: 99999; display: none; align-items: flex-start; justify-content: center;
  padding-top: 80px;
}}
#hbremap-search-overlay.active {{ display: flex; }}
#hbremap-search-box {{
  width: 90%; max-width: 800px; background: #1a1a2e; border: 1px solid #444;
  border-radius: 12px; box-shadow: 0 8px 32px rgba(0,0,0,0.8);
  overflow: hidden; max-height: 80vh; display: flex; flex-direction: column;
}}
#hbremap-search-input {{
  width: 100%; padding: 16px 20px; font-size: 16px; color: #e0e0e0;
  background: transparent; border: none; border-bottom: 1px solid #333;
  outline: none; box-sizing: border-box;
}}
#hbremap-search-input::placeholder {{ color: #666; }}
#hbremap-search-results {{
  overflow-y: auto; flex: 1;
}}
.hbremap-search-result {{
  padding: 12px 20px; border-bottom: 1px solid #222; cursor: pointer;
  transition: background 0.15s;
}}
.hbremap-search-result:hover {{ background: rgba(255,255,255,0.05); }}
.hbremap-search-result-title {{
  font-size: 14px; font-weight: 600; color: #4a9eff; margin-bottom: 4px;
}}
.hbremap-search-result-type {{
  display: inline-block; font-size: 10px; padding: 1px 6px; border-radius: 3px;
  margin-left: 8px; vertical-align: middle; text-transform: uppercase; letter-spacing: 0.5px;
}}
.hbremap-search-result-snippet {{
  font-size: 12px; color: #aaa; line-height: 1.5; margin-top: 4px;
}}
.hbremap-search-result-snippet mark {{
  background: #ff6a6a; color: #fff; padding: 0 2px; border-radius: 2px;
}}
.hbremap-search-result-github {{
  font-size: 11px; color: #666; margin-top: 4px; text-decoration: none;
}}
.hbremap-search-result-github:hover {{ color: #4a9eff; }}
.hbremap-search-empty {{
  padding: 40px; text-align: center; color: #666; font-size: 14px;
}}
.hbremap-search-hint {{
  padding: 8px 20px; font-size: 11px; color: #555; border-top: 1px solid #222;
  text-align: right;
}}
</style>
<div id="hbremap-search-overlay">
  <div id="hbremap-search-box">
    <input id="hbremap-search-input" type="text" placeholder="Search documents, hex addresses, function names..." autocomplete="off" />
    <div id="hbremap-search-results"></div>
    <div class="hbremap-search-hint">Esc to close · Enter to open first result · Ctrl+K to toggle</div>
  </div>
</div>
<script>
(function() {{
  const SEARCH_INDEX = {index_json};
  const overlay = document.getElementById('hbremap-search-overlay');
  const input = document.getElementById('hbremap-search-input');
  const resultsDiv = document.getElementById('hbremap-search-results');
  let selectedIndex = -1;
  let currentResults = [];

  function escapeHtml(s) {{
    return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
  }}

  function highlight(text, query) {{
    if (!query) return escapeHtml(text);
    const escaped = escapeHtml(text);
    const terms = query.toLowerCase().split(/\\s+/).filter(t => t.length > 0);
    let result = escaped;
    for (const term of terms) {{
      const regex = new RegExp('(' + term.replace(/[.*+?^${{}}()|[\\]\\\\]/g, '\\\\$&') + ')', 'gi');
      result = result.replace(regex, '<mark>$1</mark>');
    }}
    return result;
  }}

  function getSnippet(content, query, maxLen) {{
    if (!query) return content.substring(0, maxLen) + '...';
    const lower = content.toLowerCase();
    const terms = query.toLowerCase().split(/\\s+/).filter(t => t.length > 0);
    let bestPos = 0;
    let bestScore = 0;
    for (const term of terms) {{
      const pos = lower.indexOf(term);
      if (pos >= 0) {{
        const score = term.length;
        if (score > bestScore) {{ bestScore = score; bestPos = pos; }}
      }}
    }}
    const start = Math.max(0, bestPos - 60);
    const end = Math.min(content.length, start + maxLen);
    let snippet = content.substring(start, end);
    if (start > 0) snippet = '...' + snippet;
    if (end < content.length) snippet = snippet + '...';
    return snippet;
  }}

  function search(query) {{
    if (!query || query.trim().length === 0) {{
      resultsDiv.innerHTML = '';
      currentResults = [];
      return;
    }}
    const q = query.toLowerCase().trim();
    const terms = q.split(/\\s+/);
    const results = [];
    for (const doc of SEARCH_INDEX) {{
      const titleLower = doc.title.toLowerCase();
      const contentLower = doc.content.toLowerCase();
      const tagsLower = (doc.tags || []).join(' ').toLowerCase();
      const pathMatch = doc.path.toLowerCase();
      
      let score = 0;
      let matched = false;
      
      for (const term of terms) {{
        if (titleLower.includes(term)) {{ score += 10; matched = true; }}
        if (pathMatch.includes(term)) {{ score += 8; matched = true; }}
        if (tagsLower.includes(term)) {{ score += 5; matched = true; }}
        if (contentLower.includes(term)) {{ score += 1; matched = true; }}
        // Hex address exact match bonus
        if (/^0x[0-9a-f]{{4,8}}$/.test(term) && tagsLower.includes(term)) {{ score += 50; }}
      }}
      
      if (matched) {{
        results.push({{...doc, score}});
      }}
    }}
    results.sort((a, b) => b.score - a.score);
    currentResults = results.slice(0, 30);
    renderResults(currentResults, query);
  }}

  const typeColors = {jsonmod.dumps({k: v["fill"] for k, v in RECORD_TYPES.items()})};

  function renderResults(results, query) {{
    if (results.length === 0) {{
      resultsDiv.innerHTML = '<div class="hbremap-search-empty">No results found for "' + escapeHtml(query) + '"</div>';
      selectedIndex = -1;
      return;
    }}
    selectedIndex = 0;
    resultsDiv.innerHTML = results.map((r, i) => {{
      const color = typeColors[r.type] || '#666';
      const snippet = getSnippet(r.content, query, 150);
      return '<div class="hbremap-search-result" data-id="' + r.id + '" data-index="' + i + '">' +
        '<div class="hbremap-search-result-title">' +
          highlight(r.title, query) +
          '<span class="hbremap-search-result-type" style="background:' + color + ';color:#fff;">' + r.type + '</span>' +
        '</div>' +
        '<div class="hbremap-search-result-snippet">' + highlight(snippet, query) + '</div>' +
        '<a class="hbremap-search-result-github" href="' + r.github_url + '" target="_blank" onclick="event.stopPropagation()">📂 View on GitHub →</a>' +
      '</div>';
    }}).join('');
    
    // Update selected
    updateSelection();
    
    // Click handlers
    resultsDiv.querySelectorAll('.hbremap-search-result').forEach(el => {{
      el.addEventListener('click', function() {{
        const id = this.getAttribute('data-id');
        openRecord(id);
      }});
    }});
  }}

  function updateSelection() {{
    resultsDiv.querySelectorAll('.hbremap-search-result').forEach((el, i) => {{
      if (i === selectedIndex) el.style.background = 'rgba(74,158,255,0.15)';
      else el.style.background = '';
    }});
  }}

  function openRecord(id) {{
    // Find the node in the cosmoscope and click it
    const node = document.querySelector('[data-id="' + id + '"]');
    if (node) {{
      node.click();
      closeSearch();
    }} else {{
      // Try to find by link text
      const links = document.querySelectorAll('a');
      for (const link of links) {{
        if (link.textContent.includes(id) || link.getAttribute('href') === '#' + id) {{
          link.click();
          closeSearch();
          return;
        }}
      }}
      // Fallback: just close
      closeSearch();
    }}
  }}

  function openSearch() {{
    overlay.classList.add('active');
    setTimeout(() => input.focus(), 50);
  }}

  function closeSearch() {{
    overlay.classList.remove('active');
    input.value = '';
    resultsDiv.innerHTML = '';
    currentResults = [];
  }}

  // Event listeners
  document.addEventListener('keydown', function(e) {{
    if ((e.ctrlKey || e.metaKey) && e.key === 'k') {{
      e.preventDefault();
      if (overlay.classList.contains('active')) closeSearch();
      else openSearch();
    }}
    if (e.key === 'Escape' && overlay.classList.contains('active')) {{
      closeSearch();
    }}
    if (overlay.classList.contains('active')) {{
      if (e.key === 'ArrowDown') {{
        e.preventDefault();
        selectedIndex = Math.min(selectedIndex + 1, currentResults.length - 1);
        updateSelection();
        const el = resultsDiv.children[selectedIndex];
        if (el) el.scrollIntoView({{block: 'nearest'}});
      }}
      if (e.key === 'ArrowUp') {{
        e.preventDefault();
        selectedIndex = Math.max(selectedIndex - 1, 0);
        updateSelection();
        const el = resultsDiv.children[selectedIndex];
        if (el) el.scrollIntoView({{block: 'nearest'}});
      }}
      if (e.key === 'Enter' && selectedIndex >= 0 && currentResults[selectedIndex]) {{
        e.preventDefault();
        openRecord(currentResults[selectedIndex].id);
      }}
    }}
  }});

  input.addEventListener('input', function() {{ search(this.value); }});

  overlay.addEventListener('click', function(e) {{
    if (e.target === overlay) closeSearch();
  }});

  // Add a floating search button
  const searchBtn = document.createElement('div');
  searchBtn.style.cssText = 'position:fixed;bottom:20px;right:20px;width:48px;height:48px;' +
    'background:#4a9eff;border-radius:50%;cursor:pointer;z-index:99998;' +
    'display:flex;align-items:center;justify-content:center;font-size:20px;' +
    'color:white;box-shadow:0 4px 12px rgba(74,158,255,0.5);transition:transform 0.2s;';
  searchBtn.innerHTML = '🔍';
  searchBtn.title = 'Search (Ctrl+K)';
  searchBtn.onclick = openSearch;
  searchBtn.onmouseenter = function() {{ this.style.transform = 'scale(1.1)'; }};
  searchBtn.onmouseleave = function() {{ this.style.transform = 'scale(1)'; }};
  document.body.appendChild(searchBtn);

  console.log('hbremap search overlay loaded — ' + SEARCH_INDEX.length + ' documents indexed');
}})();
</script>
"""
    
    # Inject before </body>
    if "</body>" in html:
        html = html.replace("</body>", search_overlay + "\n</body>", 1)
    else:
        html = html + search_overlay
    
    return html

def main():
    print("=" * 60)
    print("  hbremap — Building Cosma Cosmoscope")
    print("=" * 60)
    
    # Step 1: Collect files
    files = collect_files()
    print(f"\n[1/5] Collected {len(files)} markdown files")
    
    # Step 2: Read and analyze all files
    files_info = {}
    for rel_path in files:
        content = read_file_content(rel_path)
        title = extract_title(rel_path, content)
        record_id = generate_id(rel_path)
        record_type = get_type_for_file(rel_path)
        tags = extract_tags(content)
        
        files_info[rel_path] = {
            "id": record_id,
            "title": title,
            "type": record_type,
            "tags": tags,
            "github_url": get_github_url(rel_path),
            "original_content": content,
        }
    
    print(f"[2/5] Analyzed {len(files_info)} documents")
    
    # Step 3: Build wikilink map and process content
    link_map = build_wikilink_map(files_info)
    related_map = compute_related_docs(files_info)
    
    # Count how many docs got related links
    total_related = sum(1 for v in related_map.values() if len(v) > 0)
    total_links_count = sum(len(v) for v in related_map.values())
    print(f"       Related doc links: {total_links_count} across {total_related} docs")
    
    # Clear content directory
    if CONTENT_DIR.exists():
        shutil.rmtree(CONTENT_DIR)
    CONTENT_DIR.mkdir(parents=True)
    
    for rel_path, info in files_info.items():
        content = info["original_content"]
        content = add_frontmatter(content, info["id"], info["title"],
                                  info["type"], info["tags"], info["github_url"])
        content = insert_wikilinks(content, link_map, info["id"])
        # Add related documents section for more graph edges
        content = add_related_links(content, info["id"], related_map.get(rel_path, []), files_info)
        
        # Write to content dir with sanitized filename
        safe_name = rel_path.replace("/", "_").replace(" ", "-")
        out_path = CONTENT_DIR / safe_name
        out_path.parent.mkdir(parents=True, exist_ok=True)
        with open(out_path, "w", encoding="utf-8") as f:
            f.write(content)
    
    print(f"[3/5] Wrote {len(files_info)} processed files to {CONTENT_DIR}")
    
    # Print stats
    print(f"       Tags extracted: {sum(len(i['tags']) for i in files_info.values())}")
    
    # Step 4: Write config
    write_config_yml()
    print(f"[4/5] Config written to {CONFIG_PATH}")
    
    # Step 5: Run Cosma
    print(f"[5/5] Running cosma modelize...")
    result = subprocess.run(
        ["cosma", "modelize"],
        cwd=str(Path(__file__).parent),
        capture_output=True,
        text=True,
        timeout=120
    )
    print(f"       stdout: {result.stdout.strip()}")
    if result.stderr:
        print(f"       stderr: {result.stderr.strip()}")
    print(f"       exit code: {result.returncode}")
    
    # Check output
    cosmoscope_path = Path(__file__).parent / "cosmoscope.html"
    # Rename to index.html for GitHub Pages
    index_path = Path(__file__).parent / "index.html"
    if cosmoscope_path.exists():
        # Inject enhanced search overlay
        html = cosmoscope_path.read_text(encoding="utf-8")
        enhanced = inject_search_overlay(html, files_info)
        index_path.write_text(enhanced, encoding="utf-8")
        size = index_path.stat().st_size
        print(f"\n✅ Cosmoscope generated: {index_path} ({size:,} bytes)")
    else:
        print(f"\n❌ No output HTML found!")
    
    return files_info

if __name__ == "__main__":
    main()
