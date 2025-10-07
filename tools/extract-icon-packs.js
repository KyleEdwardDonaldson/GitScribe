/**
 * Extract SVG icons from HTML samples and convert to ICO files
 *
 * Requires: Node.js, sharp package
 * Install: npm install sharp
 */

const fs = require('fs');
const path = require('path');
const { JSDOM } = require('jsdom');

const SAMPLES_DIR = 'C:/R/tabler-icons/git-status-samples';
const OUTPUT_DIR = 'C:/R/GitScribe/gitscribe-shell/resources/icon-packs';

// Icon status mapping
const STATUS_MAP = {
    'added': { name: 'added', gradient: true },
    'clean': { name: 'clean', gradient: true },
    'conflicted': { name: 'conflicted', gradient: true },
    'ignored': { name: 'ignored', gradient: true },
    'modified': { name: 'modified', gradient: true },
    'tracked': { name: 'untracked', gradient: true } // Map 'tracked' to 'untracked'
};

// Sample pack names
const PACKS = [
    { file: 'sample-1-aztec-bold.html', name: 'aztec-bold', displayName: 'Aztec Bold' },
    { file: 'sample-2-coral-reef.html', name: 'coral-reef', displayName: 'Coral Reef' },
    { file: 'sample-3-neon-city.html', name: 'neon-city', displayName: 'Neon City' },
    { file: 'sample-4-desert-sun.html', name: 'desert-sun', displayName: 'Desert Sun' },
    { file: 'sample-5-tropical-paradise.html', name: 'tropical-paradise', displayName: 'Tropical Paradise' },
    { file: 'sample-6-electric-plasma.html', name: 'electric-plasma', displayName: 'Electric Plasma' },
    { file: 'sample-7-sunset-vibes.html', name: 'sunset-vibes', displayName: 'Sunset Vibes' },
    { file: 'sample-8-candy-pop.html', name: 'candy-pop', displayName: 'Candy Pop' },
    { file: 'sample-10-fire-ice.html', name: 'fire-ice', displayName: 'Fire & Ice' }
];

async function extractIconPack(packInfo) {
    const htmlPath = path.join(SAMPLES_DIR, packInfo.file);
    const outputPath = path.join(OUTPUT_DIR, packInfo.name);

    console.log(`\nProcessing: ${packInfo.displayName}`);

    // Create output directory
    if (!fs.existsSync(outputPath)) {
        fs.mkdirSync(outputPath, { recursive: true });
    }

    // Read HTML file
    const html = fs.readFileSync(htmlPath, 'utf-8');
    const dom = new JSDOM(html);
    const document = dom.window.document;

    // Extract each icon card
    const iconCards = document.querySelectorAll('.icon-card');
    const packMetadata = {
        name: packInfo.name,
        displayName: packInfo.displayName,
        icons: {}
    };

    for (const card of iconCards) {
        const className = Array.from(card.classList).find(c => c !== 'icon-card');
        if (!STATUS_MAP[className]) continue;

        const statusInfo = STATUS_MAP[className];
        const svg = card.querySelector('svg');
        const iconWrapper = card.querySelector('.icon-wrapper');

        // Get background gradient from CSS
        const gradientStyle = dom.window.getComputedStyle(iconWrapper).background;

        // Extract SVG
        const svgContent = svg.outerHTML;

        // Save SVG with gradient as background
        const svgWithBg = createSVGWithBackground(svgContent, gradientStyle, iconWrapper);

        const svgPath = path.join(outputPath, `${statusInfo.name}.svg`);
        fs.writeFileSync(svgPath, svgWithBg);

        packMetadata.icons[statusInfo.name] = {
            svg: `${statusInfo.name}.svg`,
            gradient: gradientStyle
        };

        console.log(`  ✓ Extracted ${statusInfo.name}`);
    }

    // Save metadata
    const metadataPath = path.join(outputPath, 'pack.json');
    fs.writeFileSync(metadataPath, JSON.stringify(packMetadata, null, 2));

    console.log(`  ✓ Saved metadata to pack.json`);

    return packMetadata;
}

function createSVGWithBackground(svgContent, gradient, iconWrapper) {
    // Parse gradient colors from CSS
    const gradientMatch = gradient.match(/linear-gradient\([^)]+\)/);
    if (!gradientMatch) {
        // Fallback to simple background
        return svgContent;
    }

    // Create SVG with circular background
    return `<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 128 128">
  <defs>
    <linearGradient id="bg-gradient" x1="0%" y1="0%" x2="100%" y2="100%">
      <!-- Will be filled by CSS gradient colors -->
    </linearGradient>
  </defs>
  <!-- Circular background -->
  <circle cx="64" cy="64" r="56" fill="url(#bg-gradient)" />
  <!-- Icon -->
  <g transform="translate(40, 40) scale(2)">
    ${svgContent.match(/<path[^>]+>/g).join('\n    ')}
  </g>
</svg>`;
}

async function main() {
    console.log('GitScribe Icon Pack Extractor');
    console.log('==============================\n');

    // Create base output directory
    if (!fs.existsSync(OUTPUT_DIR)) {
        fs.mkdirSync(OUTPUT_DIR, { recursive: true });
    }

    const allPacks = [];

    // Process each pack
    for (const pack of PACKS) {
        try {
            const metadata = await extractIconPack(pack);
            allPacks.push(metadata);
        } catch (error) {
            console.error(`  ✗ Error processing ${pack.displayName}:`, error.message);
        }
    }

    // Save global icon packs metadata
    const globalMetadata = {
        version: '1.0.0',
        packs: allPacks
    };

    const globalMetadataPath = path.join(OUTPUT_DIR, 'icon-packs.json');
    fs.writeFileSync(globalMetadataPath, JSON.stringify(globalMetadata, null, 2));

    console.log(`\n✓ Extracted ${allPacks.length} icon packs`);
    console.log(`\nNext step: Convert SVGs to ICO files`);
    console.log(`  Run: node convert-svg-to-ico.js`);
}

main().catch(console.error);
