// ============================================================
// Bend Gizmo — Mouse-based visual bend tool for Three.js editor
// Pure editor-side: no changes to import/export/collision/events
// ============================================================
//
// USAGE:
//   const gizmo = new BendGizmo(scene, renderer, camera);
//   gizmo.attach(selectedItem);   // enter bend mode
//   gizmo.detach();               // exit bend mode (commit)
//   gizmo.cancel();               // exit bend mode (revert)
//
// KEYBOARD:
//   B = enter bend mode (if object selected)
//   Enter = commit
//   Esc = cancel
//
// MOUSE:
//   Drag the arc handle = adjust bend angle
//   Scroll wheel = adjust pivot offset (0.0–1.0)
//
// ============================================================

class BendGizmo {

    constructor(scene, camera, renderer, orbitControls) {
        this.scene = scene;
        this.camera = camera;
        this.renderer = renderer;
        this.orbitControls = orbitControls || null;

        this.item = null;          // selected editor item
        this.hbMesh = null;        // item.hbMesh
        this.originalVerts = null; // Float32Array saved on attach
        this.active = false;       // is bend mode on?

        // Bend params
        this.bendAxis = 2;         // 0=X, 1=Y, 2=Z (default Z — ramp direction)
        this.curveDir = 1;         // 0=X, 1=Y, 2=Z (default Y — upward)
        this.angleDeg = 0;         // current bend angle
        this.pivotOffset = 0;      // 0.0–1.0

        // Gizmo group (all visual elements live here)
        this.group = new THREE.Group();
        this.group.visible = false;
        this.scene.add(this.group);

        // Interaction state
        this.dragging = false;
        this.dragStart = null;     // {x, y} screen coords
        this.dragStartAngle = 0;
        this.raycastPlane = null;  // reusable plane for raycasting

        // Bind handlers
        this._onPointerDown = this._handlePointerDown.bind(this);
        this._onPointerMove = this._handlePointerMove.bind(this);
        this._onPointerUp = this._handlePointerUp.bind(this);
        this._onWheel = this._handleWheel.bind(this);
        this._onKeyDown = this._handleKeyDown.bind(this);
    }

    // --------------------------------------------------------
    // Mode management
    // --------------------------------------------------------

    /**
     * Enter bend mode for the selected item.
     * Saves original vertices for cancel/restore.
     */
    attach(item) {
        if (!item || !item.hbMesh) {
            console.warn('BendGizmo: no item/mesh selected');
            return;
        }

        // If already active, detach first
        if (this.active) this.detach();

        this.item = item;
        this.hbMesh = item.hbMesh;
        this.originalVerts = new Float32Array(item.hbMesh.vertices); // deep copy
        this.active = true;

        // Reset angle
        this.angleDeg = 0;
        this.pivotOffset = 0;

        // Auto-detect best bend axis (longest extent)
        this._autoDetectBendAxis();

        // Build visual gizmo
        this._rebuildGizmo();
        this.group.visible = true;

        // Register event listeners
        this._addListeners();

        console.log('BendGizmo: attached');
    }

    /**
     * Commit the bend and exit mode.
     * Keeps the bent vertices in the mesh.
     */
    detach() {
        if (!this.active) return;

        // Apply final bend if angle is non-zero (already applied in preview)
        this._removeListeners();
        this.group.visible = false;
        this._disposeGizmo();

        this.item = null;
        this.hbMesh = null;
        this.originalVerts = null;
        this.active = false;
        this.dragging = false;

        console.log('BendGizmo: detached (committed)');
    }

    /**
     * Cancel the bend and exit mode.
     * Restores original vertices.
     */
    cancel() {
        if (!this.active) return;

        // Restore original vertices
        if (this.originalVerts && this.hbMesh) {
            this.hbMesh.vertices = new Float32Array(this.originalVerts);
            if (typeof refreshSingleObject === 'function') {
                refreshSingleObject(this.item);
            }
        }

        this._removeListeners();
        this.group.visible = false;
        this._disposeGizmo();

        this.item = null;
        this.hbMesh = null;
        this.originalVerts = null;
        this.active = false;
        this.dragging = false;

        console.log('BendGizmo: cancelled (reverted)');
    }

    // --------------------------------------------------------
    // Auto-detection
    // --------------------------------------------------------

    /**
     * Auto-detect the longest axis of the selected geometry.
     * That becomes the bend axis. Curve direction defaults to Y (up).
     */
    _autoDetectBendAxis() {
        const verts = this.hbMesh.vertices;
        const n = verts.length / 8;
        if (n === 0) return;

        let minX=Infinity, maxX=-Infinity;
        let minY=Infinity, maxY=-Infinity;
        let minZ=Infinity, maxZ=-Infinity;

        for (let i = 0; i < n; i++) {
            const off = i * 8;
            if (verts[off]   < minX) minX = verts[off];
            if (verts[off]   > maxX) maxX = verts[off];
            if (verts[off+1] < minY) minY = verts[off+1];
            if (verts[off+1] > maxY) maxY = verts[off+1];
            if (verts[off+2] < minZ) minZ = verts[off+2];
            if (verts[off+2] > maxZ) maxZ = verts[off+2];
        }

        const dx = maxX - minX;
        const dy = maxY - minY;
        const dz = maxZ - minZ;

        // Pick longest axis as bend axis
        if (dz >= dx && dz >= dy) {
            this.bendAxis = 2; // Z
        } else if (dx >= dy) {
            this.bendAxis = 0; // X
        } else {
            this.bendAxis = 1; // Y
        }

        // Curve direction: prefer Y (upward), unless Y is the bend axis
        if (this.bendAxis === 1) {
            this.curveDir = 2; // Z if bending along Y
        } else {
            this.curveDir = 1; // Y otherwise
        }
    }

    // --------------------------------------------------------
    // Gizmo construction
    // --------------------------------------------------------

    _disposeGizmo() {
        while (this.group.children.length > 0) {
            const child = this.group.children[0];
            this.group.remove(child);
            if (child.geometry) child.geometry.dispose();
            if (child.material) child.material.dispose();
        }
    }

    /**
     * Rebuild the visual gizmo: arc, reference line, pivot sphere, direction arrow.
     */
    _rebuildGizmo() {
        this._disposeGizmo();

        const verts = this.originalVerts;
        const n = verts.length / 8;
        if (n === 0) return;

        // Get bounds
        const bounds = this._getBounds();
        const center = {
            x: (bounds.minX + bounds.maxX) / 2,
            y: (bounds.minY + bounds.maxY) / 2,
            z: (bounds.minZ + bounds.maxZ) / 2,
        };

        const bAxis = this.bendAxis;
        const cDir = this.curveDir;
        const pAxis = 3 - bAxis - cDir;

        const bendMin = [bounds.minX, bounds.minY, bounds.minZ][bAxis];
        const bendMax = [bounds.maxX, bounds.maxY, bounds.maxZ][bAxis];
        const bendLength = bendMax - bendMin;

        // Pivot position along bend axis
        const pivot = bendMin + bendLength * this.pivotOffset;

        // 1. Reference line (original geometry extent — straight, thin gray)
        const lineMat = new THREE.LineBasicMaterial({ color: 0x888888, transparent: true, opacity: 0.5 });
        const lineGeo = new THREE.BufferGeometry();
        const linePts = [];
        const perpCenter = ([bounds.minX, bounds.minY, bounds.minZ][pAxis] + [bounds.maxX, bounds.maxY, bounds.maxZ][pAxis]) / 2;
        const curveCenter = ([bounds.minX, bounds.minY, bounds.minZ][cDir] + [bounds.maxX, bounds.maxY, bounds.maxZ][cDir]) / 2;
        const p0 = [0,0,0]; p0[bAxis] = bendMin; p0[cDir] = curveCenter; p0[pAxis] = perpCenter;
        const p1 = [0,0,0]; p1[bAxis] = bendMax; p1[cDir] = curveCenter; p1[pAxis] = perpCenter;
        lineGeo.setAttribute('position', new THREE.Float32BufferAttribute([...p0, ...p1], 3));
        this.group.add(new THREE.Line(lineGeo, lineMat));

        // 2. Arc (shows the current bend curve — semi-transparent colored tube)
        this.arcMesh = this._createArcMesh(bendLength, pivot, bAxis, cDir, pAxis);
        this.group.add(this.arcMesh);

        // 3. Pivot sphere (draggable — sets pivotOffset)
        const sphereGeo = new THREE.SphereGeometry(8, 16, 16);
        const sphereMat = new THREE.MeshBasicMaterial({ color: 0xffff00, transparent: true, opacity: 0.8 });
        this.pivotSphere = new THREE.Mesh(sphereGeo, sphereMat);
        this._setPivotSpherePosition(pivot, bAxis, cDir, pAxis, curveCenter, perpCenter);
        this.group.add(this.pivotSphere);

        // 4. Direction arrow (shows curve direction)
        const arrowDir = [0, 0, 0];
        arrowDir[cDir] = 1;
        const arrow = new THREE.ArrowHelper(
            new THREE.Vector3(...arrowDir),
            new THREE.Vector3(center.x, center.y, center.z),
            40, 0x00ff00, 15, 10
        );
        this.group.add(arrow);

        // 5. Bend handle (invisible sphere for raycasting — larger than visual elements)
        const handleGeo = new THREE.SphereGeometry(20, 8, 8);
        const handleMat = new THREE.MeshBasicMaterial({ visible: false });
        this.dragHandle = new THREE.Mesh(handleGeo, handleMat);
        // Place handle at the end of the arc (where geometry bends most)
        const handlePos = [0,0,0];
        handlePos[bAxis] = bendMax;
        handlePos[cDir] = curveCenter;
        handlePos[pAxis] = perpCenter;
        this.dragHandle.position.set(...handlePos);
        this.dragHandle.userData.isBendHandle = true;
        this.group.add(this.dragHandle);
    }

    _createArcMesh(length, pivot, bAxis, cDir, pAxis) {
        const segments = 64;
        const angleRad = this.angleDeg * Math.PI / 180;
        const R = Math.abs(angleRad) < 0.001 ? 0 : length / angleRad;

        const points = [];
        for (let s = 0; s <= segments; s++) {
            const t = s / segments;
            const d = t * length;
            const theta = (d / length) * angleRad;
            const cosT = Math.cos(theta);
            const sinT = Math.sin(theta);
            const r = R;

            const pos = [0, 0, 0];
            pos[bAxis] = pivot + r * sinT;
            pos[cDir] = R - r * cosT;
            pos[pAxis] = 0;
            points.push(new THREE.Vector3(...pos));
        }

        const curve = new THREE.CatmullRomCurve3(points);
        const tubeGeo = new THREE.TubeGeometry(curve, segments, 3, 8, false);
        const tubeMat = new THREE.MeshBasicMaterial({
            color: 0x00aaff,
            transparent: true,
            opacity: 0.6,
        });
        return new THREE.Mesh(tubeGeo, tubeMat);
    }

    _setPivotSpherePosition(pivot, bAxis, cDir, pAxis, curveCenter, perpCenter) {
        const pos = [0, 0, 0];
        pos[bAxis] = pivot;
        pos[cDir] = curveCenter;
        pos[pAxis] = perpCenter;
        this.pivotSphere.position.set(...pos);
    }

    _getBounds() {
        const verts = this.originalVerts;
        const n = verts.length / 8;
        let minX=Infinity, maxX=-Infinity;
        let minY=Infinity, maxY=-Infinity;
        let minZ=Infinity, maxZ=-Infinity;
        for (let i = 0; i < n; i++) {
            const off = i * 8;
            if (verts[off]   < minX) minX = verts[off];
            if (verts[off]   > maxX) maxX = verts[off];
            if (verts[off+1] < minY) minY = verts[off+1];
            if (verts[off+1] > maxY) maxY = verts[off+1];
            if (verts[off+2] < minZ) minZ = verts[off+2];
            if (verts[off+2] > maxZ) maxZ = verts[off+2];
        }
        return { minX, maxX, minY, maxY, minZ, maxZ };
    }

    // --------------------------------------------------------
    // Bend application
    // --------------------------------------------------------

    _applyBend() {
        if (!this.originalVerts || !this.hbMesh) return;

        // Use BendTool.bendVertices if available, otherwise inline
        if (typeof BendTool !== 'undefined' && BendTool.bendVertices) {
            this.hbMesh.vertices = BendTool.bendVertices(
                this.originalVerts,
                this.originalVerts.length / 8,
                {
                    bendAxis: this.bendAxis,
                    curveDir: this.curveDir,
                    angleDeg: this.angleDeg,
                    pivotOffset: this.pivotOffset,
                }
            );
        } else {
            this.hbMesh.vertices = this._bendInline();
        }

        // Refresh preview
        if (typeof refreshSingleObject === 'function') {
            refreshSingleObject(this.item);
        }

        // Update gizmo arc
        this._rebuildGizmo();
    }

    _bendInline() {
        const verts = this.originalVerts;
        const n = verts.length / 8;
        const out = new Float32Array(verts.length);

        const bAxis = this.bendAxis;
        const cDir = this.curveDir;
        const pAxis = 3 - bAxis - cDir;

        // Bounds
        let minB = Infinity, maxB = -Infinity;
        for (let i = 0; i < n; i++) {
            const v = verts[i * 8 + bAxis];
            if (v < minB) minB = v;
            if (v > maxB) maxB = v;
        }
        const length = maxB - minB;
        if (length < 0.001) return new Float32Array(verts);

        const pivot = minB + length * this.pivotOffset;
        const totalAngle = this.angleDeg * Math.PI / 180;
        const useLinear = Math.abs(totalAngle) < 0.001;
        const R = useLinear ? 0 : length / totalAngle;

        for (let i = 0; i < n; i++) {
            const off = i * 8;
            const bendCoord = verts[off + bAxis];
            const curveCoord = verts[off + cDir];
            const perpCoord = verts[off + pAxis];

            const nBend = verts[off + 3 + bAxis];
            const nCurve = verts[off + 3 + cDir];
            const nPerp = verts[off + 3 + pAxis];

            if (useLinear) {
                for (let k = 0; k < 8; k++) out[off + k] = verts[off + k];
            } else {
                const d = bendCoord - pivot;
                const theta = (d / length) * totalAngle;
                const cosT = Math.cos(theta);
                const sinT = Math.sin(theta);
                const r = R - curveCoord;

                out[off + pAxis] = perpCoord;
                out[off + cDir] = R - r * cosT;
                out[off + bAxis] = pivot + r * sinT;

                out[off + 3 + pAxis] = nPerp;
                out[off + 3 + cDir] = nCurve * cosT - nBend * sinT;
                out[off + 3 + bAxis] = nCurve * sinT + nBend * cosT;
            }
            out[off + 6] = verts[off + 6];
            out[off + 7] = verts[off + 7];
        }
        return out;
    }

    // --------------------------------------------------------
    // Mouse / wheel / keyboard handlers
    // --------------------------------------------------------

    _addListeners() {
        const dom = this.renderer.domElement;
        dom.addEventListener('pointerdown', this._onPointerDown);
        window.addEventListener('pointermove', this._onPointerMove);
        window.addEventListener('pointerup', this._onPointerUp);
        dom.addEventListener('wheel', this._onWheel, { passive: false });
        window.addEventListener('keydown', this._onKeyDown);
    }

    _removeListeners() {
        const dom = this.renderer.domElement;
        dom.removeEventListener('pointerdown', this._onPointerDown);
        window.removeEventListener('pointermove', this._onPointerMove);
        window.removeEventListener('pointerup', this._onPointerUp);
        dom.removeEventListener('wheel', this._onWheel);
        window.removeEventListener('keydown', this._onKeyDown);
    }

    _handlePointerDown(e) {
        if (!this.active) return;

        // Raycast to check if user clicked the bend handle or pivot sphere
        const raycaster = new THREE.Raycaster();
        const rect = this.renderer.domElement.getBoundingClientRect();
        const mouse = new THREE.Vector2(
            ((e.clientX - rect.left) / rect.width) * 2 - 1,
            -((e.clientY - rect.top) / rect.height) * 2 + 1
        );
        raycaster.setFromCamera(mouse, this.camera);

        const targets = [this.dragHandle, this.pivotSphere].filter(Boolean);
        const hits = raycaster.intersectObjects(targets);

        if (hits.length > 0) {
            // Disable orbit controls while dragging
            if (this.orbitControls) this.orbitControls.enabled = false;
            this.dragging = true;
            this.dragStart = { x: e.clientX, y: e.clientY };
            this.dragStartAngle = this.angleDeg;

            // Track which element was grabbed
            this.dragTarget = hits[0].object === this.pivotSphere ? 'pivot' : 'angle';

            // For pivot dragging, set up a raycast plane perpendicular to camera
            // passing through the pivot sphere center
            if (this.dragTarget === 'pivot') {
                this.raycastPlane = new THREE.Plane();
                const camDir = new THREE.Vector3();
                this.camera.getWorldDirection(camDir);
                this.raycastPlane.setFromNormalAndCoplanarPoint(camDir.negate(), this.pivotSphere.position);
            }
            e.preventDefault();
        }
    }

    _handlePointerMove(e) {
        if (!this.active || !this.dragging) return;

        if (this.dragTarget === 'angle') {
            // Map vertical mouse drag to angle: 1 pixel ≈ 0.5°
            const dy = e.clientY - this.dragStart.y;
            this.angleDeg = this.dragStartAngle + dy * 0.5;
            // Clamp to ±360°
            this.angleDeg = Math.max(-360, Math.min(360, this.angleDeg));

        } else if (this.dragTarget === 'pivot') {
            // Raycast mouse onto plane, project onto bend axis
            const raycaster = new THREE.Raycaster();
            const rect = this.renderer.domElement.getBoundingClientRect();
            const mouse = new THREE.Vector2(
                ((e.clientX - rect.left) / rect.width) * 2 - 1,
                -((e.clientY - rect.top) / rect.height) * 2 + 1
            );
            raycaster.setFromCamera(mouse, this.camera);

            const hit = new THREE.Vector3();
            if (raycaster.ray.intersectPlane(this.raycastPlane, hit)) {
                // Get bounds along bend axis
                const bounds = this._getBounds();
                const bendMin = [bounds.minX, bounds.minY, bounds.minZ][this.bendAxis];
                const bendMax = [bounds.maxX, bounds.maxY, bounds.maxZ][this.bendAxis];
                const bendLength = bendMax - bendMin;
                if (bendLength < 0.001) return;

                const mouseBendCoord = [hit.x, hit.y, hit.z][this.bendAxis];
                this.pivotOffset = (mouseBendCoord - bendMin) / bendLength;
                this.pivotOffset = Math.max(0, Math.min(1, this.pivotOffset));
            }
        }

        this._applyBend();
    }

    _handlePointerUp(e) {
        if (!this.active) return;
        this.dragging = false;
        this.dragTarget = null;
        if (this.orbitControls) this.orbitControls.enabled = true;
    }

    _handleWheel(e) {
        if (!this.active) return;
        e.preventDefault();
        // Scroll wheel adjusts pivot offset (0.0–1.0)
        const delta = e.deltaY > 0 ? 0.02 : -0.02;
        this.pivotOffset = Math.max(0, Math.min(1, this.pivotOffset + delta));
        this._applyBend();
    }

    _handleKeyDown(e) {
        if (!this.active) return;
        switch (e.key) {
            case 'Escape':
                this.cancel();
                break;
            case 'Enter':
                this.detach();
                break;
            case 'b':
            case 'B':
                this.detach(); // B toggles off
                break;
            case 'x': case 'X':
                this.bendAxis = 0; this.curveDir = 1; this._rebuildGizmo();
                break;
            case 'y': case 'Y':
                this.bendAxis = 1; this.curveDir = 2; this._rebuildGizmo();
                break;
            case 'z': case 'Z':
                this.bendAxis = 2; this.curveDir = 1; this._rebuildGizmo();
                break;
        }
    }

    // --------------------------------------------------------
    // Public API for external UI (sliders, buttons)
    // --------------------------------------------------------

    setAngle(deg) {
        this.angleDeg = Math.max(-360, Math.min(360, deg));
        this._applyBend();
    }

    setPivot(offset) {
        this.pivotOffset = Math.max(0, Math.min(1, offset));
        this._applyBend();
    }

    setBendAxis(axis) {
        this.bendAxis = axis;
        if (axis === this.curveDir) this.curveDir = (axis + 1) % 3;
        this._rebuildGizmo();
    }

    setCurveDir(dir) {
        this.curveDir = dir;
        if (dir === this.bendAxis) this.bendAxis = (dir + 1) % 3;
        this._rebuildGizmo();
    }

    isActive() { return this.active; }
    getAngle() { return this.angleDeg; }
    getPivot() { return this.pivotOffset; }
}
