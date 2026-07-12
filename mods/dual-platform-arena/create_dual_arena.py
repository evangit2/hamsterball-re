#!/usr/bin/env python3
"""
Create a custom Arena MESHWORLD: Two circular platforms connected by a bridge.

Replaces Warm-Up Arena (Arena1.MESHWORLD). Arena layout:
  - Platform A: circle radius 200, center at (-350, 0, 0), pink material
  - Platform B: circle radius 200, center at (+350, 0, 0), blue material  
  - Bridge: rectangular, connects the two platforms, brown/gray material
  - Player start: on Platform A
  - BADBALL spawn: one on each platform
  - CameraLocus: centered between platforms

Each platform is a thick cylinder (top + bottom + side wall) for robust collision.
The bridge is a thick box.
"""

import struct
import math
from io import BytesIO


class MWWriter:
    def __init__(self):
        self.buf = BytesIO()

    def write_u32(self, val):
        self.buf.write(struct.pack('<I', val))

    def write_i32(self, val):
        self.buf.write(struct.pack('<i', val))

    def write_f32(self, val):
        self.buf.write(struct.pack('<f', val))

    def write_string(self, s):
        encoded = s.encode('ascii') + b'\x00'
        self.write_u32(len(encoded))
        self.buf.write(encoded)

    def write_vec3(self, x, y, z):
        self.write_f32(x)
        self.write_f32(y)
        self.write_f32(z)

    def write_material(self, diffuse=None, ambient=None, specular=None, emissive=None,
                       power=10.0, has_reflection=0, texture=None):
        def write_color4(c):
            if c is None:
                self.buf.write(struct.pack('<4f', 0.8, 0.8, 0.8, 1.0))
            else:
                self.buf.write(struct.pack('<4f', *c))
        write_color4(ambient)
        write_color4(diffuse)
        write_color4(specular)
        write_color4(emissive)
        self.write_f32(power)
        self.write_u32(has_reflection)
        if texture:
            self.write_u32(1)
            self.write_string(texture)
        else:
            self.write_u32(0)

    def write_vertex(self, x, y, z, nx=0, ny=1, nz=0, u=0, v=0):
        self.write_f32(x)
        self.write_f32(y)
        self.write_f32(z)
        self.write_f32(nx)
        self.write_f32(ny)
        self.write_f32(nz)
        self.write_f32(u)
        self.write_f32(v)

    def write_cube(self, x1, y1, z1, x2, y2, z2):
        self.write_f32(x1)
        self.write_f32(y1)
        self.write_f32(z1)
        self.write_f32(x2)
        self.write_f32(y2)
        self.write_f32(z2)

    def write_geom_with_strips(self, name="", diffuse=None, ambient=None, specular=None,
                                emissive=None, power=10.0, strips=None):
        self.write_string(name)
        self.write_material(ambient=ambient, diffuse=diffuse, specular=specular,
                            emissive=emissive, power=power, has_reflection=0, texture=None)
        self.write_u32(len(strips) if strips else 0)
        if strips:
            for tri_count, vertex_offset in strips:
                self.write_u32(tri_count)
                self.write_u32(vertex_offset)

    def get_bytes(self):
        return self.buf.getvalue()


def make_cylinder_platform(cx, cy, cz, radius, thickness, n_segs=24,
                            diffuse_color=(0.8, 0.3, 0.5, 1.0)):
    """Generate vertices and strips for a thick cylinder platform.

    Returns (vertices_list, strips_list) where:
    - vertices_list: list of (x,y,z,nx,ny,nz,u,v) tuples
    - strips_list: list of (tri_count, vertex_offset) tuples

    The cylinder has:
    - Top surface (circle at y=cy+thickness/2, normal up)
    - Bottom surface (circle at y=cy-thickness/2, normal down)
    - Side wall (connecting top and bottom rims, normal outward)
    """
    verts = []
    strips = []
    offset = 0

    half_t = thickness / 2.0
    y_top = cy + half_t
    y_bot = cy - half_t

    # --- Top surface: triangle fan from center ---
    # Center vertex
    verts.append((cx, y_top, cz, 0, 1, 0, 0.5, 0.5))
    # Rim vertices (top)
    for i in range(n_segs):
        angle = 2.0 * math.pi * i / n_segs
        x = cx + radius * math.cos(angle)
        z = cz + radius * math.sin(angle)
        u = 0.5 + 0.5 * math.cos(angle)
        v = 0.5 + 0.5 * math.sin(angle)
        verts.append((x, y_top, z, 0, 1, 0, u, v))

    # Top fan strips: each triangle = center, rim[i], rim[i+1]
    for i in range(n_segs):
        i_next = (i + 1) % n_segs
        # 3 verts per tri (strip with 1 tri = 3 verts)
        v0 = 0  # center
        v1 = 1 + i
        v2 = 1 + i_next
        # We already have these verts in the buffer, so reference them
        # But strips reference consecutive verts... we need to duplicate
        # Actually, the MW strip format references the global vertex buffer.
        # A strip with tri_count=1 consumes 3 consecutive vertices.
        # So we need to add 3 verts per triangle (duplicated from the fan).
        pass

    # Actually, let's use a simpler approach: build the top as a strip of
    # (center, rim[0], rim[1], center, rim[1], rim[2], ...) — no, that wastes verts.
    # Simplest proven approach from mw_create.py: write each triangle as a
    # 1-tri strip with 3 consecutive vertices. This is wasteful but WORKS.

    # Reset — build all geometry as flat triangle lists (1-tri strips)
    verts = []
    strips = []
    offset = 0

    # --- Top surface ---
    center_top = (cx, y_top, cz, 0, 1, 0, 0.5, 0.5)
    top_rim = []
    for i in range(n_segs):
        angle = 2.0 * math.pi * i / n_segs
        x = cx + radius * math.cos(angle)
        z = cz + radius * math.sin(angle)
        u = 0.5 + 0.5 * math.cos(angle)
        v = 0.5 + 0.5 * math.sin(angle)
        top_rim.append((x, y_top, z, 0, 1, 0, u, v))

    for i in range(n_segs):
        i_next = (i + 1) % n_segs
        verts.append(center_top)
        verts.append(top_rim[i])
        verts.append(top_rim[i_next])
        strips.append((1, offset))
        offset += 3

    # --- Bottom surface (normal down) ---
    center_bot = (cx, y_bot, cz, 0, -1, 0, 0.5, 0.5)
    bot_rim = []
    for i in range(n_segs):
        angle = 2.0 * math.pi * i / n_segs
        x = cx + radius * math.cos(angle)
        z = cz + radius * math.sin(angle)
        u = 0.5 + 0.5 * math.cos(angle)
        v = 0.5 + 0.5 * math.sin(angle)
        bot_rim.append((x, y_bot, z, 0, -1, 0, u, v))

    for i in range(n_segs):
        i_next = (i + 1) % n_segs
        # Reverse winding for bottom face (visible from below)
        verts.append(center_bot)
        verts.append(bot_rim[i_next])
        verts.append(bot_rim[i])
        strips.append((1, offset))
        offset += 3

    # --- Side wall ---
    for i in range(n_segs):
        i_next = (i + 1) % n_segs
        angle = 2.0 * math.pi * i / n_segs
        angle_next = 2.0 * math.pi * (i + 1) / n_segs
        nx = math.cos(angle)
        nz = math.sin(angle)
        nxn = math.cos(angle_next)
        nzn = math.sin(angle_next)

        v0 = top_rim[i]
        v1 = top_rim[i_next]
        v2 = bot_rim[i]
        v3 = bot_rim[i_next]

        # Triangle 1: v0, v1, v2
        verts.append((v0[0], v0[1], v0[2], nx, 0, nz, v0[6], v0[7]))
        verts.append((v1[0], v1[1], v1[2], nxn, 0, nzn, v1[6], v1[7]))
        verts.append((v2[0], v2[1], v2[2], nx, 0, nz, v2[6], v2[7]))
        strips.append((1, offset))
        offset += 3

        # Triangle 2: v1, v3, v2
        verts.append((v1[0], v1[1], v1[2], nxn, 0, nzn, v1[6], v1[7]))
        verts.append((v3[0], v3[1], v3[2], nxn, 0, nzn, v3[6], v3[7]))
        verts.append((v2[0], v2[1], v2[2], nx, 0, nz, v2[6], v2[7]))
        strips.append((1, offset))
        offset += 3

    return verts, strips, diffuse_color


def make_box_geometry(x1, y1, z1, x2, y2, z2, diffuse_color=(0.6, 0.5, 0.4, 1.0)):
    """Generate vertices and strips for a thick box.

    Returns (vertices_list, strips_list, diffuse_color).
    Uses the same proven format as create_plane_level: 6 faces × 2 tris each.
    """
    verts = []
    strips = []
    offset = 0

    # Top face (y=y2, normal up)
    verts.extend([
        (x1, y2, z1, 0, 1, 0, 0, 1),
        (x2, y2, z1, 0, 1, 0, 1, 1),
        (x1, y2, z2, 0, 1, 0, 0, 0),
        (x2, y2, z2, 0, 1, 0, 1, 0),
    ])
    strips.append((2, offset)); offset += 4

    # Bottom face (y=y1, normal down)
    verts.extend([
        (x1, y1, z1, 0, -1, 0, 0, 1),
        (x1, y1, z2, 0, -1, 0, 0, 0),
        (x2, y1, z1, 0, -1, 0, 1, 1),
        (x2, y1, z2, 0, -1, 0, 1, 0),
    ])
    strips.append((2, offset)); offset += 4

    # Front face (z=z2, normal forward)
    verts.extend([
        (x1, y1, z2, 0, 0, 1, 0, 1),
        (x1, y2, z2, 0, 0, 1, 0, 0),
        (x2, y1, z2, 0, 0, 1, 1, 1),
        (x2, y2, z2, 0, 0, 1, 1, 0),
    ])
    strips.append((2, offset)); offset += 4

    # Back face (z=z1, normal backward)
    verts.extend([
        (x1, y1, z1, 0, 0, -1, 1, 1),
        (x2, y1, z1, 0, 0, -1, 0, 1),
        (x1, y2, z1, 0, 0, -1, 1, 0),
        (x2, y2, z1, 0, 0, -1, 0, 0),
    ])
    strips.append((2, offset)); offset += 4

    # Right face (x=x2, normal right)
    verts.extend([
        (x2, y1, z1, 1, 0, 0, 0, 1),
        (x2, y1, z2, 1, 0, 0, 1, 1),
        (x2, y2, z1, 1, 0, 0, 0, 0),
        (x2, y2, z2, 1, 0, 0, 1, 0),
    ])
    strips.append((2, offset)); offset += 4

    # Left face (x=x1, normal left)
    verts.extend([
        (x1, y1, z1, -1, 0, 0, 1, 1),
        (x1, y1, z2, -1, 0, 0, 0, 1),
        (x1, y2, z1, -1, 0, 0, 1, 0),
        (x1, y2, z2, -1, 0, 0, 0, 0),
    ])
    strips.append((2, offset)); offset += 4

    return verts, strips, diffuse_color


def create_dual_platform_arena():
    """Create the custom arena: 2 circular platforms + bridge."""
    w = MWWriter()

    # Layout parameters
    platform_radius = 200.0
    platform_thickness = 30.0
    platform_y = 0.0  # center of platform thickness
    platform_sep = 350.0  # distance from center to each platform center

    plat_a_cx = -platform_sep
    plat_a_cz = 0.0
    plat_b_cx = platform_sep
    plat_b_cz = 0.0

    y_top = platform_y + platform_thickness / 2.0  # = 15.0
    y_bot = platform_y - platform_thickness / 2.0  # = -15.0

    # Bridge parameters — connects inner edges of both platforms
    bridge_half_width = 60.0  # X-extent of bridge from center
    bridge_half_depth = 40.0  # Z-extent (width of bridge)
    bridge_thickness = 20.0
    bridge_y_top = y_top  # flush with platform tops
    bridge_y_bot = y_top - bridge_thickness  # hangs below platform top

    # Player start — on Platform A, above surface
    player_start = (plat_a_cx, y_top + 52, 0.0)

    # Camera focus — center between platforms
    camera_pos = (0, 50, 0)

    # 8-ball spawn points — one on each platform, away from player
    badball_a_pos = (plat_a_cx + 80, y_top + 52, 80)
    badball_b_pos = (plat_b_cx - 80, y_top + 52, -80)

    # ================================================================
    # Section 1: Ref Points
    # ================================================================
    ref_points = [
        # Player start
        ("START1-1", player_start, (0, 0, 0)),
        # Camera locus
        ("CameraLocus1", camera_pos, (0, 0, 0)),
        # 8-ball spawn on Platform A
        ("BADBALL", badball_a_pos, (0, 0, 0)),
        # 8-ball spawn on Platform B
        ("BADBALL", badball_b_pos, (0, 0, 0)),
    ]

    w.write_u32(len(ref_points))
    for name, pos, rot in ref_points:
        w.write_string(name)
        w.write_vec3(*pos)
        w.write_f32(rot[0]); w.write_f32(rot[1]); w.write_f32(rot[2])
        w.write_u32(0)  # no material on ref points

    # ================================================================
    # Section 2: Splines (none)
    # ================================================================
    w.write_u32(0)

    # ================================================================
    # Section 3: Lights
    # ================================================================
    w.write_u32(1)  # 1 light
    w.write_u32(0)  # DISTANTLIGHT type
    w.write_vec3(100, 500, 100)   # light position/direction
    w.write_vec3(0, 0, 0)         # light lookat
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)  # color

    # ================================================================
    # Section 4: Background & Ambient
    # ================================================================
    # Sky blue background, warm ambient
    w.write_f32(0.33); w.write_f32(0.47); w.write_f32(0.84)  # bg color
    w.write_f32(0.35); w.write_f32(0.35); w.write_f32(0.40)  # ambient

    # ================================================================
    # Section 5: Global Vertex Buffer
    # ================================================================
    all_verts = []
    all_geoms = []  # list of (strips_list, diffuse_color, name)
    current_offset = 0

    # Platform A (pink)
    pa_verts, pa_strips, pa_color = make_cylinder_platform(
        plat_a_cx, platform_y, plat_a_cz, platform_radius, platform_thickness,
        n_segs=24, diffuse_color=(0.99, 0.63, 1.0, 1.0))
    pa_base = current_offset
    for v in pa_verts:
        all_verts.append(v)
    pa_strips_adj = [(tc, vo + pa_base) for tc, vo in pa_strips]
    all_geoms.append((pa_strips_adj, pa_color, ""))
    current_offset += len(pa_verts)

    # Platform B (blue)
    pb_verts, pb_strips, pb_color = make_cylinder_platform(
        plat_b_cx, platform_y, plat_b_cz, platform_radius, platform_thickness,
        n_segs=24, diffuse_color=(0.42, 0.62, 0.91, 1.0))
    pb_base = current_offset
    for v in pb_verts:
        all_verts.append(v)
    pb_strips_adj = [(tc, vo + pb_base) for tc, vo in pb_strips]
    all_geoms.append((pb_strips_adj, pb_color, ""))
    current_offset += len(pb_verts)

    # Bridge (brown/gray)
    # Bridge spans from plat_a inner edge to plat_b inner edge
    # Inner edge of A: x = plat_a_cx + platform_radius = -350 + 200 = -150
    # Inner edge of B: x = plat_b_cx - platform_radius = 350 - 200 = 150
    bridge_x1 = plat_a_cx + platform_radius - 10  # slight overlap with platform
    bridge_x2 = plat_b_cx - platform_radius + 10
    bridge_z1 = -bridge_half_depth
    bridge_z2 = bridge_half_depth
    bridge_y1 = bridge_y_bot
    bridge_y2 = bridge_y_top

    br_verts, br_strips, br_color = make_box_geometry(
        bridge_x1, bridge_y1, bridge_z1,
        bridge_x2, bridge_y2, bridge_z2,
        diffuse_color=(0.55, 0.42, 0.30, 1.0))
    br_base = current_offset
    for v in br_verts:
        all_verts.append(v)
    br_strips_adj = [(tc, vo + br_base) for tc, vo in br_strips]
    all_geoms.append((br_strips_adj, br_color, ""))
    current_offset += len(br_verts)

    # Write vertex count + vertices
    w.write_u32(len(all_verts))
    for v in all_verts:
        w.write_vertex(*v)

    # ================================================================
    # Section 6: Octree — single leaf containing everything
    # ================================================================
    bounds_x1 = plat_a_cx - platform_radius - 20
    bounds_x2 = plat_b_cx + platform_radius + 20
    bounds_y1 = y_bot - 5
    bounds_y2 = y_top + 5
    bounds_z1 = -platform_radius - 20
    bounds_z2 = platform_radius + 20
    w.write_cube(bounds_x1, bounds_y1, bounds_z1,
                 bounds_x2, bounds_y2, bounds_z2)
    w.write_u32(0)  # leaf (no subdivision)
    w.write_u32(len(all_geoms))  # 3 geoms

    for strips, color, name in all_geoms:
        w.write_geom_with_strips(
            name=name,
            ambient=(0.3, 0.3, 0.35, 1.0),
            diffuse=color,
            specular=(0.5, 0.5, 0.5, 1.0),
            emissive=(0.05, 0.05, 0.05, 1.0),
            power=20.0,
            strips=strips)

    return w.get_bytes()


if __name__ == '__main__':
    import sys
    out = sys.argv[1] if len(sys.argv) > 1 else 'DualPlatformArena.MESHWORLD'
    data = create_dual_platform_arena()
    with open(out, 'wb') as f:
        f.write(data)
    n_verts = len(data) // 32  # rough estimate
    print(f"Written {len(data)} bytes to {out}")
    print(f"Layout: 2 circular platforms (r=200) + bridge connecting them")
    print(f"Platform A: pink at (-350, 0, 0)")
    print(f"Platform B: blue at (+350, 0, 0)")
    print(f"Bridge: from x=-150 to x=150, y_top=15")
    print(f"Player start: (-350, 67, 0)")
    print(f"8-ball A: (-270, 67, 80)")
    print(f"8-ball B: (+270, 67, -80)")
