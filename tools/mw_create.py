#!/usr/bin/env python3
"""
MESHWORLD Binary File Creator

Creates custom .MESHWORLD level files for Hamsterball.
Follows the official Raptisoft exporter format exactly:
  docs/MESHWORLD_BINARY_FORMAT_OFFICIAL.md

Coordinate system: D3D (Y-up). All positions written in D3D order.
The binary MESHWORLD format stores vertices as (x, y, z) D3D coords,
NOT Max (x, z, y). Verified from Arena-WarmUp binary analysis.

CRITICAL: The strip format uses D3D triangle strips, NOT flat triangle lists!
Each strip with tri_count=N consumes N+2 vertices starting at vertex_offset.
The reimpl decodes strips with even/odd winding flip (standard D3D strip).

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
        """Write a 32-byte vertex in D3D coordinate order (Y-up)."""
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

    def write_geom_with_strips(self, name="", diffuse=None, ambient=None, specular=None,
                                emissive=None, power=10.0, strips=None):
        """Write a complete geom (name + material + strip data).

        strips: list of (tri_count, vertex_offset) tuples.
        Each strip with tri_count=N references N+2 consecutive vertices
        in the global vertex buffer starting at vertex_offset.
        This is the D3D triangle strip format.
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

        self.write_u32(len(strips) if strips else 0)
        if strips:
            for tri_count, vertex_offset in strips:
                self.write_u32(tri_count)
                self.write_u32(vertex_offset)

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

    Uses triangle strip format: 12 triangles in one strip using 14 vertices.
    A box has 12 triangles total, but we write it as a flat triangle list
    in strip-compatible order by using strip_count=12 with 1 tri each.
    Actually — simplest proven format: write each face as a 2-tri strip.

    Proven format that works: 6 faces × 1 strip each, 2 tris per strip.
    Each strip uses 4 vertices (2 triangles in strip = 4 vertices).
    Total: 6 strips, 12 tris, 24 unique vertices.
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

    # Section 5: Vertex buffer — 6 faces, each as a 4-vertex strip (2 tris)
    # Total: 24 vertices
    vertices = [
        # Top face (y=y_top, normal up) — strip: v0,v1,v2,v3 → tris (0,1,2),(1,3,2)
        (-h, y_top, -h, 0, 1, 0, 0, 1),   # 0
        ( h, y_top, -h, 0, 1, 0, 1, 1),   # 1
        (-h, y_top,  h, 0, 1, 0, 0, 0),   # 2
        ( h, y_top,  h, 0, 1, 0, 1, 0),   # 3
        # Bottom face (y=y_bot, normal down) — strip
        (-h, y_bot, -h, 0, -1, 0, 0, 1),  # 4
        (-h, y_bot,  h, 0, -1, 0, 0, 0),  # 5
        ( h, y_bot, -h, 0, -1, 0, 1, 1),  # 6
        ( h, y_bot,  h, 0, -1, 0, 1, 0),  # 7
        # Front face (z=h, normal forward) — strip
        (-h, y_bot, h, 0, 0, 1, 0, 1),    # 8
        (-h, y_top, h, 0, 0, 1, 0, 0),    # 9
        ( h, y_bot, h, 0, 0, 1, 1, 1),    # 10
        ( h, y_top, h, 0, 0, 1, 1, 0),    # 11
        # Back face (z=-h, normal backward) — strip
        (-h, y_bot, -h, 0, 0, -1, 1, 1),  # 12
        ( h, y_bot, -h, 0, 0, -1, 0, 1),  # 13
        (-h, y_top, -h, 0, 0, -1, 1, 0),  # 14
        ( h, y_top, -h, 0, 0, -1, 0, 0),  # 15
        # Right face (x=h, normal right) — strip
        (h, y_bot, -h, 1, 0, 0, 0, 1),    # 16
        (h, y_bot,  h, 1, 0, 0, 1, 1),    # 17
        (h, y_top, -h, 1, 0, 0, 0, 0),    # 18
        (h, y_top,  h, 1, 0, 0, 1, 0),    # 19
        # Left face (x=-h, normal left) — strip
        (-h, y_bot, -h, -1, 0, 0, 1, 1),  # 20
        (-h, y_top, -h, -1, 0, 0, 1, 0),  # 21
        (-h, y_bot,  h, -1, 0, 0, 0, 1),  # 22
        (-h, y_top,  h, -1, 0, 0, 0, 0),  # 23
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    # Section 6: Octree
    w.write_cube(-h - 1, y_bot - 1, -h - 1, h + 1, y_top + 1, h + 1)
    w.write_u32(0)  # leaf
    w.write_u32(1)  # 1 geom

    # 6 strips, each with 2 tris and correct vertex_offset
    strips = [
        (2, 0),   # Top face: vertices 0-3
        (2, 4),   # Bottom face: vertices 4-7
        (2, 8),   # Front face: vertices 8-11
        (2, 12),  # Back face: vertices 12-15
        (2, 16),  # Right face: vertices 16-19
        (2, 20),  # Left face: vertices 20-23
    ]
    w.write_geom_with_strips(
        name="",
        ambient=(0.3, 0.35, 0.5, 1.0),
        diffuse=color,
        specular=(0.5, 0.5, 0.5, 1.0),
        emissive=(0.0, 0.0, 0.0, 1.0),
        power=10.0,
        strips=strips
    )

    return w.get_bytes()


def create_bowl_level(radius=300.0, depth=150.0, color=None, start_pos=None):
    """Create a MESHWORLD arena-style bowl level.

    Generates a hemispherical bowl with inward-facing normals, a closed bottom
    (triangle fan), and a rim edge. Uses D3D triangle strip format.

    Key design: bottom ring has small but nonzero radius to avoid degenerate
    triangles, plus a center vertex fan to close the bottom completely.
    The ball (radius ~26) cannot fall through the closed bottom.
    """
    w = MWWriter()

    if color is None:
        color = (0.78, 0.32, 0.80, 1.0)  # Purple-pink like Arena-WarmUp

    if start_pos is None:
        start_pos = (0, -depth + 26 + 1, 0)  # Ball center just above bowl floor

    # Sections 1-4
    _write_sections_1_to_4(w, start_pos=start_pos,
                           camera_pos=(0, -depth / 2, 0),
                           bg_color=(0.98, 0.46, 1.0),
                           ambient_color=(0.56, 0.56, 0.56))

    # Bowl parameters — use more rings for smoother curve and closed bottom
    n_rings = 12
    n_sectors = 16
    bottom_radius = 5.0  # Small but nonzero — avoids degenerate triangles

    # Generate ring positions (bowl interior)
    # Ring 0 = rim (radius=300, y=0)
    # Ring n_rings = bottom (radius=bottom_radius, y=-depth)
    rings = []  # rings[ring][sector] = (x, y, z, nx, ny, nz, u, v)
    for ring in range(n_rings + 1):
        t = ring / n_rings
        r = radius * (1.0 - t) + bottom_radius * t  # Linear interpolation to bottom_radius
        y = -depth * t
        ring_data = []
        for sector in range(n_sectors):
            angle = 2.0 * math.pi * sector / n_sectors
            x = r * math.cos(angle)
            z = r * math.sin(angle)
            # Inward-pointing normal (toward bowl center)
            if r > 0.01:
                nx = -x / r
                nz = -z / r
                ny = depth / (radius - bottom_radius)  # Approximate bowl slope
            else:
                nx = 0; ny = 1; nz = 0
            nl = math.sqrt(nx*nx + ny*ny + nz*nz)
            if nl > 0.001: nx/=nl; ny/=nl; nz/=nl
            ring_data.append((x, y, z, nx, ny, nz, sector/n_sectors, t))
        rings.append(ring_data)

    # Bottom center vertex (closes the bowl)
    center_y = -depth
    bottom_center = (0, center_y, 0, 0, 1, 0, 0.5, 1.0)

    # Rim top vertices (flat rim at y=10, slightly outside the bowl edge)
    rim_tops = []
    for sector in range(n_sectors):
        angle = 2.0 * math.pi * sector / n_sectors
        x = (radius + 15) * math.cos(angle)
        z = (radius + 15) * math.sin(angle)
        rim_tops.append((x, 10, z, 0, 1, 0, sector/n_sectors, 0))

    # Rim bottom vertices (at the bowl rim level, outside edge)
    rim_bottoms = []
    for sector in range(n_sectors):
        angle = 2.0 * math.pi * sector / n_sectors
        x = (radius + 15) * math.cos(angle)
        z = (radius + 15) * math.sin(angle)
        rim_bottoms.append((x, -5, z, 0, 1, 0, sector/n_sectors, 0))

    # Build vertex buffer as triangle strips
    all_vertices = []
    geom_strips = []
    current_offset = 0

    # Bowl wall bands: each ring-to-ring band is one strip
    for ring in range(n_rings):
        strip_verts = []
        for sector in range(n_sectors):
            strip_verts.append(rings[ring][sector])
            strip_verts.append(rings[ring + 1][sector])
        # Close the strip
        strip_verts.append(rings[ring][0])
        strip_verts.append(rings[ring + 1][0])

        n_tris = 2 * n_sectors
        for v in strip_verts:
            all_vertices.append(v)
        geom_strips.append((n_tris, current_offset))
        current_offset += len(strip_verts)

    # Bottom cap: triangle fan from bottom ring to center vertex
    # In strip format: center, ring[n_rings][0], ring[n_rings][1], center, ring[n_rings][1], ring[n_rings][2], ...
    # Actually, simpler: write each triangle of the fan as a separate strip (1 tri each)
    # Or use a single strip that zigzags: c, r[0], r[1], c, r[1], r[2], c, ...
    # Most compatible: write as individual 1-tri strips
    bottom_ring = rings[n_rings]
    center_offset = len(all_vertices)
    all_vertices.append(bottom_center)  # center vertex
    for sector in range(n_sectors):
        s_next = (sector + 1) % n_sectors
        # Each fan triangle: center, ring[sector], ring[sector+1]
        # As a 1-tri strip: needs 3 vertices
        v0 = bottom_center
        v1 = bottom_ring[sector]
        v2 = bottom_ring[s_next]
        # Add 3 vertices for this 1-tri strip
        for v in [v0, v1, v2]:
            all_vertices.append(v)
        geom_strips.append((1, current_offset))
        current_offset += 3

    # Rim edge: connect bowl rim (ring 0) to rim_bottom and rim_top
    # Outer wall strip: ring0 → rim_bottom (vertical wall)
    for sector in range(n_sectors):
        s_next = (sector + 1) % n_sectors
        v0 = rings[0][sector]
        v1 = rim_bottoms[sector]
        v2 = rings[0][s_next]
        v3 = rim_bottoms[s_next]
        for v in [v0, v1, v2, v3]:
            all_vertices.append(v)
        geom_strips.append((2, current_offset))
        current_offset += 4

    # Top rim strip: rim_bottom → rim_top (horizontal surface)
    for sector in range(n_sectors):
        s_next = (sector + 1) % n_sectors
        v0 = rim_bottoms[sector]
        v1 = rim_tops[sector]
        v2 = rim_bottoms[s_next]
        v3 = rim_tops[s_next]
        for v in [v0, v1, v2, v3]:
            all_vertices.append(v)
        geom_strips.append((2, current_offset))
        current_offset += 4

    # Write vertex buffer
    w.write_u32(len(all_vertices))
    for v in all_vertices:
        w.write_vertex(*v)

    # Section 6: Octree — single leaf with one geom
    w.write_cube(-radius - 20, -depth - 5, -radius - 20,
                 radius + 20, 15, radius + 20)
    w.write_u32(0)  # leaf
    w.write_u32(1)  # 1 geom

    w.write_geom_with_strips(
        name="",
        ambient=(0.4, 0.3, 0.4, 1.0),
        diffuse=color,
        specular=(0.6, 0.6, 0.6, 1.0),
        emissive=(0.1, 0.05, 0.1, 1.0),
        power=20.0,
        strips=geom_strips
    )

    return w.get_bytes()


def create_ramp_level(size=400.0, ramp_angle=15.0, color=None, start_pos=None):
    """Create a MESHWORLD level with a flat plane and a ramp.

    Two separate geoms, each with their own strips.
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

    # Geom 1: flat section — 4 vertices, 1 strip of 2 tris
    # Geom 2: ramp section — 4 vertices, 1 strip of 2 tris
    vertices = [
        # Flat section (strip: v0,v1,v2,v3)
        (-h/2, 0, -h, 0, 1, 0, 0, 1),  # 0
        (-h/2, 0,  0, 0, 1, 0, 0, 0),  # 1
        ( h/2, 0, -h, 0, 1, 0, 1, 1),  # 2
        ( h/2, 0,  0, 0, 1, 0, 1, 0),  # 3
        # Ramp section (strip: v0,v1,v2,v3)
        (-h/2, 0,          0, 0, ny, nz, 0, 1),  # 4
        (-h/2, ramp_height, h, 0, ny, nz, 0, 0),  # 5
        ( h/2, 0,          0, 0, ny, nz, 1, 1),  # 6
        ( h/2, ramp_height, h, 0, ny, nz, 1, 0),  # 7
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    w.write_cube(-h - 1, -1, -h - 1, h + 1, ramp_height + 1, h + 1)
    w.write_u32(0)
    w.write_u32(2)

    # Geom 1: flat (2 tris, offset 0)
    w.write_geom_with_strips("", ambient=(0.3, 0.35, 0.5, 1.0),
                              diffuse=(0.6, 0.7, 0.9, 1.0),
                              strips=[(2, 0)])
    # Geom 2: ramp (2 tris, offset 4)
    w.write_geom_with_strips("", ambient=(0.35, 0.25, 0.15, 1.0), diffuse=color,
                              strips=[(2, 4)])

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

    # Each surface is a 4-vertex strip (2 tris)
    vertices = [
        # Platform 1
        (-100, 0, -100, 0, 1, 0, 0, 1),   # 0
        (-100, 0,  100, 0, 1, 0, 0, 0),   # 1
        ( 100, 0, -100, 0, 1, 0, 1, 1),   # 2
        ( 100, 0,  100, 0, 1, 0, 1, 0),   # 3

        # Platform 2
        (-100, 50, 200, 0, 1, 0, 0, 1),   # 4
        (-100, 50, 400, 0, 1, 0, 0, 0),   # 5
        ( 100, 50, 200, 0, 1, 0, 1, 1),   # 6
        ( 100, 50, 400, 0, 1, 0, 1, 0),   # 7

        # Ramp
        (-100,  0, 100, 0, 0.894, -0.447, 0, 1),  # 8
        (-100, 50, 200, 0, 0.894, -0.447, 0, 0),  # 9
        ( 100,  0, 100, 0, 0.894, -0.447, 1, 1),  # 10
        ( 100, 50, 200, 0, 0.894, -0.447, 1, 0),  # 11
    ]

    w.write_u32(len(vertices))
    for v in vertices:
        w.write_vertex(*v)

    w.write_cube(-101, -1, -101, 101, 51, 401)
    w.write_u32(0)
    w.write_u32(3)

    w.write_geom_with_strips("", ambient=(0.2, 0.35, 0.2, 1.0), diffuse=color,
                              strips=[(2, 0)])
    w.write_geom_with_strips("", ambient=(0.2, 0.2, 0.35, 1.0),
                              diffuse=(0.4, 0.4, 0.8, 1.0),
                              strips=[(2, 4)])
    w.write_geom_with_strips("", ambient=(0.3, 0.3, 0.15, 1.0),
                              diffuse=(0.8, 0.7, 0.3, 1.0),
                              strips=[(2, 8)])

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
