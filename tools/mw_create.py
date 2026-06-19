#!/usr/bin/env python3
"""
MESHWORLD Binary File Creator

Creates custom .MESHWORLD level files for Hamsterball.
Follows the official Raptisoft exporter format exactly:
  docs/MESHWORLD_BINARY_FORMAT_OFFICIAL.md

Coordinate system: 3DS Max (Z-up, Y-forward) — the file stores
positions as x, z, y (Max order), and the parser swaps to D3D Y-up.

Usage:
  python3 mw_create.py --output CustomLevel.MESHWORLD
  python3 mw_create.py --preset plane --output TestPlane.MESHWORLD
"""

import struct
import argparse
import math
import sys
from io import BytesIO


class MWWriter:
    """Writes MESHWORLD binary format following official spec."""

    def __init__(self):
        self.buf = BytesIO()

    def write_u32(self, val):
        self.buf.write(struct.pack('<I', val))

    def write_i32(self, val):
        self.buf.write(struct.pack('<i', val))

    def write_f32(self, val):
        self.buf.write(struct.pack('<f', val))

    def write_string(self, s):
        """Write length-prefixed string (official format: [int length][data]).
        Length includes NUL terminator. Data is NUL-terminated."""
        encoded = s.encode('ascii') + b'\x00'
        self.write_u32(len(encoded))
        self.buf.write(encoded)

    def write_vec3_max(self, x, y, z):
        """Write position in Section 1 file order (ref points, lights, splines).
        Section 1 file stores: max_x, max_z, max_y (XZY Max order).
        Parser reads 3 floats as (max_x, max_z, max_y) then:
          d3d_x = max_x, d3d_y = max_z, d3d_z = max_y
        Result: file floats map 1:1 to D3D coords.
        Verified: Level1 START1-1 at D3D (23.4, 40.1, -13.4) stored as same values."""
        self.write_f32(x)
        self.write_f32(y)
        self.write_f32(z)

    def write_material(self, diffuse=None, ambient=None, specular=None, emissive=None,
                       power=10.0, has_reflection=0, texture=None):
        """Write material data (4×float RGBA × 4 + power + reflection + texture)."""
        def write_color4(c):
            if c is None:
                self.buf.write(struct.pack('<4f', 0.8, 0.8, 0.8, 1.0))
            else:
                self.buf.write(struct.pack('<4f', *c))

        write_color4(ambient)    # Ambient rgba
        write_color4(diffuse)    # Diffuse rgba
        write_color4(specular)   # Specular rgba
        write_color4(emissive)   # Emissive rgba
        self.write_f32(power)    # Shininess
        self.write_u32(has_reflection)
        if texture:
            self.write_u32(1)  # has_texture
            self.write_string(texture)
        else:
            self.write_u32(0)  # no texture

    def write_vertex(self, x, y, z, nx=0, ny=1, nz=0, u=0, v=0):
        """Write a 32-byte vertex (Max coordinate order for file storage).
        Input: D3D coordinates (Y-up). Output: stored with Y↔Z swap for parser.
        The parser swaps: v->y=max_z, v->z=max_y, v->ny=max_nz, v->nz=max_ny.
        So file order: d3d_x, d3d_z, d3d_y (same swap as write_vec3_max)."""
        # Position: swap Y and Z
        self.write_f32(x)       # D3D X → file X
        self.write_f32(z)       # D3D Z → file Y (parser: max_y → d3d_z)
        self.write_f32(y)       # D3D Y → file Z (parser: max_z → d3d_y)
        # Normals: same swap
        self.write_f32(nx)      # Normal X
        self.write_f32(nz)      # D3D NZ → file NY (parser: max_ny → d3d_nz)
        self.write_f32(ny)      # D3D NY → file NZ (parser: max_nz → d3d_ny)
        self.write_f32(u)
        self.write_f32(v)

    def write_cube(self, x1, y1, z1, x2, y2, z2):
        """Write octree cube bounds (6 floats)."""
        self.write_f32(x1)
        self.write_f32(y1)
        self.write_f32(z1)
        self.write_f32(x2)
        self.write_f32(y2)
        self.write_f32(z2)

    def get_bytes(self):
        return self.buf.getvalue()


def create_plane_level(size=400.0, y=0.0, color=None, start_pos=None):
    """Create a minimal MESHWORLD level: a flat plane with a START point.

    Args:
        size: Half-width of the plane (in D3D units)
        y: Y position of the plane (D3D Y-up)
        color: Diffuse color as (r,g,b,a) tuple, or None for default grey
        start_pos: (x, y, z) D3D coords for START point, or None for center
    """
    w = MWWriter()

    if color is None:
        color = (0.6, 0.7, 0.9, 1.0)  # Light blue

    if start_pos is None:
        start_pos = (0, y + 25 + 26 + 2, 0)  # Ball spawn above top of box (y_top + radius + margin)

    # ========== SECTION 1: Ref Points ==========
    # Two objects: START1-1 and CAMERALOOKAT
    w.write_u32(2)

    # START1-1 object
    w.write_string("START1-1")
    w.write_vec3_max(start_pos[0], start_pos[1], start_pos[2])
    w.write_f32(0)  # rot x
    w.write_f32(0)  # rot z
    w.write_f32(0)  # rot y
    w.write_u32(0)  # no material

    # CAMERALOOKAT — center of level, slightly above plane
    w.write_string("CameraLocus1")
    w.write_vec3_max(0, y + 10, 0)
    w.write_f32(0)
    w.write_f32(0)
    w.write_f32(0)
    w.write_u32(0)  # no material

    # ========== SECTION 2: Splines ==========
    w.write_u32(0)  # no splines

    # ========== SECTION 3: Lights ==========
    w.write_u32(1)  # one distant light
    w.write_u32(0)  # DISTANTLIGHT type
    # Position (Max order: x, z, y)
    w.write_vec3_max(100, y + 500, 100)
    # LookAt (Max order)
    w.write_vec3_max(0, y, 0)
    # Color
    w.write_f32(1.0)
    w.write_f32(1.0)
    w.write_f32(1.0)

    # ========== SECTION 4: Background & Ambient ==========
    w.write_f32(0.33)  # bg R
    w.write_f32(0.47)  # bg G
    w.write_f32(0.84)  # bg B
    w.write_f32(0.25)  # ambient R
    w.write_f32(0.30)  # ambient G
    w.write_f32(0.40)  # ambient B

    # ========== SECTION 5: Global Vertex Buffer ==========
    # A thick box (slab) instead of a thin plane — gives collision system more to hit.
    # 6 faces × 2 triangles × 3 vertices = 36 vertices
    h = size  # half-size
    t = 25.0  # thickness (half) — thick enough for reliable collision
    y_top = y + t
    y_bot = y - t

    vertices = [
        # Top face (y=y_top, normal up) — the surface the ball rolls on
        (-h, y_top, -h, 0, 1, 0, 0, 1),  # v0
        ( h, y_top, -h, 0, 1, 0, 1, 1),  # v1
        ( h, y_top,  h, 0, 1, 0, 1, 0),  # v2
        (-h, y_top, -h, 0, 1, 0, 0, 1),  # v3
        ( h, y_top,  h, 0, 1, 0, 1, 0),  # v4
        (-h, y_top,  h, 0, 1, 0, 0, 0),  # v5

        # Bottom face (y=y_bot, normal down)
        (-h, y_bot, -h, 0, -1, 0, 0, 1),  # v6
        ( h, y_bot,  h, 0, -1, 0, 1, 0),  # v7
        ( h, y_bot, -h, 0, -1, 0, 1, 1),  # v8
        (-h, y_bot, -h, 0, -1, 0, 0, 1),  # v9
        (-h, y_bot,  h, 0, -1, 0, 0, 0),  # v10
        ( h, y_bot,  h, 0, -1, 0, 1, 0),  # v11

        # Front face (z=h, normal forward)
        (-h, y_bot, h, 0, 0, 1, 0, 1),  # v12
        (-h, y_top, h, 0, 0, 1, 0, 0),  # v13
        ( h, y_top, h, 0, 0, 1, 1, 0),  # v14
        (-h, y_bot, h, 0, 0, 1, 0, 1),  # v15
        ( h, y_top, h, 0, 0, 1, 1, 0),  # v16
        ( h, y_bot, h, 0, 0, 1, 1, 1),  # v17

        # Back face (z=-h, normal backward)
        (-h, y_bot, -h, 0, 0, -1, 1, 1),  # v18
        ( h, y_top, -h, 0, 0, -1, 0, 0),  # v19
        (-h, y_top, -h, 0, 0, -1, 1, 0),  # v20
        (-h, y_bot, -h, 0, 0, -1, 1, 1),  # v21
        ( h, y_bot, -h, 0, 0, -1, 0, 1),  # v22
        ( h, y_top, -h, 0, 0, -1, 0, 0),  # v23

        # Right face (x=h, normal right)
        (h, y_bot, -h, 1, 0, 0, 0, 1),  # v24
        (h, y_top,  h, 1, 0, 0, 1, 0),  # v25
        (h, y_top, -h, 1, 0, 0, 0, 0),  # v26
        (h, y_bot, -h, 1, 0, 0, 0, 1),  # v27
        (h, y_bot,  h, 1, 0, 0, 1, 1),  # v28
        (h, y_top,  h, 1, 0, 0, 1, 0),  # v29

        # Left face (x=-h, normal left)
        (-h, y_bot, -h, -1, 0, 0, 1, 1),  # v30
        (-h, y_top, -h, -1, 0, 0, 1, 0),  # v31
        (-h, y_top,  h, -1, 0, 0, 0, 0),  # v32
        (-h, y_bot, -h, -1, 0, 0, 1, 1),  # v33
        (-h, y_top,  h, -1, 0, 0, 0, 0),  # v34
        (-h, y_bot,  h, -1, 0, 0, 0, 1),  # v35
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # ========== SECTION 6: Octree ==========
    # Single leaf node containing one geom with one strip (12 triangles)
    # Cube bounds encompass the box
    w.write_cube(-h - 1, y_bot - 1, -h - 1, h + 1, y_top + 1, h + 1)

    # Leaf node: submesh_count = 0
    w.write_u32(0)

    # geom_count = 1
    w.write_u32(1)

    # Geom: empty name (no N:/E: prefix = rendered + collision)
    w.write_string("")

    # Material for this geom
    w.write_material(
        ambient=(0.3, 0.35, 0.5, 1.0),
        diffuse=color,
        specular=(0.5, 0.5, 0.5, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
        power=10.0,
        has_reflection=0,
        texture=None
    )

    # Strip: 12 triangles starting at vertex offset 0
    w.write_u32(1)   # strip_count = 1
    w.write_u32(12)  # triangle_count = 12
    w.write_u32(0)   # vertex_ref_offset = 0

    return w.get_bytes()


def create_ramp_level(size=400.0, ramp_angle=15.0, color=None, start_pos=None):
    """Create a MESHWORLD level with a flat plane and a ramp.

    The ramp goes from one side up at the specified angle.
    """
    w = MWWriter()

    if color is None:
        color = (0.7, 0.5, 0.3, 1.0)  # Warm brown

    if start_pos is None:
        start_pos = (0, 52, 0)

    angle_rad = math.radians(ramp_angle)
    ramp_height = size * math.tan(angle_rad)

    # ========== SECTION 1: Ref Points ==========
    w.write_u32(2)
    w.write_string("START1-1")
    w.write_vec3_max(start_pos[0], start_pos[1], start_pos[2])
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    w.write_string("CameraLocus1")
    w.write_vec3_max(0, ramp_height / 2, 0)
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    # ========== SECTION 2: Splines ==========
    w.write_u32(0)

    # ========== SECTION 3: Lights ==========
    w.write_u32(1)
    w.write_u32(0)
    w.write_vec3_max(100, ramp_height + 500, 100)
    w.write_vec3_max(0, ramp_height / 2, 0)
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)

    # ========== SECTION 4: Background & Ambient ==========
    w.write_f32(0.33); w.write_f32(0.47); w.write_f32(0.84)
    w.write_f32(0.25); w.write_f32(0.30); w.write_f32(0.40)

    # ========== SECTION 5: Global Vertex Buffer ==========
    h = size
    ny = math.cos(angle_rad)
    nz = -math.sin(angle_rad)  # ramp tilts up in -Z direction

    vertices = [
        # Flat section (first half, z from -h to 0)
        (-h/2, 0, -h, 0, 1, 0, 0, 1),  # v0
        ( h/2, 0, -h, 0, 1, 0, 1, 1),  # v1
        ( h/2, 0,  0, 0, 1, 0, 1, 0),  # v2
        (-h/2, 0, -h, 0, 1, 0, 0, 1),  # v3
        ( h/2, 0,  0, 0, 1, 0, 1, 0),  # v4
        (-h/2, 0,  0, 0, 1, 0, 0, 0),  # v5

        # Ramp section (z from 0 to h, y rises from 0 to ramp_height)
        (-h/2, 0,          0, 0, ny, nz, 0, 1),  # v6
        ( h/2, 0,          0, 0, ny, nz, 1, 1),  # v7
        ( h/2, ramp_height, h, 0, ny, nz, 1, 0),  # v8
        (-h/2, 0,          0, 0, ny, nz, 0, 1),  # v9
        ( h/2, ramp_height, h, 0, ny, nz, 1, 0),  # v10
        (-h/2, ramp_height, h, 0, ny, nz, 0, 0),  # v11
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # ========== SECTION 6: Octree ==========
    w.write_cube(-h - 1, -1, -h - 1, h + 1, ramp_height + 1, h + 1)
    w.write_u32(0)  # leaf
    w.write_u32(2)  # 2 geoms

    # Geom 1: flat section
    w.write_string("")
    w.write_material(
        ambient=(0.3, 0.35, 0.5, 1.0),
        diffuse=(0.6, 0.7, 0.9, 1.0),
        specular=(0.5, 0.5, 0.5, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
    )
    w.write_u32(1)  # 1 strip
    w.write_u32(2)  # 2 triangles
    w.write_u32(0)  # vertex offset 0

    # Geom 2: ramp section
    w.write_string("")
    w.write_material(
        ambient=(0.35, 0.25, 0.15, 1.0),
        diffuse=color,
        specular=(0.5, 0.5, 0.5, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
    )
    w.write_u32(1)  # 1 strip
    w.write_u32(2)  # 2 triangles
    w.write_u32(6)  # vertex offset 6

    return w.get_bytes()


def create_bowl_level(radius=300.0, depth=150.0, color=None, start_pos=None):
    """Create a MESHWORLD arena-style bowl level.

    Generates a hemispherical bowl made of triangle strips.
    """
    w = MWWriter()

    if color is None:
        color = (0.9, 0.4, 0.4, 1.0)  # Red-ish arena

    if start_pos is None:
        start_pos = (0, depth/2 + 52, 0)

    # ========== SECTION 1: Ref Points ==========
    w.write_u32(2)
    w.write_string("START1-1")
    w.write_vec3_max(start_pos[0], start_pos[1], start_pos[2])
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    w.write_string("CameraLocus1")
    w.write_vec3_max(0, 0, 0)
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    # ========== SECTION 2: Splines ==========
    w.write_u32(0)

    # ========== SECTION 3: Lights ==========
    w.write_u32(1)
    w.write_u32(0)
    w.write_vec3_max(100, 500, 100)
    w.write_vec3_max(0, 0, 0)
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)

    # ========== SECTION 4: Background & Ambient ==========
    w.write_f32(0.2); w.write_f32(0.1); w.write_f32(0.3)
    w.write_f32(0.2); w.write_f32(0.15); w.write_f32(0.25)

    # ========== SECTION 5: Global Vertex Buffer ==========
    # Bowl: rings from top (y=0) to bottom (y=-depth)
    n_rings = 16
    n_sectors = 24

    vertices = []
    for ring in range(n_rings + 1):
        t = ring / n_rings  # 0=top edge, 1=bottom center
        r = radius * (1.0 - t)  # radius shrinks toward bottom
        y = -depth * t  # Y goes down

        for sector in range(n_sectors):
            angle = 2.0 * math.pi * sector / n_sectors
            x = r * math.cos(angle)
            z = r * math.sin(angle)

            # Normal: pointing outward from bowl center
            # For a paraboloid, normal = normalize(vec3(x, 2*depth*t/radius, z))
            if r > 0.01:
                nx = x / r
                nz = z / r
                # Y component: slope of bowl wall
                ny = depth / radius
            else:
                nx = 0; ny = 1; nz = 0

            nl = math.sqrt(nx*nx + ny*ny + nz*nz)
            nx /= nl; ny /= nl; nz /= nl

            u = sector / n_sectors
            v = t
            vertices.append((x, y, z, nx, ny, nz, u, v))

    # Bottom center vertex (if not already there from last ring at r=0)
    if radius * (1.0 - 1.0) > 0.01:
        vertices.append((0, -depth, 0, 0, 1, 0, 0.5, 1.0))

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # Build triangle indices for the bowl
    # Ring-to-ring quad strips → 2 triangles per quad
    indices = []
    for ring in range(n_rings):
        for sector in range(n_sectors):
            i0 = ring * n_sectors + sector
            i1 = ring * n_sectors + (sector + 1) % n_sectors
            i2 = (ring + 1) * n_sectors + sector
            i3 = (ring + 1) * n_sectors + (sector + 1) % n_sectors

            indices.append(i0)
            indices.append(i1)
            indices.append(i2)

            indices.append(i1)
            indices.append(i3)
            indices.append(i2)

    # Floor at the bottom (simple quad)
    bottom_center = len(vertices) - 1
    # Add floor cap triangles
    last_ring_start = n_rings * n_sectors
    for sector in range(n_sectors):
        i1 = last_ring_start + sector
        i2 = last_ring_start + (sector + 1) % n_sectors
        indices.append(bottom_center)
        indices.append(i2)
        indices.append(i1)

    # ========== SECTION 6: Octree ==========
    w.write_cube(-radius - 1, -depth - 1, -radius - 1,
                 radius + 1, 1, radius + 1)
    w.write_u32(0)  # leaf
    w.write_u32(1)  # 1 geom

    # Geom: bowl
    w.write_string("")
    w.write_material(
        ambient=(0.3, 0.15, 0.15, 1.0),
        diffuse=color,
        specular=(0.6, 0.6, 0.6, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
        power=20.0,
    )

    # Write strip — but the parser converts strips to triangle-list differently
    # The parser's walk_octree converts triangle strips to triangle lists
    # using even/odd winding. But we have a triangle LIST already.
    # 
    # The MESHWORLD strip format is: triangle_count + vertex_offset
    # The parser treats this as a triangle STRIP (sequential vertices, 
    # alternating winding). We need to write our data as a strip.
    #
    # Actually, re-reading the parser code more carefully:
    # strip_count=1, tri_count=N, vertex_offset=0
    # The parser iterates t=0..N-1 and for even t: (t,t+1,t+2), odd: (t,t+2,t+1)
    # This is triangle strip expansion.
    #
    # For a simple triangle LIST (like we generated), we need to either:
    # 1. Write multiple strips of 1 triangle each, OR
    # 2. Write one strip with the correct strip ordering
    #
    # Easiest: write each triangle as a separate strip with tri_count=1
    n_tris = len(indices) // 3

    # But we need the vertices in the right order for strip expansion.
    # For tri_count=1, strip expansion gives: base+0, base+1, base+2 (one tri).
    # vertex_offset would be the starting index in the vertex buffer.
    #
    # The problem: the strip system uses vertex_offset + sequential indices.
    # For tri_count=1 with offset K, it generates triangle (K, K+1, K+2).
    # So we need n_tris strips with offsets 0, 3, 6, 9, ...
    
    w.write_u32(n_tris)  # strip_count
    for i in range(n_tris):
        w.write_u32(1)        # 1 triangle per strip
        w.write_u32(i * 3)    # vertex offset

    return w.get_bytes()


def create_platforms_level(color=None):
    """Create a level with multiple floating platforms at different heights,
    connected by ramps. Good test for collision and physics."""
    w = MWWriter()

    if color is None:
        color = (0.4, 0.8, 0.4, 1.0)  # Green

    # ========== SECTION 1: Ref Points ==========
    w.write_u32(2)
    w.write_string("START1-1")
    w.write_vec3_max(0, 52, 0)  # Center of first platform
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    w.write_string("CameraLocus1")
    w.write_vec3_max(0, 50, 200)
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    # ========== SECTION 2: Splines ==========
    w.write_u32(0)

    # ========== SECTION 3: Lights ==========
    w.write_u32(1)
    w.write_u32(0)
    w.write_vec3_max(100, 500, 200)
    w.write_vec3_max(0, 0, 200)
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)

    # ========== SECTION 4: Background & Ambient ==========
    w.write_f32(0.1); w.write_f32(0.15); w.write_f32(0.3)
    w.write_f32(0.2); w.write_f32(0.25); w.write_f32(0.35)

    # ========== SECTION 5: Global Vertex Buffer ==========
    # Platform 1: at y=0, centered at (0, 0, 0), 200x200
    # Platform 2: at y=50, centered at (0, 50, 300), 200x200
    # Ramp connecting them: from z=100 to z=200, y from 0 to 50
    vertices = []
    platforms = [
        # Platform 1 (y=0, z=0)
        (-100, 0, -100, 0, 1, 0, 0, 1),
        ( 100, 0, -100, 0, 1, 0, 1, 1),
        ( 100, 0,  100, 0, 1, 0, 1, 0),
        (-100, 0, -100, 0, 1, 0, 0, 1),
        ( 100, 0,  100, 0, 1, 0, 1, 0),
        (-100, 0,  100, 0, 1, 0, 0, 0),

        # Platform 2 (y=50, z=300)
        (-100, 50, 200, 0, 1, 0, 0, 1),
        ( 100, 50, 200, 0, 1, 0, 1, 1),
        ( 100, 50, 400, 0, 1, 0, 1, 0),
        (-100, 50, 200, 0, 1, 0, 0, 1),
        ( 100, 50, 400, 0, 1, 0, 1, 0),
        (-100, 50, 400, 0, 1, 0, 0, 0),

        # Ramp from P1 edge (z=100, y=0) to P2 edge (z=200, y=50)
        (-100,  0, 100, 0, 0.894, -0.447, 0, 1),
        ( 100,  0, 100, 0, 0.894, -0.447, 1, 1),
        ( 100, 50, 200, 0, 0.894, -0.447, 1, 0),
        (-100,  0, 100, 0, 0.894, -0.447, 0, 1),
        ( 100, 50, 200, 0, 0.894, -0.447, 1, 0),
        (-100, 50, 200, 0, 0.894, -0.447, 0, 0),
    ]

    w.write_u32(len(platforms))
    for v in platforms:
        w.write_vertex(*v)

    # ========== SECTION 6: Octree ==========
    w.write_cube(-101, -1, -101, 101, 51, 401)
    w.write_u32(0)  # leaf
    w.write_u32(3)  # 3 geoms

    # Geom 1: Platform 1
    w.write_string("")
    w.write_material(
        ambient=(0.2, 0.35, 0.2, 1.0),
        diffuse=color,
    )
    w.write_u32(1)
    w.write_u32(2)  # 2 triangles
    w.write_u32(0)  # offset 0

    # Geom 2: Platform 2
    w.write_string("")
    w.write_material(
        ambient=(0.2, 0.2, 0.35, 1.0),
        diffuse=(0.4, 0.4, 0.8, 1.0),
    )
    w.write_u32(1)
    w.write_u32(2)  # 2 triangles
    w.write_u32(6)  # offset 6

    # Geom 3: Ramp
    w.write_string("")
    w.write_material(
        ambient=(0.3, 0.3, 0.15, 1.0),
        diffuse=(0.8, 0.7, 0.3, 1.0),
    )
    w.write_u32(1)
    w.write_u32(2)  # 2 triangles
    w.write_u32(12)  # offset 12

    return w.get_bytes()


PRESETS = {
    'plane': ('Simple flat plane', create_plane_level),
    'ramp': ('Flat plane with angled ramp', create_ramp_level),
    'bowl': ('Hemispherical arena bowl', create_bowl_level),
    'platforms': ('Floating platforms connected by ramps', create_platforms_level),
}


def main():
    parser = argparse.ArgumentParser(description='Create custom MESHWORLD level files')
    parser.add_argument('--output', '-o', required=True, help='Output .MESHWORLD file path')
    parser.add_argument('--preset', '-p', choices=list(PRESETS.keys()),
                        default='plane', help='Level preset (default: plane)')
    parser.add_argument('--size', type=float, default=400, help='Level size parameter')
    parser.add_argument('--color', type=str, default=None,
                        help='Diffuse color as R,G,B,A (0-1 range), e.g. 0.8,0.4,0.2,1.0')
    parser.add_argument('--start', type=str, default=None,
                        help='Start position as X,Y,Z (D3D coords)')

    args = parser.parse_args()

    color = None
    if args.color:
        parts = [float(x) for x in args.color.split(',')]
        color = tuple(parts)

    start_pos = None
    if args.start:
        parts = [float(x) for x in args.start.split(',')]
        start_pos = tuple(parts)

    desc, create_fn = PRESETS[args.preset]
    print(f"Creating preset '{args.preset}': {desc}")

    # Different presets accept different kwargs
    if args.preset == 'bowl':
        data = create_bowl_level(color=color, start_pos=start_pos)
    elif args.preset == 'platforms':
        data = create_platforms_level(color=color)
    elif args.preset == 'ramp':
        data = create_ramp_level(size=args.size, color=color, start_pos=start_pos)
    else:
        data = create_fn(size=args.size, color=color, start_pos=start_pos)

    with open(args.output, 'wb') as f:
        f.write(data)

    print(f"Written {len(data)} bytes to {args.output}")


if __name__ == '__main__':
    main()
