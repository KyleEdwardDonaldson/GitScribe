#!/usr/bin/env python3
"""
Add 50% transparency to all sizes in ICO files.
"""

from PIL import Image
import os

def make_transparent(ico_path, opacity=128):
    """Apply transparency to all sizes in an ICO file."""
    try:
        ico = Image.open(ico_path)
        processed_images = []

        for i in range(1000):
            try:
                ico.seek(i)
                img = ico.copy().convert('RGBA')

                # Create a new image with adjusted alpha
                pixels = img.load()
                for y in range(img.height):
                    for x in range(img.width):
                        r, g, b, a = pixels[x, y]
                        # Reduce alpha by 50%
                        new_alpha = int(a * (opacity / 255.0))
                        pixels[x, y] = (r, g, b, new_alpha)

                processed_images.append(img)
            except EOFError:
                break

        if processed_images:
            sizes = [(img.width, img.height) for img in processed_images]
            processed_images[0].save(ico_path, format='ICO', sizes=sizes, append_images=processed_images[1:])
            return True
    except Exception as e:
        print(f"[ERROR] {ico_path}: {e}")
    return False

def main():
    default_dir = os.path.join(os.path.dirname(__file__), 'default')

    icons = ['modified', 'added', 'clean', 'untracked', 'conflicted', 'ignored']

    print("Adding 50% transparency to overlay icons...")
    for name in icons:
        ico_file = os.path.join(default_dir, f"{name}.ico")
        if os.path.exists(ico_file):
            if make_transparent(ico_file):
                print(f"  [OK] {name}.ico")
        else:
            print(f"  [MISSING] {name}.ico")

    print("\n[SUCCESS] Transparency applied!")
    print("\nNext steps:")
    print("  1. Unregister: unregister.cmd (as Admin)")
    print("  2. Rebuild: cd build && cmake --build . --config Release")
    print("  3. Re-register: register.cmd (as Admin)")

if __name__ == '__main__':
    main()
