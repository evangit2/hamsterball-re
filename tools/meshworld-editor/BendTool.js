// ============================================================
// BendTool v8 — Three modes: Blender Bend, Multi-Point Curve, Axis Bend
// Screen-space vertex picking (NO raycaster). Visual markers.
// DOM overlay. No keyboard hooks. Pure editor-side.
// ============================================================

var BendTool = (function() {

    var S = {
        active: false, item: null, hbMesh: null,
        originalVerts: null, cb: null,
        bendAxis: 2, curveDir: 1, angleDeg: 0, pivotOffset: 0,
        // mode: 'blender' | 'multi' | 'axis'
        mode: 'blender',
        // blender bend
        anchorVert: -1, anchorScreen: null,
        anchorMarker: null,
        // multi-point
        ctrlPoints: [], selectedPoint: -1,
        markers: [], markerParent: null,
        // interaction
        dragMode: null, // null | 'angle' | 'marker'
        axStartX: 0, axStartY: 0, axStartAngle: 0, dragMarkerIdx: -1,
        // dom
        overlay: null, panel: null,
    };

    // ── Original single-angle bend (UNCHANGED) ──

    function bendVertices(verts, n, p) {
        var ba = p.bendAxis, cd = p.curveDir;
        if (ba === cd) return verts.slice();
        var pa = 3 - ba - cd;
        var mn = Infinity, mx = -Infinity;
        for (var i = 0; i < n; i++) { var v = verts[i*8+ba]; if(v<mn)mn=v; if(v>mx)mx=v; }
        var len = mx - mn; if (len < 0.001) return verts.slice();
        var piv = mn + len * (p.pivotOffset || 0);
        var ta = p.angleDeg * Math.PI / 180;
        if (Math.abs(ta) < 0.001) return verts.slice();
        var R = len / ta;
        var out = new Float32Array(verts.length);
        for (var i = 0; i < n; i++) {
            var o = i*8, bc=verts[o+ba], cc=verts[o+cd], pc=verts[o+pa];
            var nb=verts[o+3+ba], nc=verts[o+3+cd], np=verts[o+3+pa];
            var d = bc - piv, th = (d/len)*ta, ct=Math.cos(th), st=Math.sin(th);
            var r = R - cc;
            out[o+pa]=pc; out[o+cd]=R-r*ct; out[o+ba]=piv+r*st;
            out[o+3+pa]=np; out[o+3+cd]=nc*ct-nb*st; out[o+3+ba]=nc*st+nb*ct;
            out[o+6]=verts[o+6]; out[o+7]=verts[o+7];
        }
        return out;
    }

    // ── Multi-point arc integration (UNCHANGED from v7) ──

    function computeArc(t, pts, L) {
        var x=0, y=0, theta=pts.length>0?pts[0].angleRad:0;
        var segs=[];
        segs.push({s0:0,a0:pts[0].angleRad,s1:pts[0].pos,a1:pts[0].angleRad});
        for (var i=0; i<pts.length-1; i++)
            segs.push({s0:pts[i].pos,a0:pts[i].angleRad,s1:pts[i+1].pos,a1:pts[i+1].angleRad});
        var last=pts[pts.length-1];
        segs.push({s0:last.pos,a0:last.angleRad,s1:1.0,a1:last.angleRad});
        for (var i=0; i<segs.length; i++) {
            var seg=segs[i]; if (t<=seg.s0) break;
            var segEnd=Math.min(t,seg.s1), dS=(segEnd-seg.s0)*L;
            var denom=seg.s1-seg.s0, frac=denom>0.0001?(segEnd-seg.s0)/denom:1;
            var thEnd=seg.a0+frac*(seg.a1-seg.a0), dTh=thEnd-seg.a0;
            if (Math.abs(dTh)<0.001) { x+=dS*Math.sin(seg.a0); y+=dS*Math.cos(seg.a0); }
            else { var R=dS/dTh; x+=R*(Math.cos(seg.a0)-Math.cos(thEnd)); y+=R*(Math.sin(thEnd)-Math.sin(seg.a0)); }
            theta=thEnd; if (t<=seg.s1) break;
        }
        return {x:x, y:y, theta:theta};
    }

    function bendVerticesMulti(verts, n, p) {
        var ba=p.bendAxis, cd=p.curveDir; if (ba===cd) return verts.slice();
        var pa=3-ba-cd, mn=Infinity, mx=-Infinity;
        for (var i=0; i<n; i++) { var v=verts[i*8+ba]; if(v<mn)mn=v; if(v>mx)mx=v; }
        var len=mx-mn; if (len<0.001) return verts.slice();
        var piv=mn+len*(p.pivotOffset||0);
        var pts=p.points.slice().sort(function(a,b){return a.pos-b.pos;});
        for (var i=0; i<pts.length; i++) pts[i]={pos:pts[i].pos, angleRad:pts[i].angle*Math.PI/180};
        var out=new Float32Array(verts.length);
        for (var i=0; i<n; i++) {
            var o=i*8, bc=verts[o+ba], cc=verts[o+cd], pc=verts[o+pa];
            var nb=verts[o+3+ba], nc=verts[o+3+cd], np=verts[o+3+pa];
            var d=bc-piv, t=d/len; if(t<0)t=0; if(t>1)t=1;
            var arc=computeArc(t, pts, len), th=arc.theta, ct=Math.cos(th), st=Math.sin(th);
            out[o+pa]=pc; out[o+cd]=arc.x+cc*ct; out[o+ba]=piv+arc.y-cc*st;
            out[o+3+pa]=np; out[o+3+cd]=nc*ct-nb*st; out[o+3+ba]=nc*st+nb*ct;
            out[o+6]=verts[o+6]; out[o+7]=verts[o+7];
        }
        return out;
    }

    // ── Apply ──

    function refresh() {
        if (S.cb && S.cb.refreshFn) { try { S.cb.refreshFn(S.item); } catch(e){} }
    }

    function applyBend() {
        if (!S.originalVerts) return;
        var n = S.originalVerts.length / 8;
        if (S.mode === 'multi' && S.ctrlPoints.length > 0) {
            S.hbMesh.vertices = bendVerticesMulti(S.originalVerts, n, {
                bendAxis:S.bendAxis, curveDir:S.curveDir, pivotOffset:S.pivotOffset, points:S.ctrlPoints,
            });
        } else {
            // Both 'blender' and 'axis' modes use single-angle bend
            S.hbMesh.vertices = bendVertices(S.originalVerts, n, {
                bendAxis:S.bendAxis, curveDir:S.curveDir, angleDeg:S.angleDeg, pivotOffset:S.pivotOffset,
            });
        }
        if (S.mode === 'multi') updateMarkerPositions();
        if (S.mode === 'blender') updateAnchorMarker();
        refresh();
    }

    // ── Screen-space projection ──

    function getCanvasRect() { return S.cb.renderer.domElement.getBoundingClientRect(); }

    function getMeshMatrix() {
        if (!S.cb.getMeshObject) return null;
        var m = S.cb.getMeshObject(S.item);
        if (!m) return null;
        m.updateMatrixWorld(true);
        return m.matrixWorld || null;
    }

    function projectVertex(vx, vy, vz, meshMatrix, rect) {
        var v = new THREE.Vector3(vx, vy, vz);
        if (meshMatrix) v.applyMatrix4(meshMatrix);
        v.project(S.cb.camera);
        return { x:(v.x*0.5+0.5)*rect.width+rect.left, y:(-v.y*0.5+0.5)*rect.height+rect.top, z:v.z };
    }

    function findVertexAtScreen(mx, my) {
        var rect = getCanvasRect(), meshMatrix = getMeshMatrix();
        var v = S.originalVerts, n = v.length/8, best=-1, bestD=40*40;
        for (var i=0; i<n; i++) {
            var o=i*8, sp=projectVertex(v[o],v[o+1],v[o+2],meshMatrix,rect);
            var dx=sp.x-mx, dy=sp.y-my, d2=dx*dx+dy*dy;
            if (d2<bestD) { bestD=d2; best=i; }
        }
        return best;
    }

    function findMarkerAtScreen(mx, my) {
        if (S.markers.length===0) return -1;
        var rect=getCanvasRect(), best=-1, bestD=25*25;
        for (var i=0; i<S.ctrlPoints.length; i++) {
            if (!S.markers[i]) continue;
            var wp=new THREE.Vector3();
            S.markers[i].getWorldPosition(wp);
            var sp=projectVertex(wp.x,wp.y,wp.z,null,rect);
            var dx=sp.x-mx, dy=sp.y-my, d2=dx*dx+dy*dy;
            if (d2<bestD) { bestD=d2; best=i; }
        }
        return best;
    }

    function findAnchorAtScreen(mx, my) {
        if (!S.anchorScreen) return false;
        var dx=S.anchorScreen.x-mx, dy=S.anchorScreen.y-my;
        return (dx*dx+dy*dy) < 25*25;
    }

    // ── Anchor marker (Blender mode) ──

    function getMarkerParent() {
        if (S.cb.getMeshObject) { var m=S.cb.getMeshObject(S.item); if(m&&m.parent) return m.parent; }
        return S.cb.scene;
    }

    function createAnchorMarker(localPos) {
        var geo=new THREE.SphereGeometry(8,12,10);
        var mat=new THREE.MeshBasicMaterial({color:0xff3333,depthTest:true,transparent:true,opacity:0.95});
        var mesh=new THREE.Mesh(geo,mat);
        mesh.position.copy(localPos);
        mesh.userData.isBendMarker=true;
        mesh.renderOrder=999;
        return mesh;
    }

    function showAnchorMarker(vertIdx) {
        removeAnchorMarker();
        var o=vertIdx*8, v=S.originalVerts;
        var pos=new THREE.Vector3(v[o],v[o+1],v[o+2]);
        S.anchorMarker=createAnchorMarker(pos);
        var parent=getMarkerParent();
        if (parent) parent.add(S.anchorMarker);
        // Record screen position
        var rect=getCanvasRect(), meshMatrix=getMeshMatrix();
        S.anchorScreen=projectVertex(v[o],v[o+1],v[o+2],meshMatrix,rect);
    }

    function updateAnchorMarker() {
        if (!S.anchorMarker || S.anchorVert<0) return;
        var o=S.anchorVert*8, v=S.hbMesh.vertices; // use bent vertices
        S.anchorMarker.position.set(v[o],v[o+1],v[o+2]);
    }

    function removeAnchorMarker() {
        if (S.anchorMarker) {
            if (S.anchorMarker.parent) S.anchorMarker.parent.remove(S.anchorMarker);
            S.anchorMarker.geometry.dispose();
            S.anchorMarker.material.dispose();
            S.anchorMarker=null;
        }
        S.anchorScreen=null;
    }

    // ── Multi-point markers ──

    function createSphereMarker(selected) {
        var geo=new THREE.SphereGeometry(6,10,8);
        var mat=new THREE.MeshBasicMaterial({color:selected?0x00ff00:0xffff00,depthTest:true,transparent:true,opacity:0.9});
        var mesh=new THREE.Mesh(geo,mat);
        mesh.userData.isBendMarker=true;
        mesh.renderOrder=999;
        return mesh;
    }

    function rebuildMarkers() {
        for (var i=0; i<S.markers.length; i++) {
            if (S.markers[i]&&S.markers[i].parent) S.markers[i].parent.remove(S.markers[i]);
            if (S.markers[i]) { S.markers[i].geometry.dispose(); S.markers[i].material.dispose(); }
        }
        S.markers=[]; S.markerParent=getMarkerParent();
        if (!S.markerParent) return;
        for (var i=0; i<S.ctrlPoints.length; i++) {
            var pos=getPointWorldPos(S.ctrlPoints[i]);
            var m=createSphereMarker(i===S.selectedPoint);
            m.position.copy(pos);
            S.markerParent.add(m); S.markers.push(m);
        }
    }

    function getPointWorldPos(cp) {
        if (cp.vertIdx>=0) {
            var o=cp.vertIdx*8, v=S.hbMesh.vertices;
            return new THREE.Vector3(v[o],v[o+1],v[o+2]);
        }
        return new THREE.Vector3(0,0,0);
    }

    function updateMarkerPositions() {
        for (var i=0; i<S.ctrlPoints.length; i++) {
            if (!S.markers[i]) continue;
            var pos=getPointWorldPos(S.ctrlPoints[i]);
            S.markers[i].position.copy(pos);
            S.markers[i].material.color.setHex(i===S.selectedPoint?0x00ff00:0xffff00);
            S.markers[i].scale.setScalar(i===S.selectedPoint?1.5:1.0);
        }
    }

    function disposeMarkers() {
        for (var i=0; i<S.markers.length; i++) {
            if (S.markers[i]&&S.markers[i].parent) S.markers[i].parent.remove(S.markers[i]);
            if (S.markers[i]) { S.markers[i].geometry.dispose(); S.markers[i].material.dispose(); }
        }
        S.markers=[]; S.markerParent=null;
    }

    // ── Bounds / auto-detect ──

    function getBounds(v) {
        var n=v.length/8, b={mX:Infinity,MX:-Infinity,mY:Infinity,MY:-Infinity,mZ:Infinity,MZ:-Infinity};
        for(var i=0;i<n;i++){var o=i*8;
            if(v[o]<b.mX)b.mX=v[o];if(v[o]>b.MX)b.MX=v[o];
            if(v[o+1]<b.mY)b.mY=v[o+1];if(v[o+1]>b.MY)b.MY=v[o+1];
            if(v[o+2]<b.mZ)b.mZ=v[o+2];if(v[o+2]>b.MZ)b.MZ=v[o+2];}
        return b;
    }

    function autoAxis(v) {
        var b=getBounds(v), dx=b.MX-b.mX, dy=b.MY-b.mY, dz=b.MZ-b.mZ;
        if (dz>=dx&&dz>=dy) return {ba:2,cd:1};
        if (dx>=dy) return {ba:0,cd:1};
        return {ba:1,cd:2};
    }

    function getAxisBounds(v, ba) {
        var b=getBounds(v), mn, mx;
        if (ba===0){mn=b.mX;mx=b.MX;} else if(ba===1){mn=b.mY;mx=b.MY;} else {mn=b.mZ;mx=b.MZ;}
        return {mn:mn, mx:mx, len:mx-mn};
    }

    // ── Blender mode: anchor + drag ──

    function setAnchor(vertIdx) {
        S.anchorVert = vertIdx;
        var v = S.originalVerts;
        var ab = getAxisBounds(v, S.bendAxis);
        S.pivotOffset = (v[vertIdx*8+S.bendAxis] - ab.mn) / ab.len;
        showAnchorMarker(vertIdx);
        // Update pivot slider
        var ps = S.panel.querySelector('#bt-ps');
        if (ps) ps.value = S.pivotOffset;
        var pv = S.panel.querySelector('#bt-pv');
        if (pv) pv.textContent = S.pivotOffset.toFixed(2);
        S.angleDeg = 0;
        var as = S.panel.querySelector('#bt-as');
        if (as) as.value = 0;
        var av = S.panel.querySelector('#bt-av');
        if (av) av.textContent = '0°';
        applyBend();
    }

    // ── Multi-point management ──

    function addPointAtVertex(vertIdx) {
        var v=S.originalVerts, ab=getAxisBounds(v,S.bendAxis);
        if (ab.len<0.001) return;
        var pos=(v[vertIdx*8+S.bendAxis]-ab.mn)/ab.len;
        if(pos<0)pos=0; if(pos>1)pos=1;
        var angle=0;
        if (S.ctrlPoints.length>0) {
            var sorted=S.ctrlPoints.slice().sort(function(a,b){return a.pos-b.pos;});
            if (pos<=sorted[0].pos) angle=sorted[0].angle;
            else if (pos>=sorted[sorted.length-1].pos) angle=sorted[sorted.length-1].angle;
            else { for (var i=0; i<sorted.length-1; i++) {
                if(pos>=sorted[i].pos&&pos<=sorted[i+1].pos){
                    var f=(pos-sorted[i].pos)/(sorted[i+1].pos-sorted[i].pos);
                    angle=sorted[i].angle+f*(sorted[i+1].angle-sorted[i].angle); break; } } }
        } else { angle = S.angleDeg; }
        S.ctrlPoints.push({pos:pos,angle:angle,vertIdx:vertIdx});
        S.selectedPoint=S.ctrlPoints.length-1;
        rebuildMarkers(); refreshPointsUI(); applyBend();
    }

    function addPointMidpoint() {
        if (S.ctrlPoints.length===0) {
            var v=S.originalVerts, n=v.length/8, ab=getAxisBounds(v,S.bendAxis);
            var midCoord=(ab.mn+ab.mx)/2, best=-1, bestD=Infinity;
            for(var i=0;i<n;i++){var o=i*8; var d=Math.abs(v[o+S.bendAxis]-midCoord); if(d<bestD){bestD=d;best=i;}}
            if(best>=0){addPointAtVertex(best);return;}
        }
        var pts=S.ctrlPoints.slice().sort(function(a,b){return a.pos-b.pos;}), maxGap=0,gapStart=0;
        if(pts[0].pos>maxGap){maxGap=pts[0].pos;gapStart=0;}
        for(var i=0;i<pts.length-1;i++){var g=pts[i+1].pos-pts[i].pos; if(g>maxGap){maxGap=g;gapStart=pts[i].pos;}}
        var lg=1-pts[pts.length-1].pos; if(lg>maxGap){maxGap=lg;gapStart=pts[pts.length-1].pos;}
        var newPos=gapStart+maxGap/2;
        var v=S.originalVerts, ab=getAxisBounds(v,S.bendAxis);
        var tc=ab.mn+newPos*ab.len, best=-1, bestD=Infinity;
        for(var i=0;i<v.length/8;i++){var o=i*8; var d=Math.abs(v[o+S.bendAxis]-tc); if(d<bestD){bestD=d;best=i;}}
        if(best>=0) addPointAtVertex(best);
    }

    function deletePoint(idx) {
        if(idx<0||idx>=S.ctrlPoints.length) return;
        S.ctrlPoints.splice(idx,1);
        if(S.selectedPoint===idx)S.selectedPoint=-1; else if(S.selectedPoint>idx)S.selectedPoint--;
        rebuildMarkers(); refreshPointsUI(); applyBend();
    }

    function clearPoints() {
        S.ctrlPoints=[]; S.selectedPoint=-1;
        rebuildMarkers(); refreshPointsUI(); applyBend();
    }

    function selectPoint(idx) {
        S.selectedPoint=idx; refreshPointsUI(); updateMarkerPositions(); refresh();
    }

    function updatePointFromScreen(idx, mx, my) {
        var vi=findVertexAtScreen(mx,my); if(vi<0) return;
        var v=S.originalVerts, ab=getAxisBounds(v,S.bendAxis);
        var newPos=(v[vi*8+S.bendAxis]-ab.mn)/ab.len;
        if(newPos<0)newPos=0; if(newPos>1)newPos=1;
        S.ctrlPoints[idx].pos=newPos; S.ctrlPoints[idx].vertIdx=vi;
        refreshPointsUI(); applyBend();
    }

    // ── Mode switch ──

    function setMode(m) {
        S.mode = m;
        // Clear mode-specific markers
        if (m !== 'multi') { disposeMarkers(); S.ctrlPoints=[]; S.selectedPoint=-1; }
        if (m !== 'blender') { removeAnchorMarker(); S.anchorVert=-1; }
        if (m === 'multi') { S.ctrlPoints=[]; }
        if (m === 'blender') { S.angleDeg=0; S.pivotOffset=0; }

        var p = S.panel;
        // Show/hide mode panels
        var blDiv = p.querySelector('#bt-blender-ctrl');
        var mpDiv = p.querySelector('#bt-multi-ctrl');
        var axDiv = p.querySelector('#bt-axis-ctrl');
        var hint = p.querySelector('#bt-hint');

        if (blDiv) blDiv.style.display = m === 'blender' ? 'block' : 'none';
        if (mpDiv) mpDiv.style.display = m === 'multi' ? 'block' : 'none';
        if (axDiv) axDiv.style.display = m === 'axis' ? 'block' : 'none';

        // Mode button highlight
        var buttons = ['bt-mode-blender','bt-mode-multi','bt-mode-axis'];
        for (var i = 0; i < buttons.length; i++) {
            var btn = p.querySelector('#' + buttons[i]);
            if (btn) btn.style.background = '#444';
        }
        var activeBtn = p.querySelector('#bt-mode-' + m);
        if (activeBtn) activeBtn.style.background = '#2d5a2d';

        if (hint) {
            if (m === 'blender') hint.innerHTML = 'Click object: set anchor<br>Drag: bend angle';
            else if (m === 'multi') hint.innerHTML = 'Click object: add point<br>Drag marker: move<br>Drag canvas: angle';
            else hint.innerHTML = 'Drag canvas: angle<br>Scroll wheel: pivot';
        }

        S.angleDeg = 0; S.pivotOffset = 0;
        var as = p.querySelector('#bt-as'); if (as) as.value = 0;
        var ps = p.querySelector('#bt-ps'); if (ps) ps.value = 0;
        var av = p.querySelector('#bt-av'); if (av) av.textContent = '0°';
        var pv = p.querySelector('#bt-pv'); if (pv) pv.textContent = '0.00';

        S.hbMesh.vertices = new Float32Array(S.originalVerts);
        refreshPointsUI();
        applyBend();
    }

    // ── UI ──

    function refreshPointsUI() {
        var p=S.panel; if(!p) return;
        var listDiv=p.querySelector('#bt-points-list'); if(!listDiv) return;
        var html='';
        if(S.ctrlPoints.length===0){
            html='<div style="color:#666;font-size:11px;margin:4px 0;">No points. Click object or Add.</div>';
        } else {
            var sorted=S.ctrlPoints.map(function(cp,i){return{cp:cp,idx:i};}).sort(function(a,b){return a.cp.pos-b.cp.pos;});
            for(var i=0;i<sorted.length;i++){
                var e=sorted[i],sel=e.idx===S.selectedPoint;
                html+='<div data-pidx="'+e.idx+'" style="'+(sel?'background:#2d5a2d;':'background:#333;')+'padding:4px 6px;margin:2px 0;border-radius:4px;cursor:pointer;display:flex;justify-content:space-between;align-items:center;">';
                html+='<span>t='+(e.cp.pos*100).toFixed(0)+'% '+e.cp.angle.toFixed(0)+'°</span>';
                html+='<button data-del="'+e.idx+'" style="background:#5a2d2d;color:#fff;border:none;padding:2px 6px;border-radius:3px;cursor:pointer;font-size:11px;">✕</button></div>';
            }
        }
        listDiv.innerHTML=html;
        var rows=listDiv.querySelectorAll('[data-pidx]');
        for(var i=0;i<rows.length;i++){rows[i].onclick=function(e){if(e.target.tagName==='BUTTON')return;selectPoint(parseInt(this.getAttribute('data-pidx')));};}
        var dels=listDiv.querySelectorAll('[data-del]');
        for(var i=0;i<dels.length;i++){dels[i].onclick=function(e){e.stopPropagation();deletePoint(parseInt(this.getAttribute('data-del')));};}
        var selDiv=p.querySelector('#bt-sel-ctrl');
        if(S.selectedPoint>=0&&S.selectedPoint<S.ctrlPoints.length){
            var sp=S.ctrlPoints[S.selectedPoint];
            if(p.querySelector('#bt-sel-info'))p.querySelector('#bt-sel-info').textContent='t='+(sp.pos*100).toFixed(0)+'%';
            if(p.querySelector('#bt-spos'))p.querySelector('#bt-spos').value=sp.pos*100;
            if(p.querySelector('#bt-sposv'))p.querySelector('#bt-sposv').textContent=(sp.pos*100).toFixed(0)+'%';
            if(p.querySelector('#bt-sang'))p.querySelector('#bt-sang').value=sp.angle;
            if(p.querySelector('#bt-sangv'))p.querySelector('#bt-sangv').textContent=sp.angle.toFixed(0)+'°';
            if(selDiv)selDiv.style.display='block';
        } else { if(selDiv)selDiv.style.display='none'; }
    }

    function createOverlay() {
        var ov=document.createElement('div');
        ov.id='bendtool-overlay';
        ov.style.cssText='position:absolute;top:0;left:0;width:100%;height:100%;z-index:1000;cursor:crosshair;background:rgba(0,0,0,0.01);';

        var p=document.createElement('div');
        p.style.cssText='position:absolute;top:10px;right:10px;background:rgba(30,30,40,0.95);color:#e0e0e0;padding:12px;border-radius:8px;font-family:monospace;font-size:13px;min-width:260px;max-width:320px;box-shadow:0 4px 12px rgba(0,0,0,0.5);user-select:none;max-height:90vh;overflow-y:auto;';

        p.innerHTML=[
            '<div style="font-weight:bold;font-size:14px;margin-bottom:8px;">🔨 Bend Tool</div>',
            // Mode buttons
            '<div style="margin-bottom:8px;">',
            '<button id="bt-mode-blender" style="background:#2d5a2d;color:#fff;border:1px solid #555;padding:4px 8px;border-radius:4px;cursor:pointer;margin-right:4px;">🌀 Bend</button>',
            '<button id="bt-mode-multi" style="background:#444;color:#fff;border:1px solid #555;padding:4px 8px;border-radius:4px;cursor:pointer;margin-right:4px;">📊 Multi</button>',
            '<button id="bt-mode-axis" style="background:#444;color:#fff;border:1px solid #555;padding:4px 8px;border-radius:4px;cursor:pointer;">📐 Axis</button>',
            '</div>',
            // Common: axis/curve/pivot
            '<div style="margin-bottom:6px;">Axis: <select id="bt-axis" style="background:#222;color:#ddd;border:1px solid #555;padding:2px;"><option value="0">X</option><option value="1">Y</option><option value="2" selected>Z</option></select>',
            ' Curve: <select id="bt-curve" style="background:#222;color:#ddd;border:1px solid #555;padding:2px;"><option value="0">X</option><option value="1" selected>Y</option><option value="2">Z</option></select></div>',
            '<div style="margin-bottom:6px;">Pivot: <span id="bt-pv">0.00</span></div>',
            '<input type="range" id="bt-ps" min="0" max="1" value="0" step="0.01" style="width:100%;margin-bottom:8px;">',
            '<input type="range" id="bt-as" min="-360" max="360" value="0" step="1" style="width:100%;margin-bottom:8px;">',

            // Blender mode panel
            '<div id="bt-blender-ctrl" style="margin-bottom:8px;">',
            '  <div style="color:#8c8;margin-bottom:4px;">🔴 Click object to set anchor<br>Then drag to bend</div>',
            '  <div>Angle: <span id="bt-av">0°</span></div>',
            '</div>',

            // Multi mode panel
            '<div id="bt-multi-ctrl" style="display:none;margin-bottom:8px;">',
            '  <div style="border-top:1px solid #444;padding-top:8px;margin-bottom:4px;">',
            '    <div style="font-weight:bold;margin-bottom:4px;">Control Points</div>',
            '    <button id="bt-add" style="background:#2d4a2d;color:#fff;border:1px solid #555;padding:3px 8px;border-radius:4px;cursor:pointer;margin-right:4px;">+ Add</button>',
            '    <button id="bt-clear" style="background:#555;color:#fff;border:1px solid #555;padding:3px 8px;border-radius:4px;cursor:pointer;">Clear</button>',
            '    <div id="bt-points-list" style="margin-top:6px;"></div>',
            '  </div>',
            '  <div id="bt-sel-ctrl" style="display:none;border-top:1px solid #444;padding-top:8px;margin-bottom:4px;">',
            '    <div>Selected: <span id="bt-sel-info"></span></div>',
            '    <div style="margin-top:4px;">Position: <span id="bt-sposv">50%</span></div>',
            '    <input type="range" id="bt-spos" min="0" max="100" value="50" step="1" style="width:100%;margin-bottom:6px;">',
            '    <div>Angle: <span id="bt-sangv">0°</span></div>',
            '    <input type="range" id="bt-sang" min="-360" max="360" value="0" step="1" style="width:100%;">',
            '  </div>',
            '</div>',

            // Axis mode panel (empty — uses common angle/pivot sliders)
            '<div id="bt-axis-ctrl" style="display:none;margin-bottom:8px;">',
            '  <div>Angle: <span id="bt-av-axis">0°</span> (use slider above)</div>',
            '</div>',

            // Common buttons
            '<div style="border-top:1px solid #444;padding-top:8px;">',
            '  <button id="bt-apply" style="background:#2d5a2d;color:#fff;border:none;padding:6px 12px;border-radius:4px;cursor:pointer;margin-right:4px;">✅ Apply</button>',
            '  <button id="bt-reset" style="background:#555;color:#fff;border:none;padding:6px 12px;border-radius:4px;cursor:pointer;margin-right:4px;">🔄 Reset</button>',
            '  <button id="bt-cancel" style="background:#5a2d2d;color:#fff;border:none;padding:6px 12px;border-radius:4px;cursor:pointer;">❌ Cancel</button>',
            '</div>',
            '<div style="margin-top:8px;font-size:11px;color:#888;" id="bt-hint">Click object: set anchor<br>Drag: bend angle</div>',
        ].join('');

        ov.appendChild(p); document.body.appendChild(ov);
        S.overlay=ov; S.panel=p;

        // Common controls
        p.querySelector('#bt-axis').onchange=function(){S.bendAxis=+this.value; if(S.mode==='multi')rebuildMarkers(); applyBend();};
        p.querySelector('#bt-curve').onchange=function(){S.curveDir=+this.value; applyBend();};
        var ps=p.querySelector('#bt-ps');
        ps.oninput=function(){S.pivotOffset=+this.value;p.querySelector('#bt-pv').textContent=S.pivotOffset.toFixed(2);applyBend();};
        var as=p.querySelector('#bt-as');
        as.oninput=function(){
            S.angleDeg=+this.value;
            var avEl=p.querySelector('#bt-av'); if(avEl)avEl.textContent=S.angleDeg.toFixed(0)+'°';
            var avAxis=p.querySelector('#bt-av-axis'); if(avAxis)avAxis.textContent=S.angleDeg.toFixed(0)+'°';
            if(S.mode==='multi'&&S.selectedPoint>=0){
                S.ctrlPoints[S.selectedPoint].angle=S.angleDeg;
                p.querySelector('#bt-sang').value=S.angleDeg;
                p.querySelector('#bt-sangv').textContent=S.angleDeg.toFixed(0)+'°';
                refreshPointsUI();
            }
            applyBend();
        };

        // Mode buttons
        p.querySelector('#bt-mode-blender').onclick=function(){setMode('blender');};
        p.querySelector('#bt-mode-multi').onclick=function(){setMode('multi');};
        p.querySelector('#bt-mode-axis').onclick=function(){setMode('axis');};

        // Multi controls
        p.querySelector('#bt-add').onclick=function(){addPointMidpoint();};
        p.querySelector('#bt-clear').onclick=function(){clearPoints();};
        var spos=p.querySelector('#bt-spos');
        spos.oninput=function(){
            if(S.selectedPoint<0)return;
            var newPos=+this.value/100; S.ctrlPoints[S.selectedPoint].pos=newPos;
            var v=S.originalVerts,ab=getAxisBounds(v,S.bendAxis),tc=ab.mn+newPos*ab.len;
            var best=-1,bestD=Infinity;
            for(var i=0;i<v.length/8;i++){var o=i*8;var d=Math.abs(v[o+S.bendAxis]-tc);if(d<bestD){bestD=d;best=i;}}
            if(best>=0)S.ctrlPoints[S.selectedPoint].vertIdx=best;
            p.querySelector('#bt-sposv').textContent=(newPos*100).toFixed(0)+'%';
            applyBend();refreshPointsUI();
        };
        var sang=p.querySelector('#bt-sang');
        sang.oninput=function(){
            if(S.selectedPoint<0)return;
            S.ctrlPoints[S.selectedPoint].angle=+this.value;
            p.querySelector('#bt-sangv').textContent=(+this.value).toFixed(0)+'°';
            applyBend();refreshPointsUI();
        };

        // Apply/Reset/Cancel
        p.querySelector('#bt-apply').onclick=function(){exitBendMode(true);};
        p.querySelector('#bt-reset').onclick=function(){
            S.angleDeg=0;S.pivotOffset=0;S.anchorVert=-1;
            as.value=0;ps.value=0;
            p.querySelector('#bt-av').textContent='0°';
            p.querySelector('#bt-pv').textContent='0.00';
            removeAnchorMarker();
            if(S.mode==='multi'){S.ctrlPoints=[];S.selectedPoint=-1;rebuildMarkers();refreshPointsUI();}
            S.hbMesh.vertices=new Float32Array(S.originalVerts);refresh();
        };
        p.querySelector('#bt-cancel').onclick=function(){exitBendMode(false);};

        // ══ MOUSE HANDLERS ══

        ov.onmousedown=function(e){
            if(e.target!==ov)return;
            var mx=e.clientX,my=e.clientY;

            if(S.mode==='blender'){
                // Click on anchor → start drag for angle
                if(findAnchorAtScreen(mx,my)){
                    S.dragMode='angle';
                    S.axStartX=mx;S.axStartY=my;S.axStartAngle=S.angleDeg;
                    e.preventDefault();return;
                }
                // Click on object → set anchor
                var vi=findVertexAtScreen(mx,my);
                if(vi>=0){setAnchor(vi);e.preventDefault();return;}
                // Click empty → angle drag
                S.dragMode='angle';
                S.axStartX=mx;S.axStartY=my;S.axStartAngle=S.angleDeg;
                e.preventDefault();
            } else if(S.mode==='multi'){
                // Click marker → drag marker
                var mi=findMarkerAtScreen(mx,my);
                if(mi>=0){S.selectedPoint=mi;S.dragMode='marker';S.dragMarkerIdx=mi;refreshPointsUI();updateMarkerPositions();e.preventDefault();return;}
                // Click object → add point
                var vi2=findVertexAtScreen(mx,my);
                if(vi2>=0){addPointAtVertex(vi2);e.preventDefault();return;}
                // Click empty → angle drag
                S.dragMode='angle';S.axStartX=mx;S.axStartY=my;
                S.axStartAngle=S.selectedPoint>=0?S.ctrlPoints[S.selectedPoint].angle:S.angleDeg;
                e.preventDefault();
            } else {
                // Axis mode: always angle drag
                S.dragMode='angle';S.axStartX=mx;S.axStartY=my;S.axStartAngle=S.angleDeg;
                e.preventDefault();
            }
        };

        ov.onmousemove=function(e){
            if(!S.dragMode)return;
            var mx=e.clientX,my=e.clientY,dx=mx-S.axStartX,dy=my-S.axStartY;
            var na=S.axStartAngle+dx*0.5+dy*0.2;
            na=Math.max(-360,Math.min(360,na));

            if(S.dragMode==='marker'){
                updatePointFromScreen(S.dragMarkerIdx,mx,my);
            } else {
                S.angleDeg=na;
                p.querySelector('#bt-av').textContent=na.toFixed(0)+'°';
                if(p.querySelector('#bt-av-axis'))p.querySelector('#bt-av-axis').textContent=na.toFixed(0)+'°';
                as.value=na;
                if(S.mode==='multi'&&S.selectedPoint>=0){
                    S.ctrlPoints[S.selectedPoint].angle=na;
                    if(p.querySelector('#bt-sang'))p.querySelector('#bt-sang').value=na;
                    if(p.querySelector('#bt-sangv'))p.querySelector('#bt-sangv').textContent=na.toFixed(0)+'°';
                    refreshPointsUI();
                }
                applyBend();
            }
        };

        ov.onmouseup=function(e){S.dragMode=null;S.dragMarkerIdx=-1;};
        ov.onwheel=function(e){
            e.preventDefault();
            var d=e.deltaY>0?0.02:-0.02;
            S.pivotOffset=Math.max(0,Math.min(1,S.pivotOffset+d));
            ps.value=S.pivotOffset;
            p.querySelector('#bt-pv').textContent=S.pivotOffset.toFixed(2);
            applyBend();
        };

        refreshPointsUI();
    }

    // ── Enter / Exit ──

    function enterBendMode(item, callbacks) {
        if(!item||!item.hbMesh) return;
        if(S.active) exitBendMode(false);
        S.item=item; S.hbMesh=item.hbMesh;
        S.originalVerts=new Float32Array(item.hbMesh.vertices);
        S.cb=callbacks||{};
        S.angleDeg=0; S.pivotOffset=0;
        S.ctrlPoints=[]; S.selectedPoint=-1;
        S.anchorVert=-1; S.mode='blender';
        var det=autoAxis(S.originalVerts);
        S.bendAxis=det.ba; S.curveDir=det.cd;
        S.active=true;
        createOverlay();
    }

    function exitBendMode(commit) {
        try {
            if(!S.active) return;
            if(!commit&&S.originalVerts&&S.hbMesh){
                S.hbMesh.vertices=new Float32Array(S.originalVerts);
                refresh();
            }
        } finally {
            removeAnchorMarker();
            disposeMarkers();
            if(S.overlay&&S.overlay.parentNode) S.overlay.parentNode.removeChild(S.overlay);
            S.active=false;S.overlay=null;S.panel=null;
            S.item=null;S.hbMesh=null;S.originalVerts=null;
            S.ctrlPoints=[];S.selectedPoint=-1;
            S.anchorVert=-1;S.dragMode=null;S.dragMarkerIdx=-1;
        }
    }

    return {
        bendVertices:bendVertices,
        bendVerticesMulti:bendVerticesMulti,
        enterBendMode:enterBendMode,
        exitBendMode:exitBendMode,
        isActive:function(){return S.active;},
    };
})();
