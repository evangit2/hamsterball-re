#!/usr/bin/env python3
"""
Hamsterball MESH Renderer - Loads .MESH files and renders them as PNG images.
Uses matplotlib for 3D wireframe/solid rendering with auto-rotation.
"""

import struct
import os
import sys
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

MESH_DIR = os.path.expanduser("~/hamsterball-re/originals/installed/extracted/Meshes")
OUT_DIR = os.path.expanduser("~/hamsterball-re/analysis/screenshots/meshes")

def read_u32(data, pos):
    return struct.unpack_from('<I', data, pos)[0]

def read_f32(data, pos):
    return struct.unpack_from('<f', data, pos)[0]

def parse_mesh(filepath):
    """Parse a .MESH file into vertex data and metadata."""
    with open(filepath, 'rb') as f:
        data = f.read()
    
    if len(data) < 32:
        return None
    
    pos = 0
    version = read_u32(data, pos); pos += 4
    if version < 1 or version > 5:
        return None
    
    name_len = read_u32(data, pos); pos += 4
    name = data[pos:pos+name_len].split(b'\x00')[0].decode('ascii', errors='replace')
    pos += name_len
    
    # Parse material (88 bytes for v1)
    ambient = [read_f32(data, pos + i*4) for i in range(4)]
    diffuse = [read_f32(data, pos + 16 + i*4) for i in range(4)]
    specular = [read_f32(data, pos + 32 + i*4) for i in range(4)]
    shine = read_f32(data, pos + 48)
    pos += 88
    
    # Texture name
    texture = ""
    if pos + 4 <= len(data):
        tex_len = read_u32(data, pos); pos += 4
        if tex_len < 256 and pos + tex_len <= len(data):
            texture = data[pos:pos+tex_len].split(b'\x00')[0].decode('ascii', errors='replace')
            pos += tex_len
    
    # For v2+, find vertex data
    if version > 1:
        # Scan for vertex count
        found = False
        for i in range(min(100, len(data) - pos - 4)):
            val = read_u32(data, pos + i*4)
            if 3 <= val <= 50000:
                # Check if following data looks like floats
                fx = read_f32(data, pos + (i+1)*4)
                fy = read_f32(data, pos + (i+2)*4)
                fz = read_f32(data, pos + (i+3)*4)
                if abs(fx) < 500 and abs(fy) < 500 and abs(fz) < 500:
                    pos = pos + i*4
                    found = True
                    break
        if not found:
            return {'name': name, 'version': version, 'texture': texture,
                    'diffuse': diffuse, 'vertices': np.zeros((0, 3)),
                    'faces': [], 'vertex_count': 0}
    
    vertex_count = read_u32(data, pos); pos += 4
    
    if vertex_count < 1 or vertex_count > 100000:
        return {'name': name, 'version': version, 'texture': texture,
                'diffuse': diffuse, 'vertices': np.zeros((0, 3)),
                'faces': [], 'vertex_count': 0}
    
    # Skip unknown uint32
    if pos + 4 <= len(data):
        pos += 4
    
    # Read vertices: x, y, z, nx, ny, nz, u, v = 32 bytes each
    verts = []
    norms = []
    uvs = []
    for i in range(min(vertex_count, (len(data) - pos) // 32)):
        if pos + 32 > len(data):
            break
        x = read_f32(data, pos); pos += 4
        y = read_f32(data, pos); pos += 4
        z = read_f32(data, pos); pos += 4
        nx = read_f32(data, pos); pos += 4
        ny = read_f32(data, pos); pos += 4
        nz = read_f32(data, pos); pos += 4
        u = read_f32(data, pos); pos += 4
        v = read_f32(data, pos); pos += 4
        verts.append([x, y, z])
        norms.append([nx, ny, nz])
        uvs.append([u, v])
    
    verts = np.array(verts)
    
    # Try to read face data (triangle indices after vertices)
    faces = []
    remaining = len(data) - pos
    # Try reading triangles as triplets of uint16 indices
    if remaining >= 6:
        face_pos = pos
        # Try to find face count
        for offset in range(0, min(16, remaining - 4), 4):
            fc = read_u32(data, face_pos + offset)
            if 1 <= fc <= vertex_count // 3:
                face_pos = face_pos + offset + 4
                for _ in range(min(fc, 20000)):
                    if face_pos + 6 > len(data):
                        break
                    i0 = struct.unpack_from('<H', data, face_pos)[0]
                    i1 = struct.unpack_from('<H', data, face_pos + 2)[0]
                    i2 = struct.unpack_from('<H', data, face_pos + 4)[0]
                    face_pos += 6
                    if i0 < len(verts) and i1 < len(verts) and i2 < len(verts):
                        faces.append([i0, i1, i2])
                if faces:
                    break
    
    return {
        'name': name,
        'version': version,
        'texture': texture,
        'diffuse': diffuse,
        'vertices': verts,
        'normals': np.array(norms) if norms else None,
        'uvs': np.array(uvs) if uvs else None,
        'faces': faces,
        'vertex_count': len(verts)
    }


def render_mesh(mesh, output_path, views=3):
    """Render a mesh model as a multi-view PNG image."""
    verts = mesh['vertices']
    if len(verts) < 3:
        return False
    
    fig = plt.figure(figsize=(views * 4, 5), dpi=100)
    fig.patch.set_facecolor('#1a1a2e')
    
    # Title with mesh info
    title = f"{mesh['name']} (v{mesh['version']}, {len(verts)} verts"
    if mesh['texture']:
        title += f", tex: {mesh['texture']}"
    title += ")"
    
    fig.suptitle(title, color='white', fontsize=11, fontweight='bold', y=0.97)
    
    # Compute center and scale
    center = (verts.max(axis=0) + verts.min(axis=0)) / 2
    extent = (verts.max(axis=0) - verts.min(axis=0)).max() / 2
    if extent < 0.001:
        extent = 1.0
    
    angles = [30, 120, 210] if views == 3 else [45]
    
    for idx, elev_angle in enumerate(angles):
        ax = fig.add_subplot(1, views, idx + 1, projection='3d',
                             computed_zorder=False)
        ax.set_facecolor('#16213e')
        
        # Draw faces if available
        if mesh['faces']:
            # Use Poly3DCollection for solid faces
            face_verts = []
            face_colors = []
            d = mesh['diffuse']
            fc = (min(d[0], 1.0), min(d[1], 1.0), min(d[2], 1.0), 0.85)
            
            for face in mesh['faces'][:5000]:  # Limit for performance
                tri = [verts[i] for i in face]
                face_verts.append(tri)
                face_colors.append(fc)
            
            poly = Poly3DCollection(face_verts, alpha=0.85,
                                    facecolors=face_colors,
                                    edgecolors=(0.4, 0.6, 1.0, 0.3),
                                    linewidths=0.3)
            ax.add_collection3d(poly)
        
        # Always draw wireframe dots/lines
        ax.scatter(verts[:, 0], verts[:, 1], verts[:, 2],
                  c='#e94560', s=2, alpha=0.8, depthshade=True)
        
        # Set limits
        ax.set_xlim(center[0] - extent, center[0] + extent)
        ax.set_ylim(center[1] - extent, center[1] + extent)
        ax.set_zlim(center[2] - extent, center[2] + extent)
        
        ax.view_init(elev=25, azim=elev_angle)
        ax.set_axis_off()
        
        # Add view angle label
        ax.text2D(0.5, 0.02, f"View {idx+1}: {elev_angle}°",
                  transform=ax.transAxes, color='gray', fontsize=8,
                  ha='center')
    
    plt.tight_layout(rect=[0, 0.03, 1, 0.93])
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    fig.savefig(output_path, dpi=100, bbox_inches='tight',
                facecolor=fig.get_facecolor(), edgecolor='none')
    plt.close(fig)
    return True


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    
    mesh_files = sorted([f for f in os.listdir(MESH_DIR) if f.endswith('.MESH')])
    
    print(f"Found {len(mesh_files)} mesh files in {MESH_DIR}")
    
    results = []
    for mf in mesh_files:
        path = os.path.join(MESH_DIR, mf)
        mesh = parse_mesh(path)
        if not mesh:
            print(f"  SKIP {mf}: failed to parse")
            continue
        
        basename = mf.replace('.MESH', '')
        out_path = os.path.join(OUT_DIR, f"{basename}.png")
        
        if len(mesh['vertices']) < 3:
            print(f"  SKIP {mf}: only {len(mesh['vertices'])} vertices")
            continue
        
        print(f"  RENDER {mf}: {len(mesh['vertices'])} verts, {len(mesh['faces'])} faces, tex={mesh.get('texture','')}")
        
        try:
            success = render_mesh(mesh, out_path)
            if success:
                results.append((basename, len(mesh['vertices']), len(mesh['faces']), mesh.get('texture', ''), out_path))
        except Exception as e:
            print(f"  ERROR rendering {mf}: {e}")
    
    print(f"\nRendered {len(results)} meshes to {OUT_DIR}")
    
    # Generate index page
    index_path = os.path.join(OUT_DIR, "index.md")
    with open(index_path, 'w') as f:
        f.write("# Hamsterball Mesh Object Gallery\n\n")
        f.write("| # | Name | Vertices | Faces | Texture | Preview |\n")
        f.write("|---|------|----------|-------|---------|----------|\n")
        for i, (name, nverts, nfaces, tex, path) in enumerate(results, 1):
            rel = os.path.basename(path)
            f.write(f"| {i} | {name} | {nverts} | {nfaces} | {tex} | ![{name}]({rel}) |\n")
    
    print(f"Index written to {index_path}")
    return results

if __name__ == '__main__':
    main()