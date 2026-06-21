#!/usr/bin/env python3
"""
Create a custom Arena MESHWORLD v2: Two large circular platforms with
C-shaped railings, connected by a wide bridge.

Fixes from v1:
  - START2-1 through START2-4 (arena spawn format, spread across platform)
  - Bigger platforms: radius 350 (was 200)
  - C-shaped railings on each platform (wall on 3 sides, open toward bridge)
  - Wider bridge: 120 units wide (was 80)
  - CAMERALOOKAT instead of CameraLocus1
  - Removed BADBALL ref points (game spawns 8-balls automatically in arena)
  - Added STANDS and SAFESPOT ref points
  - Player spawns spread across Platform A at ~150 unit intervals
"""

import struct
import math
from io import BytesIO


class MWWriter:
    def __init__(self):
        self.buf = BytesIO()

    def write_u32(self, val):
        self.buf.write(struct.pack('<I', val))

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
        for v in [x1, y1, z1, x2, y2, z2]:
            self.write_f32(v)

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


def make_cylinder_platform(cx, cy, cz, radius, thickness, n_segs=32,
                            diffuse_color=(0.8, 0.3, 0.5, 1.0)):
    """Generate a thick cylinder platform with top, bottom, and side wall."""
    verts = []
    strips = []
    offset = 0

    half_t = thickness / 2.0
    y_top = cy + half_t
    y_bot = cy - half_t

    # Top surface: triangle fan (center + rim)
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

    # Bottom surface
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
        verts.append(center_bot)
        verts.append(bot_rim[i_next])
        verts.append(bot_rim[i])
        strips.append((1, offset))
        offset += 3

    # Side wall
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

        verts.append((v0[0], v0[1], v0[2], nx, 0, nz, v0[6], v0[7]))
        verts.append((v1[0], v1[1], v1[2], nxn, 0, nzn, v1[6], v1[7]))
        verts.append((v2[0], v2[1], v2[2], nx, 0, nz, v2[6], v2[7]))
        strips.append((1, offset))
        offset += 3

        verts.append((v1[0], v1[1], v1[2], nxn, 0, nzn, v1[6], v1[7]))
        verts.append((v3[0], v3[1], v3[2], nxn, 0, nzn, v3[6], v3[7]))
        verts.append((v2[0], v2[1], v2[2], nx, 0, nz, v2[6], v2[7]))
        strips.append((1, offset))
        offset += 3

    return verts, strips, diffuse_color


def make_c_shaped_railing(cx, cy, cz, radius, railing_height, gap_angle_deg,
                          n_segs=40, diffuse_color=(0.5, 0.5, 0.6, 1.0)):
    """Generate a C-shaped railing around a platform.

    The railing is a wall on 3 sides of the platform, with a gap
    facing the bridge. The gap is centered on the angle pointing
    toward the bridge (0 for Platform A facing right, 180 for B facing left).

    The railing is a curved wall (arc segment) with thickness.

    cx, cy, cz: platform center
    radius: railing radius (slightly outside platform edge)
    railing_height: height of the wall above platform top
    gap_angle_deg: width of the gap in degrees (centered on bridge direction)
    n_segs: number of segments in the arc
    """
    verts = []
    strips = []
    offset = 0

    # The railing sits on top of the platform surface
    y_base = cy  # platform top
    y_top = cy + railing_height

    # Gap is centered on angle 0 (facing +X for Platform A)
    # For Platform B, we'll rotate the whole thing by 180°
    half_gap = math.radians(gap_angle_deg / 2)

    # The arc goes from half_gap to (2*pi - half_gap), skipping the gap
    # Generate inner and outer rim vertices
    inner_r = radius - 5  # railing thickness: 5 units inner
    outer_r = radius + 5  # 5 units outer

    # Build arc segments (only on the C-shape, not the gap)
    arc_angles = []
    for i in range(n_segs + 1):
        t = i / n_segs
        # Map from [0,1] to the arc range [half_gap, 2pi - half_gap]
        angle = half_gap + t * (2 * math.pi - 2 * half_gap)
        arc_angles.append(angle)

    # For each angle, create inner-top, outer-top, inner-bot, outer-bot verts
    for angle in arc_angles:
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)

        ix = cx + inner_r * cos_a
        iz = cz + inner_r * sin_a
        ox = cx + outer_r * cos_a
        oz = cz + outer_r * sin_a

        # Normal points outward
        nx = cos_a
        nz = sin_a

        verts.append((ix, y_top, iz, nx, 0, nz, 0, 0))  # inner top
        verts.append((ox, y_top, oz, nx, 0, nz, 1, 0))  # outer top
        verts.append((ix, y_base, iz, nx, 0, nz, 0, 1))  # inner bottom
        verts.append((ox, y_base, oz, nx, 0, nz, 1, 1))  # outer bottom

    # Build quads between consecutive angles
    # Each segment has: outer wall, inner wall, top cap, end caps
    for i in range(len(arc_angles) - 1):
        v_base = i * 4  # 4 verts per angle

        # Outer wall (faces outward)
        verts.append((
            cx + outer_r * math.cos(arc_angles[i]), y_top,
            cz + outer_r * math.sin(arc_angles[i]),
            math.cos(arc_angles[i]), 0, math.sin(arc_angles[i]), 0, 0))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i+1]), y_top,
            cz + outer_r * math.sin(arc_angles[i+1]),
            math.cos(arc_angles[i+1]), 0, math.sin(arc_angles[i+1]), 1, 0))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i]), y_base,
            cz + outer_r * math.sin(arc_angles[i]),
            math.cos(arc_angles[i]), 0, math.sin(arc_angles[i]), 0, 1))
        strips.append((2, offset))
        offset += 4

        verts.append((
            cx + outer_r * math.cos(arc_angles[i+1]), y_top,
            cz + outer_r * math.sin(arc_angles[i+1]),
            math.cos(arc_angles[i+1]), 0, math.sin(arc_angles[i+1]), 1, 0))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i+1]), y_base,
            cz + outer_r * math.sin(arc_angles[i+1]),
            math.cos(arc_angles[i+1]), 0, math.sin(arc_angles[i+1]), 1, 1))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i]), y_base,
            cz + outer_r * math.sin(arc_angles[i]),
            math.cos(arc_angles[i]), 0, math.sin(arc_angles[i]), 0, 1))
        strips.append((1, offset))
        offset += 3

        # Inner wall (faces inward)
        verts.append((
            cx + inner_r * math.cos(arc_angles[i]), y_top,
            cz + inner_r * math.sin(arc_angles[i]),
            -math.cos(arc_angles[i]), 0, -math.sin(arc_angles[i]), 0, 0))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i+1]), y_top,
            cz + inner_r * math.sin(arc_angles[i+1]),
            -math.cos(arc_angles[i+1]), 0, -math.sin(arc_angles[i+1]), 1, 0))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i]), y_base,
            cz + inner_r * math.sin(arc_angles[i]),
            -math.cos(arc_angles[i]), 0, -math.sin(arc_angles[i]), 0, 1))
        strips.append((1, offset))
        offset += 3

        verts.append((
            cx + inner_r * math.cos(arc_angles[i+1]), y_top,
            cz + inner_r * math.sin(arc_angles[i+1]),
            -math.cos(arc_angles[i+1]), 0, -math.sin(arc_angles[i+1]), 1, 0))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i]), y_base,
            cz + inner_r * math.sin(arc_angles[i]),
            -math.cos(arc_angles[i]), 0, -math.sin(arc_angles[i]), 0, 1))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i+1]), y_base,
            cz + inner_r * math.sin(arc_angles[i+1]),
            -math.cos(arc_angles[i+1]), 0, -math.sin(arc_angles[i+1]), 1, 1))
        strips.append((1, offset))
        offset += 3

        # Top cap (flat surface on top of railing)
        verts.append((
            cx + inner_r * math.cos(arc_angles[i]), y_top,
            cz + inner_r * math.sin(arc_angles[i]),
            0, 1, 0, 0, 0))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i]), y_top,
            cz + outer_r * math.sin(arc_angles[i]),
            0, 1, 0, 1, 0))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i+1]), y_top,
            cz + inner_r * math.sin(arc_angles[i+1]),
            0, 1, 0, 0, 1))
        strips.append((1, offset))
        offset += 3

        verts.append((
            cx + outer_r * math.cos(arc_angles[i]), y_top,
            cz + outer_r * math.sin(arc_angles[i]),
            0, 1, 0, 1, 0))
        verts.append((
            cx + outer_r * math.cos(arc_angles[i+1]), y_top,
            cz + outer_r * math.sin(arc_angles[i+1]),
            0, 1, 0, 1, 1))
        verts.append((
            cx + inner_r * math.cos(arc_angles[i+1]), y_top,
            cz + inner_r * math.sin(arc_angles[i+1]),
            0, 1, 0, 0, 1))
        strips.append((1, offset))
        offset += 3

    return verts, strips, diffuse_color


def make_box_geometry(x1, y1, z1, x2, y2, z2, diffuse_color=(0.6, 0.5, 0.4, 1.0)):
    """Generate a thick box (6 faces, 24 vertices, 12 triangles)."""
    verts = []
    strips = []
    offset = 0

    faces = [
        # (verts, normal)
        [(x1, y2, z1, 0, 1, 0, 0, 1), (x2, y2, z1, 0, 1, 0, 1, 1),
         (x1, y2, z2, 0, 1, 0, 0, 0), (x2, y2, z2, 0, 1, 0, 1, 0)],  # top
        [(x1, y1, z1, 0, -1, 0, 0, 1), (x1, y1, z2, 0, -1, 0, 0, 0),
         (x2, y1, z1, 0, -1, 0, 1, 1), (x2, y1, z2, 0, -1, 0, 1, 0)],  # bottom
        [(x1, y1, z2, 0, 0, 1, 0, 1), (x1, y2, z2, 0, 0, 1, 0, 0),
         (x2, y1, z2, 0, 0, 1, 1, 1), (x2, y2, z2, 0, 0, 1, 1, 0)],  # front
        [(x1, y1, z1, 0, 0, -1, 1, 1), (x2, y1, z1, 0, 0, -1, 0, 1),
         (x1, y2, z1, 0, 0, -1, 1, 0), (x2, y2, z1, 0, 0, -1, 0, 0)],  # back
        [(x2, y1, z1, 1, 0, 0, 0, 1), (x2, y1, z2, 1, 0, 0, 1, 1),
         (x2, y2, z1, 1, 0, 0, 0, 0), (x2, y2, z2, 1, 0, 0, 1, 0)],  # right
        [(x1, y1, z1, -1, 0, 0, 1, 1), (x1, y1, z2, -1, 0, 0, 0, 1),
         (x1, y2, z1, -1, 0, 0, 1, 0), (x1, y2, z2, -1, 0, 0, 0, 0)],  # left
    ]

    for face_verts in faces:
        verts.extend(face_verts)
        strips.append((2, offset))
        offset += 4

    return verts, strips, diffuse_color


def create_dual_platform_arena_v2():
    """Create the v2 arena with bigger platforms, C-railings, and proper spawns."""
    w = MWWriter()

    # Layout parameters (v2 — bigger!)
    platform_radius = 350.0    # was 200
    platform_thickness = 30.0
    platform_y = 0.0
    platform_sep = 600.0       # was 350 — more separation for bigger platforms

    plat_a_cx = -platform_sep
    plat_a_cz = 0.0
    plat_b_cx = platform_sep
    plat_b_cz = 0.0

    y_top = platform_y + platform_thickness / 2.0   # 15.0
    y_bot = platform_y - platform_thickness / 2.0   # -15.0

    # Railing parameters
    railing_height = 40.0
    railing_r = platform_radius + 8  # slightly outside platform edge
    gap_angle = 60.0  # 60-degree gap facing the bridge

    # Bridge parameters — wider!
    bridge_half_width = 120.0   # was 60 — 240 total width
    bridge_thickness = 20.0
    bridge_y_top = y_top
    bridge_y_bot = y_top - bridge_thickness

    # Player spawn points — START2-1 through START2-4, spread across Platform A
    # Like original: 4 corners at ~150 unit intervals from center
    spawn_r = 150.0  # spawn radius from platform center
    spawns = [
        ("START2-1", plat_a_cx - spawn_r, y_top + 52, -spawn_r),   # back-left
        ("START2-2", plat_a_cx + spawn_r, y_top + 52, spawn_r),     # front-right
        ("START2-3", plat_a_cx + spawn_r, y_top + 52, -spawn_r),    # back-right
        ("START2-4", plat_a_cx - spawn_r, y_top + 52, spawn_r),     # front-left
    ]

    # ================================================================
    # Section 1: Ref Points (matching original arena format)
    # ================================================================
    ref_points = [
        # Player spawns — spread across Platform A
        (spawns[0][0], (spawns[0][1], spawns[0][2], spawns[0][3]), (0, 0, 0)),
        (spawns[1][0], (spawns[1][1], spawns[1][2], spawns[1][3]), (0, 0, 0)),
        (spawns[2][0], (spawns[2][1], spawns[2][2], spawns[2][3]), (0, 0, 0)),
        (spawns[3][0], (spawns[3][1], spawns[3][2], spawns[3][3]), (0, 0, 0)),
        # Camera lookat — center between platforms
        ("CAMERALOOKAT", (0, 0, 0), (0, 0, 0)),
        # Platform ref points (used by arena for spawn platform positions)
        ("PLATFORM", (plat_a_cx, y_top + 800, 0), (0, 0, 0)),
        ("PLATFORM", (plat_b_cx, y_top + 800, 0), (0, 0, 0)),
        ("PLATFORM", (plat_a_cx + spawn_r, y_top + 800, spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_a_cx - spawn_r, y_top + 800, spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_a_cx + spawn_r, y_top + 800, -spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_a_cx - spawn_r, y_top + 800, -spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_b_cx + spawn_r, y_top + 800, spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_b_cx - spawn_r, y_top + 800, spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_b_cx + spawn_r, y_top + 800, -spawn_r), (0, 0, 0)),
        ("PLATFORM", (plat_b_cx - spawn_r, y_top + 800, -spawn_r), (0, 0, 0)),
        # Safespots — respawn safety points
        ("SAFESPOT", (plat_a_cx, y_top + 13, 0), (0, 0, 0)),
        ("SAFESPOT", (plat_b_cx, y_top + 13, 0), (0, 0, 0)),
        ("SAFESPOT", (0, y_top + 13, 0), (0, 0, 0)),  # bridge center
        # Stands (arena spectator stands)
        ("STANDS", (0, -776, 0), (0, 0, 0)),
    ]

    w.write_u32(len(ref_points))
    for name, pos, rot in ref_points:
        w.write_string(name)
        w.write_vec3(*pos)
        w.write_f32(rot[0]); w.write_f32(rot[1]); w.write_f32(rot[2])
        w.write_u32(0)  # no material

    # Section 2: Splines (none)
    w.write_u32(0)

    # Section 3: Lights
    w.write_u32(1)
    w.write_u32(0)  # DISTANTLIGHT
    w.write_vec3(100, 500, 100)
    w.write_vec3(0, 0, 0)
    w.write_f32(1.0); w.write_f32(1.0); w.write_f32(1.0)

    # Section 4: Background & Ambient
    w.write_f32(0.98); w.write_f32(0.46); w.write_f32(1.0)   # pink-ish bg
    w.write_f32(0.56); w.write_f32(0.56); w.write_f32(0.56)  # ambient

    # ================================================================
    # Section 5: Global Vertex Buffer
    # ================================================================
    all_verts = []
    all_geoms = []
    current_offset = 0

    # Platform A (pink) — bigger!
    pa_verts, pa_strips, pa_color = make_cylinder_platform(
        plat_a_cx, platform_y, plat_a_cz, platform_radius, platform_thickness,
        n_segs=32, diffuse_color=(0.99, 0.63, 1.0, 1.0))
    pa_base = current_offset
    all_verts.extend(pa_verts)
    pa_strips_adj = [(tc, vo + pa_base) for tc, vo in pa_strips]
    all_geoms.append((pa_strips_adj, pa_color, ""))
    current_offset += len(pa_verts)

    # Platform B (blue)
    pb_verts, pb_strips, pb_color = make_cylinder_platform(
        plat_b_cx, platform_y, plat_b_cz, platform_radius, platform_thickness,
        n_segs=32, diffuse_color=(0.42, 0.62, 0.91, 1.0))
    pb_base = current_offset
    all_verts.extend(pb_verts)
    pb_strips_adj = [(tc, vo + pb_base) for tc, vo in pb_strips]
    all_geoms.append((pb_strips_adj, pb_color, ""))
    current_offset += len(pb_verts)

    # C-shaped railing on Platform A (gap faces +X / toward bridge)
    ra_verts, ra_strips, ra_color = make_c_shaped_railing(
        plat_a_cx, y_top, plat_a_cz, railing_r, railing_height,
        gap_angle_deg=gap_angle, n_segs=36,
        diffuse_color=(0.7, 0.7, 0.8, 1.0))  # light gray
    ra_base = current_offset
    all_verts.extend(ra_verts)
    ra_strips_adj = [(tc, vo + ra_base) for tc, vo in ra_strips]
    all_geoms.append((ra_strips_adj, ra_color, ""))
    current_offset += len(ra_verts)

    # C-shaped railing on Platform B (gap faces -X / toward bridge)
    # For Platform B, we need to rotate the gap by 180°
    # We do this by negating cx offset — but actually the gap is always
    # centered on angle 0 (facing +X). For Platform B at +600, the bridge
    # is to the LEFT (-X direction). So we need the gap facing -X.
    # Solution: generate the railing with a custom gap direction.
    # Actually, make_c_shaped_railing centers the gap on angle 0 (+X).
    # For Platform B, we want the gap on angle 180° (-X).
    # Quick fix: generate for a platform at -cx, then translate.
    # Better: modify the function to accept a gap_direction angle.
    # For now, generate at origin facing +X, then rotate 180° and translate.
    rb_verts_raw, rb_strips_raw, rb_color = make_c_shaped_railing(
        0, y_top, 0, railing_r, railing_height,
        gap_angle_deg=gap_angle, n_segs=36,
        diffuse_color=(0.7, 0.7, 0.8, 1.0))
    # Rotate 180° around Y: (x,z) -> (-x,-z), normal (nx,nz) -> (-nx,-nz)
    rb_verts = []
    for v in rb_verts_raw:
        x, y, z, nx, ny, nz, u, uv = v
        rb_verts.append((-x + plat_b_cx, y, -z + plat_b_cz, -nx, ny, -nz, u, uv))
    rb_base = current_offset
    all_verts.extend(rb_verts)
    rb_strips_adj = [(tc, vo + rb_base) for tc, vo in rb_strips_raw]
    all_geoms.append((rb_strips_adj, rb_color, ""))
    current_offset += len(rb_verts)

    # Bridge (brown/gray) — wider!
    bridge_x1 = plat_a_cx + platform_radius - 10
    bridge_x2 = plat_b_cx - platform_radius + 10
    bridge_z1 = -bridge_half_width
    bridge_z2 = bridge_half_width
    br_verts, br_strips, br_color = make_box_geometry(
        bridge_x1, bridge_y_bot, bridge_z1,
        bridge_x2, bridge_y_top, bridge_z2,
        diffuse_color=(0.55, 0.42, 0.30, 1.0))
    br_base = current_offset
    all_verts.extend(br_verts)
    br_strips_adj = [(tc, vo + br_base) for tc, vo in br_strips]
    all_geoms.append((br_strips_adj, br_color, ""))
    current_offset += len(br_verts)

    # Write vertex count + vertices
    w.write_u32(len(all_verts))
    for v in all_verts:
        w.write_vertex(*v)

    # ================================================================
    # Section 6: Octree — single leaf
    # ================================================================
    bounds_x1 = plat_a_cx - platform_radius - 30
    bounds_x2 = plat_b_cx + platform_radius + 30
    bounds_y1 = y_bot - 5
    bounds_y2 = y_top + railing_height + 5
    bounds_z1 = -platform_radius - 30
    bounds_z2 = platform_radius + 30
    w.write_cube(bounds_x1, bounds_y1, bounds_z1,
                 bounds_x2, bounds_y2, bounds_z2)
    w.write_u32(0)  # leaf
    w.write_u32(len(all_geoms))

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
    out = sys.argv[1] if len(sys.argv) > 1 else 'DualPlatformArenaV2.MESHWORLD'
    data = create_dual_platform_arena_v2()
    with open(out, 'wb') as f:
        f.write(data)
    print(f"Written {len(data)} bytes to {out}")
    print(f"Layout: 2 large circular platforms (r=350) with C-railings + wide bridge")
    print(f"Platform A: pink at (-600, 0, 0), gap facing +X (bridge)")
    print(f"Platform B: blue at (+600, 0, 0), gap facing -X (bridge)")
    print(f"Bridge: width=240, from x=-240 to x=240")
    print(f"Railings: C-shaped, 40 units high, 60° gap toward bridge")
    print(f"Player spawns: START2-1 to START2-4 spread across Platform A")
    print(f"  START2-1: (-750, 67, -150)")
    print(f"  START2-2: (-450, 67, +150)")
    print(f"  START2-3: (-450, 67, -150)")
    print(f"  START2-4: (-750, 67, +150)")
