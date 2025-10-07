#!/usr/bin/env python3
"""Fix neon-city and fire-ice icon packs with correct colors"""

import subprocess
from pathlib import Path

OUTPUT_DIR = Path(r"C:\R\GitScribe\gitscribe-shell\resources\icon-packs")

# Neon City - solid neon colors
NEON_CITY = {
    'added': ('#00FFFF', '#00FFFF'),
    'clean': ('#39FF14', '#39FF14'),
    'conflicted': ('#FF073A', '#FF073A'),
    'ignored': ('#9D00FF', '#9D00FF'),
    'modified': ('#FF6600', '#FF6600'),
    'untracked': ('#FFD700', '#FFD700')
}

# Fire & Ice - hot and cold gradients
FIRE_ICE = {
    'added': ('#00FFFF', '#00BFFF'),         # Ice cyan
    'clean': ('#7FFFD4', '#40E0D0'),          # Aquamarine
    'conflicted': ('#FF6347', '#FF4500'),     # Fire red/orange
    'ignored': ('#BA55D3', '#8A2BE2'),        # Purple
    'modified': ('#FF8C00', '#FFA500'),       # Orange fire
    'untracked': ('#87CEFA', '#4682B4')       # Light blue ice
}

SVG_PATHS = {
    'added': '<path d="M3 12a9 9 0 1 0 18 0a9 9 0 0 0 -18 0" />\n    <path d="M9 12h6" />\n    <path d="M12 9v6" />',
    'clean': '<path d="M3 12a9 9 0 1 0 18 0a9 9 0 0 0 -18 0" />\n    <path d="M9 12l2 2l4 -4" />',
    'conflicted': '<path d="M12 9v4" />\n    <path d="M10.363 3.591l-8.106 13.534a1.914 1.914 0 0 0 1.636 2.871h16.214a1.914 1.914 0 0 0 1.636 -2.87l-8.106 -13.536a1.914 1.914 0 0 0 -3.274 0z" />\n    <path d="M12 16h.01" />',
    'ignored': '<path d="M10.585 10.587a2 2 0 0 0 2.829 2.828" />\n    <path d="M16.681 16.673a8.717 8.717 0 0 1 -4.681 1.327c-3.6 0 -6.6 -2 -9 -6c1.272 -2.12 2.712 -3.678 4.32 -4.674m2.86 -1.146a9.055 9.055 0 0 1 1.82 -.18c3.6 0 6.6 2 9 6c-.666 1.11 -1.379 2.067 -2.138 2.87" />\n    <path d="M3 3l18 18" />',
    'modified': '<path d="M4 20h4l10.5 -10.5a2.828 2.828 0 1 0 -4 -4l-10.5 10.5v4" />\n    <path d="M13.5 6.5l4 4" />',
    'untracked': '<path d="M10 12a2 2 0 1 0 4 0a2 2 0 0 0 -4 0" />\n    <path d="M21 12c-2.4 4 -5.4 6 -9 6c-3.6 0 -6.6 -2 -9 -6c2.4 -4 5.4 -6 9 -6c3.6 0 6.6 2 9 6" />'
}

def create_svg(pack_dir, status, color1, color2, svg_paths):
    """Create SVG file with gradient"""
    svg_content = f'''<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 128 128">
  <defs>
    <linearGradient id="bg-gradient" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:{color1};stop-opacity:1" />
      <stop offset="100%" style="stop-color:{color2};stop-opacity:1" />
    </linearGradient>
  </defs>
  <circle cx="64" cy="64" r="56" fill="url(#bg-gradient)" />
  <g transform="translate(28, 28) scale(3)" stroke="white" fill="none" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
    {svg_paths}
  </g>
</svg>
'''
    svg_path = pack_dir / f"{status}.svg"
    svg_path.write_text(svg_content, encoding='utf-8')
    return svg_path

def convert_svg_to_ico(svg_path):
    """Convert SVG to ICO"""
    base_name = svg_path.stem
    pack_dir = svg_path.parent
    sizes = [16, 32, 48, 128]
    png_files = []

    for size in sizes:
        png_file = pack_dir / f"{base_name}-{size}.png"
        subprocess.run([
            "magick", str(svg_path),
            "-background", "none",
            "-resize", f"{size}x{size}",
            str(png_file)
        ], check=True, capture_output=True)
        png_files.append(png_file)

    ico_file = pack_dir / f"{base_name}.ico"
    subprocess.run([
        "magick",
        *[str(p) for p in png_files],
        str(ico_file)
    ], check=True, capture_output=True)

    for png_file in png_files:
        png_file.unlink()

    return ico_file

def regenerate_pack(pack_name, colors):
    """Regenerate an icon pack"""
    pack_dir = OUTPUT_DIR / pack_name
    print(f"\nRegenerating: {pack_name}")

    for status, (color1, color2) in colors.items():
        svg_paths = SVG_PATHS[status]
        svg_path = create_svg(pack_dir, status, color1, color2, svg_paths)
        ico_path = convert_svg_to_ico(svg_path)
        print(f"  [OK] {status}.ico")

def main():
    print("Fixing Neon City and Fire & Ice icon packs")
    print("=" * 40)

    regenerate_pack("neon-city", NEON_CITY)
    regenerate_pack("fire-ice", FIRE_ICE)

    print("\n" + "=" * 40)
    print("[OK] Fixed both icon packs!")

if __name__ == "__main__":
    main()
