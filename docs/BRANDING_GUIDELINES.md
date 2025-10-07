# GitScribe Brand Guidelines

## Brand Vision

GitScribe represents the pinnacle of Git tooling - where power meets elegance, where complexity becomes clarity, and where version control becomes art. Our brand embodies luxury software craftsmanship, treating developers as artisans who deserve tools that are not just functional, but beautiful and inspiring.

## Brand Personality

### Core Attributes
- **Luxurious**: Premium feel without pretension
- **Powerful**: Immense capability presented simply
- **Elegant**: Every pixel considered, every interaction refined
- **Trustworthy**: Rock-solid reliability wrapped in beauty
- **Intelligent**: Smart enough to predict, humble enough to learn

### Brand Voice
- **Confident, not arrogant**: "Commit with confidence" not "The best Git client ever"
- **Refined, not complicated**: Use clear language with sophisticated presentation
- **Helpful, not patronizing**: Guide without condescending
- **Premium, not exclusive**: Luxurious experience available to everyone

## Visual Identity

### Logo

```
Primary Mark:
╔═══════════════════════════════╗
║       GitScribe               ║
║    ⟨/⟩  [Stylized GS Mark]    ║
╚═══════════════════════════════╝

The GS mark combines:
- Git branch symbol ⟨/⟩
- Fountain pen nib (scribe)
- Forward momentum
```

### Color Palette

#### Dark Mode - "Obsidian & Gold"
```scss
// Primary Colors
$obsidian-black: #0A0A0B;      // Rich, deep black with slight blue
$royal-gold: #D4A574;          // Warm, luxurious gold
$champagne-gold: #F7E7CE;      // Bright gold for highlights

// UI Colors
$surface-dark: #111113;        // Slightly lighter than obsidian
$surface-elevated: #1A1A1D;    // Elevated surfaces
$surface-overlay: #242427;     // Overlays and modals

// Accent Colors
$accent-gold: #FFD700;         // Pure gold for CTAs
$accent-amber: #FFA500;        // Amber for warnings
$accent-emerald: #50C878;      // Success states

// Text Colors
$text-primary: #FFFFFF;        // Pure white for contrast
$text-secondary: #C4C4C6;      // Muted text
$text-tertiary: #8E8E93;       // Disabled/hint text
$text-gold: #D4A574;           // Gold text for emphasis

// Semantic Colors
$success: #4ADE80;             // Emerald green
$warning: #FFA500;             // Amber
$error: #EF4444;               // Refined red
$info: #60A5FA;                // Sky blue

// Gradients
$gradient-gold: linear-gradient(135deg, #D4A574 0%, #F7E7CE 100%);
$gradient-obsidian: linear-gradient(180deg, #0A0A0B 0%, #1A1A1D 100%);
```

#### Light Mode - "Heavenly"
```scss
// Primary Colors
$heaven-white: #FAFAFA;        // Pure, divine white
$angel-pearl: #F5F5F7;         // Soft pearl white
$seraph-gold: #B8860B;         // Deep, rich gold

// UI Colors
$surface-light: #FFFFFF;       // Pure white surfaces
$surface-elevated: #F9F9FB;    // Slightly elevated
$surface-overlay: #F3F3F5;     // Overlays

// Accent Colors
$accent-gold: #DAA520;         // Goldenrod
$accent-sky: #87CEEB;          // Heavenly blue
$accent-sage: #87A96B;         // Sage green

// Text Colors
$text-primary: #1C1C1E;        // Rich black
$text-secondary: #48484A;      // Secondary text
$text-tertiary: #8E8E93;       // Muted text
$text-divine: #B8860B;         // Gold text

// Semantic Colors
$success: #34C759;             // Heavenly green
$warning: #FF9F0A;             // Divine amber
$error: #FF3B30;               // Seraphim red
$info: #5AC8FA;                // Sky blue

// Gradients
$gradient-heaven: linear-gradient(180deg, #FFFFFF 0%, #F5F5F7 100%);
$gradient-divine: radial-gradient(circle, #FFF8DC 0%, #FAFAFA 70%);
```

### Typography

```scss
// Font Stack
$font-display: 'Neue Haas Grotesk Display', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
$font-body: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
$font-mono: 'JetBrains Mono', 'Cascadia Code', 'Consolas', monospace;

// Display (Headings, CTAs)
.display-hero {
  font-family: $font-display;
  font-weight: 500;
  font-size: 48px;
  letter-spacing: -0.02em;
  line-height: 1.1;
}

.display-title {
  font-family: $font-display;
  font-weight: 500;
  font-size: 32px;
  letter-spacing: -0.01em;
  line-height: 1.2;
}

// Body Text
.body-large {
  font-family: $font-body;
  font-weight: 400;
  font-size: 16px;
  line-height: 1.6;
}

.body-regular {
  font-family: $font-body;
  font-weight: 400;
  font-size: 14px;
  line-height: 1.5;
}

// Monospace (Code, Commits)
.mono-code {
  font-family: $font-mono;
  font-weight: 400;
  font-size: 13px;
  line-height: 1.6;
  font-feature-settings: 'liga' 1; // Enable ligatures
}
```

### Spacing System

```scss
// Base unit: 4px
$space-xs: 4px;
$space-sm: 8px;
$space-md: 16px;
$space-lg: 24px;
$space-xl: 32px;
$space-2xl: 48px;
$space-3xl: 64px;

// Component spacing
$radius-sm: 4px;
$radius-md: 8px;
$radius-lg: 12px;
$radius-xl: 16px;
$radius-full: 9999px;
```

### Iconography

#### Style Principles
- **Line weight**: 1.5px for 16px icons, 2px for 24px icons
- **Corners**: Rounded, matching radius system
- **Consistency**: All icons feel part of same family
- **Clarity**: Instantly recognizable at all sizes

#### Icon Set
```
Core Actions:
📝 Commit (write/create)
↑  Push (upload)
↓  Pull (download)
🔄 Sync (bidirectional)
🌿 Branch (version)
🔀 Merge (combine)

Status Indicators:
✓  Success (checkmark)
⚠  Warning (triangle)
✕  Error (x mark)
●  Modified (dot)
+  Added (plus)
-  Deleted (minus)

Premium Features:
✨ AI/Magic (sparkles)
👑 Pro features (crown)
🚀 Performance (rocket)
```

## UI Components

### Buttons

```scss
// Primary Button (CTA)
.button-primary {
  background: $gradient-gold;
  color: $obsidian-black;
  padding: 12px 24px;
  border-radius: $radius-md;
  font-weight: 500;
  box-shadow: 0 4px 12px rgba(212, 165, 116, 0.3);
  transition: all 0.2s ease;

  &:hover {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(212, 165, 116, 0.4);
  }
}

// Secondary Button
.button-secondary {
  background: transparent;
  color: $royal-gold;
  border: 1px solid $royal-gold;
  padding: 12px 24px;
  border-radius: $radius-md;
  transition: all 0.2s ease;

  &:hover {
    background: rgba(212, 165, 116, 0.1);
  }
}

// Ghost Button
.button-ghost {
  background: transparent;
  color: $text-secondary;
  padding: 12px 24px;
  transition: all 0.2s ease;

  &:hover {
    color: $text-primary;
    background: rgba(255, 255, 255, 0.05);
  }
}
```

### Cards

```scss
// Dark Mode Card
.card-dark {
  background: $surface-elevated;
  border: 1px solid rgba(212, 165, 116, 0.1);
  border-radius: $radius-lg;
  padding: $space-lg;
  box-shadow: 0 4px 24px rgba(0, 0, 0, 0.4);

  &.card-premium {
    border-color: $royal-gold;
    box-shadow: 0 0 48px rgba(212, 165, 116, 0.2);
  }
}

// Light Mode Card
.card-light {
  background: $surface-light;
  border: 1px solid rgba(184, 134, 11, 0.1);
  border-radius: $radius-lg;
  padding: $space-lg;
  box-shadow: 0 4px 24px rgba(0, 0, 0, 0.06);

  &.card-divine {
    background: $gradient-divine;
    box-shadow: 0 8px 32px rgba(184, 134, 11, 0.1);
  }
}
```

### Status Overlays

```scss
// Overlay badge design
.overlay-modified {
  background: $accent-amber;
  width: 8px;
  height: 8px;
  border-radius: $radius-full;
  box-shadow: 0 0 4px rgba(255, 165, 0, 0.6);
}

.overlay-added {
  background: $success;
  // Green plus sign overlay
}

.overlay-deleted {
  background: $error;
  // Red minus sign overlay
}

.overlay-ignored {
  background: $text-tertiary;
  opacity: 0.6;
}
```

## Motion & Animation

### Principles
- **Purposeful**: Every animation has meaning
- **Smooth**: 60fps always, use GPU acceleration
- **Subtle**: Enhance, don't distract
- **Consistent**: Same easing curves throughout

### Timing Functions
```scss
$ease-smooth: cubic-bezier(0.4, 0.0, 0.2, 1);    // Standard easing
$ease-out: cubic-bezier(0.0, 0.0, 0.2, 1);       // Deceleration
$ease-in: cubic-bezier(0.4, 0.0, 1, 1);          // Acceleration
$ease-spring: cubic-bezier(0.68, -0.55, 0.265, 1.55); // Bouncy

// Durations
$duration-instant: 100ms;
$duration-fast: 200ms;
$duration-normal: 300ms;
$duration-slow: 500ms;
```

### Micro-interactions
```scss
// Hover states
.interactive {
  transition: all $duration-fast $ease-smooth;

  &:hover {
    transform: translateY(-2px);
    // Subtle lift effect
  }

  &:active {
    transform: translateY(0);
    // Press feedback
  }
}

// Loading states
@keyframes pulse-gold {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

.loading {
  animation: pulse-gold 2s $ease-smooth infinite;
}
```

## Context Menu Styling

### Dark Mode Context Menu
```scss
.context-menu-dark {
  background: $surface-elevated;
  border: 1px solid rgba(212, 165, 116, 0.2);
  border-radius: $radius-md;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(24px);

  .menu-item {
    color: $text-primary;
    padding: 8px 16px;

    &:hover {
      background: rgba(212, 165, 116, 0.1);
      color: $champagne-gold;
    }

    &.primary {
      color: $royal-gold;
      font-weight: 500;
    }

    &.disabled {
      color: $text-tertiary;
      opacity: 0.5;
    }
  }

  .menu-separator {
    border-top: 1px solid rgba(212, 165, 116, 0.1);
    margin: 4px 0;
  }
}
```

### Light Mode Context Menu
```scss
.context-menu-light {
  background: $surface-light;
  border: 1px solid rgba(184, 134, 11, 0.1);
  border-radius: $radius-md;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.1);

  .menu-item {
    color: $text-primary;

    &:hover {
      background: rgba(184, 134, 11, 0.05);
      color: $seraph-gold;
    }

    &.primary {
      color: $seraph-gold;
      font-weight: 500;
    }
  }
}
```

## App Window Chrome

### Dark Mode - "Obsidian Luxury"
```scss
.window-chrome-dark {
  background: $obsidian-black;
  border: 1px solid rgba(212, 165, 116, 0.2);

  .title-bar {
    background: $gradient-obsidian;
    -webkit-app-region: drag;
    height: 32px;

    .title {
      color: $royal-gold;
      font-family: $font-display;
      font-weight: 500;
      font-size: 13px;
      letter-spacing: 0.5px;
      text-transform: uppercase;
    }
  }

  .sidebar {
    background: $surface-dark;
    border-right: 1px solid rgba(212, 165, 116, 0.1);
  }

  .main-content {
    background: $surface-elevated;
  }
}
```

### Light Mode - "Divine Light"
```scss
.window-chrome-light {
  background: $heaven-white;

  .title-bar {
    background: $gradient-heaven;
    border-bottom: 1px solid rgba(184, 134, 11, 0.1);

    .title {
      color: $seraph-gold;
    }
  }

  .sidebar {
    background: $angel-pearl;
    border-right: 1px solid rgba(0, 0, 0, 0.05);
  }
}
```

## Quick Actions Bar

### Design Specification
```scss
.quick-actions-bar {
  // Dark mode: Floating obsidian panel with gold accents
  background: rgba(10, 10, 11, 0.95);
  border: 1px solid $royal-gold;
  border-radius: $radius-lg;
  box-shadow: 0 24px 64px rgba(0, 0, 0, 0.8),
              0 0 120px rgba(212, 165, 116, 0.2);
  backdrop-filter: blur(24px);

  .search-input {
    background: transparent;
    border: none;
    color: $text-primary;
    font-size: 18px;
    font-family: $font-body;

    &::placeholder {
      color: $text-tertiary;
    }
  }

  .command-item {
    padding: 12px 16px;

    &:hover, &.selected {
      background: rgba(212, 165, 116, 0.1);

      .command-text {
        color: $champagne-gold;
      }
    }

    .command-icon {
      color: $royal-gold;
    }

    .command-shortcut {
      color: $text-tertiary;
      font-size: 12px;
      font-family: $font-mono;
    }
  }
}
```

## AI Features Branding

### Visual Treatment
```scss
.ai-feature {
  // Sparkle gradient border
  position: relative;

  &::before {
    content: '';
    position: absolute;
    inset: -1px;
    border-radius: inherit;
    background: linear-gradient(90deg,
      $royal-gold 0%,
      $champagne-gold 50%,
      $royal-gold 100%);
    animation: shimmer 3s linear infinite;
    z-index: -1;
  }

  .ai-badge {
    background: $gradient-gold;
    color: $obsidian-black;
    padding: 4px 8px;
    border-radius: $radius-full;
    font-size: 11px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.5px;

    &::before {
      content: '✨';
      margin-right: 4px;
    }
  }
}

@keyframes shimmer {
  0% { background-position: 0% 50%; }
  100% { background-position: 200% 50%; }
}
```

### AI Commit Messages
```scss
.ai-suggestion {
  background: linear-gradient(135deg,
    rgba(212, 165, 116, 0.05) 0%,
    rgba(247, 231, 206, 0.05) 100%);
  border: 1px solid rgba(212, 165, 116, 0.3);
  border-radius: $radius-md;
  padding: $space-md;

  .suggestion-header {
    color: $royal-gold;
    font-size: 12px;
    font-weight: 500;
    text-transform: uppercase;
    letter-spacing: 0.5px;
    margin-bottom: $space-sm;

    &::before {
      content: '✨';
      margin-right: $space-xs;
    }
  }

  .suggestion-text {
    color: $text-primary;
    font-family: $font-mono;
    font-size: 14px;
    line-height: 1.6;
  }

  .suggestion-actions {
    margin-top: $space-md;
    display: flex;
    gap: $space-sm;

    .accept-button {
      background: $gradient-gold;
      color: $obsidian-black;
    }
  }
}
```

## Marketing Materials

### Landing Page Hero
```scss
.hero-section {
  background: $obsidian-black;
  position: relative;
  overflow: hidden;

  // Luxury gradient background
  &::before {
    content: '';
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: radial-gradient(circle at center,
      rgba(212, 165, 116, 0.1) 0%,
      transparent 70%);
    animation: rotate 30s linear infinite;
  }

  .hero-title {
    font-family: $font-display;
    font-size: 72px;
    font-weight: 300;
    letter-spacing: -0.03em;
    background: $gradient-gold;
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    text-align: center;
  }

  .hero-subtitle {
    color: $text-secondary;
    font-size: 20px;
    text-align: center;
    margin-top: $space-lg;
  }
}
```

### Product Screenshots
```scss
.screenshot-frame {
  position: relative;
  border-radius: $radius-xl;
  overflow: hidden;
  box-shadow: 0 32px 128px rgba(0, 0, 0, 0.8),
              0 0 200px rgba(212, 165, 116, 0.1);

  // Gold shimmer on hover
  &::after {
    content: '';
    position: absolute;
    top: -50%;
    left: -50%;
    width: 200%;
    height: 200%;
    background: linear-gradient(45deg,
      transparent 30%,
      rgba(212, 165, 116, 0.3) 50%,
      transparent 70%);
    transform: rotate(45deg) translate(-100%, -100%);
    transition: transform 1s $ease-smooth;
  }

  &:hover::after {
    transform: rotate(45deg) translate(100%, 100%);
  }
}
```

## Voice & Tone Examples

### Feature Descriptions

**Premium/Luxurious:**
- ✅ "Crafted for developers who appreciate excellence"
- ✅ "Where code meets artistry"
- ✅ "Intelligent Git operations that feel like magic"
- ❌ "The best Git client you'll ever use"
- ❌ "Revolutionary Git management"

### Error Messages

**Helpful/Elegant:**
- ✅ "We couldn't complete the merge. Let's resolve this together."
- ✅ "This operation needs your attention"
- ❌ "Error: Merge failed"
- ❌ "Operation aborted"

### CTAs

**Inviting/Confident:**
- ✅ "Begin Your Journey"
- ✅ "Experience GitScribe"
- ✅ "Unlock Pro Features"
- ❌ "Buy Now"
- ❌ "Get Started"

## Implementation Checklist

### Phase 1: Foundation
- [ ] Implement color system in CSS/SCSS
- [ ] Set up typography scale
- [ ] Create button components
- [ ] Design icon set
- [ ] Build dark/light theme switcher

### Phase 2: Components
- [ ] Context menu styling
- [ ] App window chrome
- [ ] Quick Actions Bar
- [ ] Card components
- [ ] Form elements

### Phase 3: Premium Features
- [ ] AI feature badges
- [ ] Pro tier indicators
- [ ] Upgrade prompts
- [ ] Success animations

### Phase 4: Marketing
- [ ] Landing page design
- [ ] Product screenshots
- [ ] Social media templates
- [ ] Email templates

## Brand Assets

### Required Files
```
/brand/
├── logos/
│   ├── gitscribe-mark.svg
│   ├── gitscribe-wordmark.svg
│   ├── gitscribe-full.svg
│   └── gitscribe-icon.ico
├── colors/
│   ├── palette-dark.ase
│   ├── palette-light.ase
│   └── gradients.ase
├── typography/
│   ├── type-scale.sketch
│   └── font-files/
└── icons/
    ├── ui-icons.svg
    ├── status-overlays.svg
    └── brand-icons.svg
```

## Usage Guidelines

### Do's
- ✅ Use gold accents sparingly for emphasis
- ✅ Maintain high contrast ratios (WCAG AAA)
- ✅ Use smooth animations for state changes
- ✅ Keep interfaces clean and uncluttered
- ✅ Make premium features feel special

### Don'ts
- ❌ Use more than 2 fonts in one view
- ❌ Mix gold tones (stick to palette)
- ❌ Use animations longer than 500ms
- ❌ Create busy, cluttered layouts
- ❌ Make free tier feel limited

## Accessibility

### Color Contrast
- Text on dark: Minimum 7:1 ratio
- Text on light: Minimum 4.5:1 ratio
- Interactive elements: 3:1 ratio minimum
- Focus indicators: Visible gold outline

### Motion
- Respect `prefers-reduced-motion`
- Provide motion-free alternatives
- No auto-playing animations
- Smooth, not jarring transitions

---

**End of GitScribe Brand Guidelines**

*"Where code meets artistry"*