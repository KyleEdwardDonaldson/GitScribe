#!/usr/bin/env python3
"""
Extract all icon packs from HTML samples and convert to ICO files
"""

import os
import re
import subprocess
from pathlib import Path

SAMPLES_DIR = Path(r"C:\R\tabler-icons\git-status-samples")
OUTPUT_DIR = Path(r"C:\R\GitScribe\gitscribe-shell\resources\icon-packs")

# Status mapping: HTML class -> GitScribe status name
STATUS_MAP = {
    'added': 'added',
    'clean': 'clean',
    'conflicted': 'conflicted',
    'ignored': 'ignored',
    'modified': 'modified',
    'tracked': 'untracked'  # Map 'tracked' to 'untracked'
}

# Sample packs
PACKS = [
    {"file": "sample-1-aztec-bold.html", "name": "aztec-bold", "display": "Aztec Bold"},
    {"file": "sample-2-coral-reef.html", "name": "coral-reef", "display": "Coral Reef"},
    {"file": "sample-3-neon-city.html", "name": "neon-city", "display": "Neon City"},
    {"file": "sample-4-desert-sun.html", "name": "desert-sun", "display": "Desert Sun"},
    {"file": "sample-5-tropical-paradise.html", "name": "tropical-paradise", "display": "Tropical Paradise"},
    {"file": "sample-6-electric-plasma.html", "name": "electric-plasma", "display": "Electric Plasma"},
    {"file": "sample-7-sunset-vibes.html", "name": "sunset-vibes", "display": "Sunset Vibes"},
    {"file": "sample-8-candy-pop.html", "name": "candy-pop", "display": "Candy Pop"},
    {"file": "sample-10-fire-ice.html", "name": "fire-ice", "display": "Fire & Ice"}
]

def extract_gradient_colors(html, status_class):
    """Extract gradient colors from CSS"""
    # Find the CSS rule for this status
    pattern = rf'\.{status_class}\s+\.icon-wrapper\s*\{{[^}}]*background:\s*linear-gradient\([^)]+\)'
    match = re.search(pattern, html)

    if match:
        css_rule = match.group(0)
        # Extract hex colors
        colors = re.findall(r'#[0-9A-Fa-f]{6}', css_rule)
        if len(colors) >= 2:
            return colors[0], colors[1]

    # Default gray gradient
    return "#888888", "#666666"

def extract_svg_paths(html, status_class):
    """Extract SVG path elements for a status"""
    # Find the icon card for this status
    card_pattern = rf'<div class="icon-card {status_class}">.*?</div>\s*</div>\s*</div>'
    card_match = re.search(card_pattern, html, re.DOTALL)

    if not card_match:
        return None

    card_html = card_match.group(0)

    # Extract SVG content
    svg_pattern = r'<svg[^>]*>(.*?)</svg>'
    svg_match = re.search(svg_pattern, card_html, re.DOTALL)

    if svg_match:
        svg_content = svg_match.group(1)
        # Extract path elements
        paths = re.findall(r'<path[^>]+>', svg_content)
        return '\n    '.join(paths)

    return None

def create_svg_file(pack_dir, status_name, svg_paths, color1, color2):
    """Create an SVG file with gradient background"""
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

    svg_path = pack_dir / f"{status_name}.svg"
    svg_path.write_text(svg_content, encoding='utf-8')
    return svg_path

def convert_svg_to_ico(svg_path):
    """Convert SVG to multi-size ICO file using ImageMagick"""
    base_name = svg_path.stem
    pack_dir = svg_path.parent

    # Generate PNGs at different sizes
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

    # Combine into ICO
    ico_file = pack_dir / f"{base_name}.ico"
    subprocess.run([
        "magick",
        *[str(p) for p in png_files],
        str(ico_file)
    ], check=True, capture_output=True)

    # Clean up PNGs
    for png_file in png_files:
        png_file.unlink()

    return ico_file

def process_pack(pack_info):
    """Process one icon pack"""
    html_path = SAMPLES_DIR / pack_info["file"]
    pack_dir = OUTPUT_DIR / pack_info["name"]

    print(f"\nProcessing: {pack_info['display']}")

    if not html_path.exists():
        print(f"  [FAIL] HTML file not found: {html_path}")
        return False

    # Create pack directory
    pack_dir.mkdir(parents=True, exist_ok=True)

    # Read HTML
    html = html_path.read_text(encoding='utf-8')

    # Process each status
    icons_created = 0
    for html_class, git_status in STATUS_MAP.items():
        # Extract SVG paths
        svg_paths = extract_svg_paths(html, html_class)
        if not svg_paths:
            print(f"  [FAIL] Could not extract SVG for {git_status}")
            continue

        # Extract gradient colors
        color1, color2 = extract_gradient_colors(html, html_class)

        # Create SVG file
        svg_path = create_svg_file(pack_dir, git_status, svg_paths, color1, color2)

        # Convert to ICO
        try:
            ico_path = convert_svg_to_ico(svg_path)
            print(f"  [OK] Created {git_status}.ico")
            icons_created += 1
        except subprocess.CalledProcessError as e:
            print(f"  [FAIL] Failed to convert {git_status}: {e}")

    return icons_created == len(STATUS_MAP)

def main():
    print("GitScribe Icon Pack Generator")
    print("=" * 40)

    # Create output directory
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    successful = 0
    for pack in PACKS:
        if process_pack(pack):
            successful += 1

    print(f"\n{'=' * 40}")
    print(f"[OK] Successfully created {successful}/{len(PACKS)} icon packs")
    print(f"\nIcon packs are in: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
