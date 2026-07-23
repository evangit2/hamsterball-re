// ============================================================
// BendTool v9 — Simple slider-based bend. One mode, one slider.
// Proven math, proven overlay pattern. Nothing else.
// ============================================================

var BendTool = (function() {

    var S = {
        active: false, item: null, hbMesh: null,
        originalVerts: null, cb: null,
        bendAxis: 2, curveDir: 1,
        angleDeg: 0, pivotOffset: 0,
        overlay: null, panel: null,
    };

    // ── Bend math (verified across all versions) ──

    function bendVertices(verts, n, p) {
        var ba = p.bendAxis, cd = p.curveDir;
        if (ba === cd) return verts.slice();
        var pa = 3 - ba - cd;
        var mn = Infinity, mx = -Infinity;
        for (var i = 0; i < n; i++) {
            var v = verts[i*8+ba];
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
        var len = mx - mn;
        if (len < 0.001) return verts.slice();
        var piv = mn + len * (p.pivotOffset || 0);
        var ta = p.angleDeg * Math.PI / 180;
        if (Math.abs(ta) < 0.001) return verts.slice();
        var R = len / ta;
        var out = new Float32Array(verts.length);
        for (var i = 0; i < n; i++) {
            var o = i * 8;
            var bc = verts[o+ba], cc = verts[o+cd], pc = verts[o+pa];
            var nb = verts[o+3+ba], nc = verts[o+3+cd], np = verts[o+3+pa];
            var d = bc - piv;
            var th = (d / len) * ta;
            var ct = Math.cos(th), st = Math.sin(th);
            var r = R - cc;
            out[o+pa]   = pc;
            out[o+cd]   = R - r * ct;
            out[o+ba]   = piv + r * st;
            out[o+3+pa] = np;
            out[o+3+cd] = nc * ct - nb * st;
            out[o+3+ba] = nc * st + nb * ct;
            out[o+6] = verts[o+6];
            out[o+7] = verts[o+7];
        }
        return out;
    }

    // ── Apply + refresh ──

    function refresh() {
        if (S.cb && S.cb.refreshFn) {
            try { S.cb.refreshFn(S.item); } catch(e) {}
        }
        // Re-apply bent normals after refreshFn (which may call computeVertexNormals
        // and overwrite our correctly-transformed normals with averaged smooth normals).
        // The game uses normals as-is from the file — no recomputation — so the
        // bent normals from bendVertices() are the correct ones to use.
        if (S.hbMesh && S.item) {
            var mesh = S.item.mesh || S.item.threeMesh || S.item.object;
            if (mesh && mesh.geometry && mesh.geometry.attributes && mesh.geometry.attributes.normal) {
                var na = mesh.geometry.attributes.normal.array;
                var v = S.hbMesh.vertices;
                var n = v.length / 8;
                for (var i = 0; i < n; i++) {
                    na[i*3]   = v[i*8+3];
                    na[i*3+1] = v[i*8+4];
                    na[i*3+2] = v[i*8+5];
                }
                mesh.geometry.attributes.normal.needsUpdate = true;
            }
        }
    }

    function applyBend() {
        if (!S.originalVerts) return;
        var n = S.originalVerts.length / 8;
        S.hbMesh.vertices = bendVertices(S.originalVerts, n, {
            bendAxis: S.bendAxis,
            curveDir: S.curveDir,
            angleDeg: S.angleDeg,
            pivotOffset: S.pivotOffset,
        });
        refresh();
    }

    // ── Auto-detect best axis ──

    function autoAxis(v) {
        var n = v.length / 8;
        var mX=Infinity,MX=-Infinity,mY=Infinity,MY=-Infinity,mZ=Infinity,MZ=-Infinity;
        for (var i = 0; i < n; i++) {
            var o = i * 8;
            if(v[o]<mX)mX=v[o]; if(v[o]>MX)MX=v[o];
            if(v[o+1]<mY)mY=v[o+1]; if(v[o+1]>MY)MY=v[o+1];
            if(v[o+2]<mZ)mZ=v[o+2]; if(v[o+2]>MZ)MZ=v[o+2];
        }
        var dx=MX-mX, dy=MY-mY, dz=MZ-mZ;
        if (dz >= dx && dz >= dy) return {ba:2, cd:1};
        if (dx >= dy) return {ba:0, cd:1};
        return {ba:1, cd:2};
    }

    // ── UI ──

    function createOverlay() {
        var ov = document.createElement('div');
        ov.id = 'bendtool-overlay';
        ov.style.cssText = 'position:absolute;top:0;left:0;width:100%;height:100%;z-index:1000;background:rgba(0,0,0,0.01);';

        var p = document.createElement('div');
        p.style.cssText = [
            'position:absolute','top:10px','right:10px',
            'background:rgba(30,30,40,0.95)','color:#e0e0e0','padding:14px',
            'border-radius:8px','font-family:monospace','font-size:13px',
            'min-width:220px','box-shadow:0 4px 12px rgba(0,0,0,0.5)',
            'user-select:none',
        ].join(';');

        p.innerHTML = [
            '<div style="font-weight:bold;font-size:14px;margin-bottom:10px;">🌀 Bend Tool</div>',

            '<div style="margin-bottom:8px;">',
            '  <div style="margin-bottom:4px;">Bend Axis</div>',
            '  <select id="bt-axis" style="width:100%;background:#222;color:#ddd;border:1px solid #555;padding:4px;border-radius:4px;">',
            '    <option value="0">X</option>',
            '    <option value="1">Y</option>',
            '    <option value="2" selected>Z</option>',
            '  </select>',
            '</div>',

            '<div style="margin-bottom:8px;">',
            '  <div style="margin-bottom:4px;">Curve Direction</div>',
            '  <select id="bt-curve" style="width:100%;background:#222;color:#ddd;border:1px solid #555;padding:4px;border-radius:4px;">',
            '    <option value="0">X</option>',
            '    <option value="1" selected>Y (up)</option>',
            '    <option value="2">Z</option>',
            '  </select>',
            '</div>',

            '<div style="margin-bottom:8px;">',
            '  <div style="margin-bottom:4px;">Bend Amount: <span id="bt-av" style="color:#8c8;">0°</span></div>',
            '  <input type="range" id="bt-angle" min="-360" max="360" value="0" step="1" style="width:100%;">',
            '</div>',

            '<div style="margin-bottom:10px;">',
            '  <div style="margin-bottom:4px;">Pivot: <span id="bt-pv" style="color:#8c8;">0%</span></div>',
            '  <input type="range" id="bt-pivot" min="0" max="1" value="0" step="0.01" style="width:100%;">',
            '</div>',

            '<div style="display:flex;gap:4px;">',
            '  <button id="bt-apply" style="flex:1;background:#2d5a2d;color:#fff;border:none;padding:6px;border-radius:4px;cursor:pointer;">✅ Apply</button>',
            '  <button id="bt-reset" style="flex:1;background:#555;color:#fff;border:none;padding:6px;border-radius:4px;cursor:pointer;">🔄 Reset</button>',
            '  <button id="bt-cancel" style="flex:1;background:#5a2d2d;color:#fff;border:none;padding:6px;border-radius:4px;cursor:pointer;">❌ Cancel</button>',
            '</div>',
        ].join('');

        ov.appendChild(p);
        document.body.appendChild(ov);
        S.overlay = ov;
        S.panel = p;

        // Wire controls
        p.querySelector('#bt-axis').onchange = function() {
            S.bendAxis = parseInt(this.value);
            applyBend();
        };
        p.querySelector('#bt-curve').onchange = function() {
            S.curveDir = parseInt(this.value);
            applyBend();
        };
        p.querySelector('#bt-angle').oninput = function() {
            S.angleDeg = parseFloat(this.value);
            p.querySelector('#bt-av').textContent = S.angleDeg.toFixed(0) + '°';
            applyBend();
        };
        p.querySelector('#bt-pivot').oninput = function() {
            S.pivotOffset = parseFloat(this.value);
            p.querySelector('#bt-pv').textContent = (S.pivotOffset * 100).toFixed(0) + '%';
            applyBend();
        };

        // Buttons
        p.querySelector('#bt-apply').onclick = function() { exitBendMode(true); };
        p.querySelector('#bt-reset').onclick = function() {
            S.angleDeg = 0;
            S.pivotOffset = 0;
            p.querySelector('#bt-angle').value = 0;
            p.querySelector('#bt-pivot').value = 0;
            p.querySelector('#bt-av').textContent = '0°';
            p.querySelector('#bt-pv').textContent = '0%';
            applyBend();
        };
        p.querySelector('#bt-cancel').onclick = function() { exitBendMode(false); };
    }

    // ── Enter / Exit ──

    function enterBendMode(item, callbacks) {
        if (!item || !item.hbMesh) return;
        if (S.active) exitBendMode(false);

        S.item = item;
        S.hbMesh = item.hbMesh;
        S.originalVerts = new Float32Array(item.hbMesh.vertices);
        S.cb = callbacks || {};
        S.angleDeg = 0;
        S.pivotOffset = 0;

        var det = autoAxis(S.originalVerts);
        S.bendAxis = det.ba;
        S.curveDir = det.cd;

        S.active = true;
        createOverlay();
    }

    function exitBendMode(commit) {
        try {
            if (!S.active) return;
            if (!commit && S.originalVerts && S.hbMesh) {
                S.hbMesh.vertices = new Float32Array(S.originalVerts);
                refresh();
            }
        } finally {
            if (S.overlay && S.overlay.parentNode) {
                S.overlay.parentNode.removeChild(S.overlay);
            }
            S.active = false;
            S.overlay = null;
            S.panel = null;
            S.item = null;
            S.hbMesh = null;
            S.originalVerts = null;
        }
    }

    return {
        bendVertices: bendVertices,
        enterBendMode: enterBendMode,
        exitBendMode: exitBendMode,
        isActive: function() { return S.active; },
    };
})();
