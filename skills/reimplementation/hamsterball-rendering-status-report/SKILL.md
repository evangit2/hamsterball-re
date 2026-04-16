---
name: hamsterball-rendering-status-report
description: Guidelines for documenting rendering milestones with screenshots and memory updates.
---

# Rendering Status Report Guidelines

When a major rendering milestone is reached (e.g., geometry visible, textures working, or lighting added):

1. **Verify Progress with Screenshots**:
   - Save the screenshot to `analysis/screenshots/`.
   - Describe what is visible and what is missing.
   - Use `vision_analyze` if necessary to confirm shapes.

2. **Update Memory**:
   - Update `memory` with technical details of the implementation (vertex format, FVF, matrix layout).
   - Update `hamsterball-re-progress` skill to mark the task as complete.

3. **Sync Git**:
   - Commit all changes (code and screenshots).
   - Use `git-sync` protocol to push to both `origin` (public) and `priv` (private).

4. **Document Implementation Quirks**:
   - Record any findings about file formats or API behavior (e.g. Wine compatibility issues with indexed primitives).
