#!/usr/bin/env python3
"""
MESHWORLD Binary File Creator

Creates custom .MESHWORLD level files for Hamsterball.
Follows the official Raptisoft exporter format exactly:
  docs/MESHWORLD_BINARY_FORMAT_OFFICIAL.md

Coordinate system: D3D (Y-up). All positions written in D3D order.
The binary MESHWORLD format stores vertices as (x, y, z) D3D coords,
NOT Max (x, z, y). Verified from Arena-WarmUp binary analysis.

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

    def write_vec3(self, x, y, z):
        """Write position in D3D file order (x, y, z).
        Verified from Arena-WarmUp binary: vertices and ref points
        are stored in D3D Y-up order, NOT Max XZY order."""
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
        """Write a 32-byte vertex in D3D coordinate order (Y-up).
        
        The binary MESHWORLD format stores vertices in D3D (x, y, z) order,
        NOT Max (x, z, y) order. Verified from Arena-WarmUp binary:
        vertex at D3D (0.34, -833.33, -375.07) stored as same values.
        The original game reads raw bytes directly into the D3D vertex buffer.
        """
        self.write_f32(x)       # D3D X
        self.write_f32(y)       # D3D Y (vertical)
        self.write_f32(z)       # D3D Z
        self.write_f32(nx)      # Normal X
        self.write_f32(ny)      # Normal Y
        self.write_f32(nz)      # Normal Z
        self.write_f32(u)
        self.write_f32(v)

    def write_cube(self, x1, y1, z1, x2, y2, z2):
        """Write octree cube bounds (6 floats) in D3D order."""
        self.write_f32(x1)
        self.write_f32(y1)
        self.write_f32(z1)
        self.write_f32(x2)
        self.write_f32(y2)
        self.write_f32(z2)

    def write_geom(self, name="", diffuse=None, ambient=None, specular=None,
                   emissive=None, power=10.0, triangles=None):
        """Write a complete geom (name + material + strip data).
        
        triangles: list of (v0_idx, v1_idx, v2_idx) index triples.
        
        The proven format for the original game is:
        strip_count=1, tri_count=N, vertex_offset=0
        with vertices laid out as 3 per triangle in the vertex buffer.
        """
        self.write_string(name)
        self.write_material(
            ambient=ambient,
            diffuse=diffuse,
            specular=specular,
            emissive=emissive,
            power=power,
            has_reflection=0,
            texture=None
        )
        
        n_tris = len(triangles) if triangles else 0
        self.write_u32(1)        # strip_count = 1 (proven working format)
        self.write_u32(n_tris)   # triangle_count
        self.write_u32(0)        # vertex_ref_offset = 0

    def get_bytes(self):
        return self.buf.getvalue()


def _write_sections_1_to_4(w, start_pos, camera_pos, n_lights=1, light_pos=None,
                           light_lookat=None, bg_color=None, ambient_color=None):
    """Write Sections 1-4 (ref points, splines, lights, background/ambient)."""
    # Section 1: Ref Points
    w.write_u32(2)
    w.write_string("START1-1")
    w.write_vec3(*start_pos)
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)  # no material

    w.write_string("CameraLocus1")
    w.write_vec3(*camera_pos)
    w.write_f32(0); w.write_f32(0); w.write_f32(0)
    w.write_u32(0)

    # Section 2: Splines
    w.write_u32(0)

    # Section 3: Lights
    w.write_u32(n_lights)
    if light_pos is None:
        light_pos = (100, 500, 100)
    if light_lookat is None:
        light_lookat = (0, 0, 0)
    w.write_u32(0)  # DISTANTLIGHT type
    w.write_vec3(*light_pos)
    w.write_vec3(*light_lookat)
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)

    # Section 4: Background & Ambient
    if bg_color is None:
        bg_color = (0.33, 0.47, 0.84)
    if ambient_color is None:
        ambient_color = (0.25, 0.30, 0.40)
    w.write_f32(bg_color[0]); w.write_f32(bg_color[1]); w.write_f32(bg_color[2])
    w.write_f32(ambient_color[0]); w.write_f32(ambient_color[1]); w.write_f32(ambient_color[2])


def create_plane_level(size=400.0, y=0.0, color=None, start_pos=None):
    """Create a minimal MESHWORLD level: a thick box slab with a START point.

    This is the simplest proven-working custom level format.
    Uses strip_count=1, tri_count=12, vertex_offset=0 with 36 vertices
    arranged as 12 triangles (3 verts each) in a flat triangle list.

    Args:
        size: Half-width of the plane (in D3D units)
        y: Y position of the plane center (D3D Y-up)
        color: Diffuse color as (r,g,b,a) tuple, or None for default
        start_pos: (x, y, z) D3D coords for START point, or None for auto
    """
    w = MWWriter()

    if color is None:
        color = (0.6, 0.7, 0.9, 1.0)

    h = size
    t = 25.0  # half-thickness
    y_top = y + t
    y_bot = y - t

    if start_pos is None:
        start_pos = (0, y_top + 52, 0)

    _write_sections_1_to_4(w, start_pos=start_pos,
                           camera_pos=(0, y_top + 10, 0),
                           light_pos=(100, y_top + 500, 100),
                           light_lookat=(0, y_top, 0))

    # Section 5: Vertex buffer — 6 faces × 2 tris × 3 verts = 36 vertices
    vertices = [
        # Top face (y=y_top, normal up)
        (-h, y_top, -h, 0, 1, 0, 0, 1),
        ( h, y_top, -h, 0, 1, 0, 1, 1),
        ( h, y_top,  h, 0, 1, 0, 1, 0),
        (-h, y_top, -h, 0, 1, 0, 0, 1),
        ( h, y_top,  h, 0, 1, 0, 1, 0),
        (-h, y_top,  h, 0, 1, 0, 0, 0),
        # Bottom face (y=y_bot, normal down)
        (-h, y_bot, -h, 0, -1, 0, 0, 1),
        ( h, y_bot,  h, 0, -1, 0, 1, 0),
        ( h, y_bot, -h, 0, -1, 0, 1, 1),
        (-h, y_bot, -h, 0, -1, 0, 0, 1),
        (-h, y_bot,  h, 0, -1, 0, 0, 0),
        ( h, y_bot,  h, 0, -1, 0, 1, 0),
        # Front face (z=h, normal forward)
        (-h, y_bot, h, 0, 0, 1, 0, 1),
        (-h, y_top, h, 0, 0, 1, 0, 0),
        ( h, y_top, h, 0, 0, 1, 1, 0),
        (-h, y_bot, h, 0, 0, 1, 0, 1),
        ( h, y_top, h, 0, 0, 1, 1, 0),
        ( h, y_bot, h, 0, 0, 1, 1, 1),
        # Back face (z=-h, normal backward)
        (-h, y_bot, -h, 0, 0, -1, 1, 1),
        ( h, y_top, -h, 0, 0, -1, 0, 0),
        (-h, y_top, -h, 0, 0, -1, 1, 0),
        (-h, y_bot, -h, 0, 0, -1, 1, 1),
        ( h, y_bot, -h, 0, 0, -1, 0, 1),
        ( h, y_top, -h, 0, 0, -1, 0, 0),
        # Right face (x=h, normal right)
        (h, y_bot, -h, 1, 0, 0, 0, 1),
        (h, y_top,  h, 1, 0, 0, 1, 0),
        (h, y_top, -h, 1, 0, 0, 0, 0),
        (h, y_bot, -h, 1, 0, 0, 0, 1),
        (h, y_bot,  h, 1, 0, 0, 1, 1),
        (h, y_top,  h, 1, 0, 0, 1, 0),
        # Left face (x=-h, normal left)
        (-h, y_bot, -h, -1, 0, 0, 1, 1),
        (-h, y_top, -h, -1, 0, 0, 1, 0),
        (-h, y_top,  h, -1, 0, 0, 0, 0),
        (-h, y_bot, -h, -1, 0, 0, 1, 1),
        (-h, y_top,  h, -1, 0, 0, 0, 0),
        (-h, y_bot,  h, -1, 0, 0, 0, 1),
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # Section 6: Octree
    w.write_cube(-h - 1, y_bot - 1, -h - 1, h + 1, y_top + 1, h + 1)
    w.write_u32(0)  # leaf
    w.write_u32(1)  # 1 geom
    w.write_geom(
        name="",
        ambient=(0.3, 0.35, 0.5, 1.0),
        diffuse=color,
        specular=(0.5, 0.5, 0.5, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
        power=10.0,
        triangles=list(range(12))  # 12 triangles
    )

    return w.get_bytes()


def create_bowl_level(radius=300.0, depth=150.0, color=None, start_pos=None):
    """Create a MESHWORLD arena-style bowl level.

    Generates a smooth hemispherical bowl with inward-facing normals
    and a rim edge. Uses the proven single-strip format:
    strip_count=1, tri_count=N, vertex_offset=0.
    
    The vertex buffer is organized as a flat triangle list:
    each triangle's 3 vertices appear consecutively.
    This matches the proven plane preset format.
    """
    w = MWWriter()

    if color is None:
        color = (0.78, 0.32, 0.80, 1.0)  # Purple-pink like Arena-WarmUp

    if start_pos is None:
        start_pos = (0, -depth + 52, 0)

    # Sections 1-4
    _write_sections_1_to_4(w, start_pos=start_pos,
                           camera_pos=(0, -depth / 2, 0),
                           bg_color=(0.98, 0.46, 1.0),
                           ambient_color=(0.56, 0.56, 0.56))

    # Section 5: Vertex buffer
    # Build the bowl as a triangle list (3 vertices per triangle, sequential).
    # This is the proven format: strip_count=1 reads vertices sequentially.
    n_rings = 8
    n_sectors = 16

    # Generate ring positions
    rings = []  # rings[ring][sector] = (x, y, z, nx, ny, nz, u, v)
    for ring in range(n_rings + 1):
        t = ring / n_rings
        r = radius * (1.0 - t)
        y = -depth * t
        ring_data = []
        for sector in range(n_sectors):
            angle = 2.0 * math.pi * sector / n_sectors
            x = r * math.cos(angle)
            z = r * math.sin(angle)
            # Inward-pointing normal
            if r > 0.01:
                nx = -x / r
                nz = -z / r
                ny = depth / radius
            else:
                nx = 0; ny = 1; nz = 0
            nl = math.sqrt(nx*nx + ny*ny + nz*nz)
            if nl > 0.001: nx/=nl; ny/=nl; nz/=nl
            ring_data.append((x, y, z, nx, ny, nz, sector/n_sectors, t))
        rings.append(ring_data)

    # Rim top vertices (outside the bowl, going up)
    rim_tops = []
    for sector in range(n_sectors):
        angle = 2.0 * math.pi * sector / n_sectors
        x = (radius + 10) * math.cos(angle)
        z = (radius + 10) * math.sin(angle)
        rim_tops.append((x, 10, z, 0, 1, 0, sector/n_sectors, 0))

    # Build vertex list as flat triangle list
    vertices = []

    def emit_tri(i0_tuple, i1_tuple, i2_tuple):
        """Append 3 vertices for one triangle."""
        vertices.append(i0_tuple)
        vertices.append(i1_tuple)
        vertices.append(i2_tuple)

    # Interior bowl: ring-to-ring quads → 2 triangles each
    for ring in range(n_rings):
        for sector in range(n_sectors):
            s_next = (sector + 1) % n_sectors
            v0 = rings[ring][sector]
            v1 = rings[ring][s_next]
            v2 = rings[ring + 1][sector]
            v3 = rings[ring + 1][s_next]
            emit_tri(v0, v2, v1)
            emit_tri(v1, v2, v3)

    # Rim edge: connects rim (ring 0) to rim_top
    for sector in range(n_sectors):
        s_next = (sector + 1) % n_sectors
        v0 = rings[0][sector]
        v1 = rings[0][s_next]
        v2 = rim_tops[sector]
        v3 = rim_tops[s_next]
        emit_tri(v0, v2, v1)
        emit_tri(v1, v2, v3)

    n_tris = len(vertices) // 3

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # Section 6: Octree
    w.write_cube(-radius - 15, -depth - 5, -radius - 15,
                 radius + 15, 15, radius + 15)
    w.write_u32(0)  # leaf
    w.write_u32(1)  # 1 geom
    w.write_geom(
        name="",
        ambient=(0.4, 0.3, 0.4, 1.0),
        diffuse=color,
        specular=(0.6, 0.6, 0.6, 1.0),
        emissive=(0.1, 0.05, 0.1, 1.0),
        power=20.0,
        triangles=list(range(n_tris))
    )

    return w.get_bytes()


def create_ramp_level(size=400.0, ramp_angle=15.0, color=None, start_pos=None):
    """Create a MESHWORLD level with a flat plane and a ramp.
    
    Two separate geoms, each with their own vertices laid out sequentially.
    """
    w = MWWriter()

    if color is None:
        color = (0.7, 0.5, 0.3, 1.0)

    if start_pos is None:
        start_pos = (0, 52, 0)

    angle_rad = math.radians(ramp_angle)
    ramp_height = size * math.tan(angle_rad)

    _write_sections_1_to_4(w, start_pos=start_pos,
                           camera_pos=(0, ramp_height / 2, 0),
                           light_pos=(100, ramp_height + 500, 100),
                           light_lookat=(0, ramp_height / 2, 0))

    h = size
    ny = math.cos(angle_rad)
    nz = -math.sin(angle_rad)

    # Vertices for both geoms, laid out sequentially
    # Geom 1 (flat): vertices 0-5
    # Geom 2 (ramp): vertices 6-11
    vertices = [
        # Flat section
        (-h/2, 0, -h, 0, 1, 0, 0, 1),  # 0
        ( h/2, 0, -h, 0, 1, 0, 1, 1),  # 1
        ( h/2, 0,  0, 0, 1, 0, 1, 0),  # 2
        (-h/2, 0, -h, 0, 1, 0, 0, 1),  # 3
        ( h/2, 0,  0, 0, 1, 0, 1, 0),  # 4
        (-h/2, 0,  0, 0, 1, 0, 0, 0),  # 5
        # Ramp section
        (-h/2, 0,          0, 0, ny, nz, 0, 1),  # 6
        ( h/2, 0,          0, 0, ny, nz, 1, 1),  # 7
        ( h/2, ramp_height, h, 0, ny, nz, 1, 0),  # 8
        (-h/2, 0,          0, 0, ny, nz, 0, 1),  # 9
        ( h/2, ramp_height, h, 0, ny, nz, 1, 0),  # 10
        (-h/2, ramp_height, h, 0, ny, nz, 0, 0),  # 11
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    w.write_cube(-h - 1, -1, -h - 1, h + 1, ramp_height + 1, h + 1)
    w.write_u32(0)
    w.write_u32(2)

    # Geom 1: flat section (2 tris at offset 0)
    w.write_geom("", ambient=(0.3, 0.35, 0.5, 1.0),
                 diffuse=(0.6, 0.7, 0.9, 1.0), triangles=list(range(2)))
    # Geom 2: ramp (2 tris at offset 6) — but write_geom writes offset=0
    # We need to write the offset manually
    w.write_string("")
    w.write_material(ambient=(0.35, 0.25, 0.15, 1.0), diffuse=color)
    w.write_u32(1)   # strip_count
    w.write_u32(2)   # tri_count
    w.write_u32(6)   # vertex_offset = 6

    return w.get_bytes()


def create_platforms_level(color=None):
    """Create a level with multiple floating platforms at different heights,
    connected by ramps. Good test for collision and physics."""
    w = MWWriter()

    if color is None:
        color = (0.4, 0.8, 0.4, 1.0)

    _write_sections_1_to_4(w, start_pos=(0, 52, 0),
                           camera_pos=(0, 50, 200),
                           light_pos=(100, 500, 200),
                           light_lookat=(0, 0, 200),
                           bg_color=(0.1, 0.15, 0.3),
                           ambient_color=(0.2, 0.25, 0.35))

    # All vertices in sequential order for 3 geoms
    # Geom 1 (platform 1): verts 0-5
    # Geom 2 (platform 2): verts 6-11
    # Geom 3 (ramp): verts 12-17
    vertices = [
        (-100, 0, -100, 0, 1, 0, 0, 1),   # 0
        ( 100, 0, -100, 0, 1, 0, 1, 1),   # 1
        ( 100, 0,  100, 0, 1, 0, 1, 0),   # 2
        (-100, 0, -100, 0, 1, 0, 0, 1),   # 3
        ( 100, 0,  100, 0, 1, 0, 1, 0),   # 4
        (-100, 0,  100, 0, 1, 0, 0, 0),   # 5

        (-100, 50, 200, 0, 1, 0, 0, 1),   # 6
        ( 100, 50, 200, 0, 1, 0, 1, 1),   # 7
        ( 100, 50, 400, 0, 1, 0, 1, 0),   # 8
        (-100, 50, 200, 0, 1, 0, 0, 1),   # 9
        ( 100, 50, 400, 0, 1, 0, 1, 0),   # 10
        (-100, 50, 400, 0, 1, 0, 0, 0),   # 11

        (-100,  0, 100, 0, 0.894, -0.447, 0, 1),  # 12
        ( 100,  0, 100, 0, 0.894, -0.447, 1, 1),  # 13
        ( 100, 50, 200, 0, 0.894, -0.447, 1, 0),  # 14
        (-100,  0, 100, 0, 0.894, -0.447, 0, 1),  # 15
        ( 100, 50, 200, 0, 0.894, -0.447, 1, 0),  # 16
        (-100, 50, 200, 0, 0.894, -0.447, 0, 0),  # 17
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    w.write_cube(-101, -1, -101, 101, 51, 401)
    w.write_u32(0)
    w.write_u32(3)

    w.write_geom("", ambient=(0.2, 0.35, 0.2, 1.0), diffuse=color,
                 triangles=list(range(2)))

    # Geom 2: offset 6
    w.write_string("")
    w.write_material(ambient=(0.2, 0.2, 0.35, 1.0),
                     diffuse=(0.4, 0.4, 0.8, 1.0))
    w.write_u32(1); w.write_u32(2); w.write_u32(6)

    # Geom 3: offset 12
    w.write_string("")
    w.write_material(ambient=(0.3, 0.3, 0.15, 1.0),
                     diffuse=(0.8, 0.7, 0.3, 1.0))
    w.write_u32(1); w.write_u32(2); w.write_u32(12)

    return w.get_bytes()


PRESETS = {
    'plane': ('Simple flat plane (thick box)', create_plane_level),
    'bowl': ('Hemispherical arena bowl', create_bowl_level),
    'ramp': ('Flat plane with angled ramp', create_ramp_level),
    'platforms': ('Floating platforms connected by ramps', create_platforms_level),
}


def main():
    parser = argparse.ArgumentParser(description='Create custom MESHWORLD level files')
    parser.add_argument('--output', '-o', required=True, help='Output .MESHWORLD file path')
    parser.add_argument('--preset', '-p', choices=list(PRESETS.keys()),
                        default='plane', help='Level preset (default: plane)')
    parser.add_argument('--size', type=float, default=400, help='Level size parameter')
    parser.add_argument('--color', type=str, default=None,
                        help='Diffuse color as R,G,B,A (0-1 range)')
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
