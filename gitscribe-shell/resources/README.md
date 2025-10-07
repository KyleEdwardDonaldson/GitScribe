# Icon Resources

This directory contains the overlay icon files for GitScribe.

## Icon Specifications

- **Format**: .ICO (Windows Icon)
- **Sizes**: 16x16, 32x32, 48x48, 128x128 (all embedded in one .ico file)
- **Color Depth**: 32-bit with alpha channel (transparency)
- **Placement**: Bottom-left corner overlay

## Icon Files

- `modified.ico` - Yellow dot (uncommitted changes) #F4C430
- `clean.ico` - Turquoise checkmark (committed, clean) #1CE4D4
- `added.ico` - Green plus (staged for commit) #00A86B
- `untracked.ico` - Purple question mark (not in Git) #9966CC
- `conflicted.ico` - Red exclamation (merge conflicts) #E34234
- `ignored.ico` - Gray X (in .gitignore) #78716C

## Creating Icons

1. Generate in Midjourney using the prompts in project docs
2. Remove white background using remove.bg
3. Resize to multiple sizes (16x16, 32x32, 48x48, 128x128)
4. Combine into single .ico file using:
   - ImageMagick: `magick convert icon-*.png icon.ico`
   - GIMP: Export as .ico with all sizes
   - Online tools: convertio.co, icoconvert.com

## Placeholder Icons

Currently using simple placeholder icons. Replace with final Midjourney-generated icons before release.
