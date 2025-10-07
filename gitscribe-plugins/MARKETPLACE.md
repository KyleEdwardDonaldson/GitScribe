# GitScribe Marketplace

**Unified marketplace for plugins and icon packs, integrated into the landing page**

## Overview

The GitScribe Marketplace serves dual purposes:
1. **Plugin Hub**: Discover, install, and manage plugins for GitScribe Full
2. **Icon Pack Repository**: Download additional icon packs to customize GitScribe Status

### Strategy: Ship Small, Grow Later

**GitScribe Status Installer Size Optimization:**
- Ship with 1-2 default icon packs (~200 KB total)
- Users download additional packs from marketplace (5-20 KB each)
- Target: Initial installer <2 MB → Reduced to <1.5 MB

**GitScribe Full Plugin Ecosystem:**
- Core app has minimal built-in features
- Users install plugins for specific workflows
- Community creates specialized tools

---

## Architecture

### Technology Stack

**Frontend (Landing Page Integration):**
```
Landing Page (React/Next.js)
├─ /marketplace
│  ├─ /plugins         → Browse plugins
│  ├─ /icon-packs      → Browse icon packs
│  ├─ /themes          → Browse UI themes
│  └─ /submit          → Submit content
└─ API Routes
   ├─ /api/marketplace/plugins
   ├─ /api/marketplace/icon-packs
   └─ /api/marketplace/analytics
```

**Backend (Serverless/Edge Functions):**
- **Hosting**: Vercel/Netlify edge functions
- **Database**: PostgreSQL (Supabase) or Firebase Firestore
- **Storage**: CDN for downloads (Cloudflare R2 / AWS S3)
- **Search**: Algolia or Meilisearch

**Client Integration:**
- **GitScribe Status**: HTTP downloads via WinINET API (C++)
- **GitScribe Full**: Electron net module (Node.js)

---

## Data Models

### Plugin Schema

```typescript
interface Plugin {
  // Metadata
  id: string;                    // UUID
  slug: string;                  // URL-friendly name
  name: string;                  // Display name
  description: string;           // Short description (1-2 sentences)
  longDescription: string;       // Markdown, detailed docs
  version: string;               // Semver (e.g., "1.2.3")

  // Author
  author: {
    name: string;
    email: string;
    url?: string;
    verified: boolean;           // Verified developer
  };

  // Distribution
  downloadUrl: string;           // CDN URL to .gspl file
  packageSize: number;           // Bytes
  checksum: string;              // SHA-256 hash

  // Categorization
  category: PluginCategory;
  tags: string[];
  keywords: string[];

  // Compatibility
  minVersion: string;            // Min GitScribe version
  maxVersion?: string;           // Max version (optional)
  platform: 'windows' | 'all';

  // Permissions
  permissions: Permission[];

  // Metrics
  downloads: number;
  rating: number;                // 0-5 stars
  reviewCount: number;

  // Dates
  publishedAt: Date;
  updatedAt: Date;

  // Media
  icon: string;                  // 256x256 PNG
  screenshots: string[];         // URLs
  videoUrl?: string;             // YouTube/demo video

  // SEO
  featured: boolean;
  verified: boolean;             // Audited by GitScribe team
  deprecated: boolean;
}

enum PluginCategory {
  PRODUCTIVITY = 'productivity',
  INTEGRATIONS = 'integrations',
  SECURITY = 'security',
  THEMES = 'themes',
  WORKFLOWS = 'workflows',
  UTILITIES = 'utilities',
}

enum Permission {
  NETWORK = 'network',
  FILESYSTEM_READ = 'filesystem:read',
  FILESYSTEM_WRITE = 'filesystem:write',
  SHELL_EXECUTE = 'shell:execute',
  CLIPBOARD = 'clipboard',
  GIT_OPERATIONS = 'git:operations',
}
```

### Icon Pack Schema

```typescript
interface IconPack {
  // Metadata
  id: string;
  slug: string;
  name: string;
  description: string;
  version: string;

  // Author
  author: {
    name: string;
    url?: string;
  };

  // Distribution
  downloadUrl: string;           // CDN URL to .zip file
  packageSize: number;           // Bytes (target: <20 KB)
  checksum: string;              // SHA-256

  // Icon Details
  icons: {
    modified: string;            // Relative path in zip
    added: string;
    deleted: string;
    conflicted: string;
    untracked: string;
    ignored: string;
  };

  // Preview
  previewUrl: string;            // Composite image showing all icons

  // Categorization
  style: 'minimal' | 'bold' | 'neon' | 'flat' | 'gradient' | 'outline';
  tags: string[];

  // Metrics
  downloads: number;
  rating: number;

  // Dates
  publishedAt: Date;
  updatedAt: Date;

  // Flags
  featured: boolean;
  default: boolean;              // Shipped with installer
}
```

### Theme Schema (UI Themes for GitScribe Full)

```typescript
interface Theme {
  id: string;
  slug: string;
  name: string;
  description: string;
  version: string;

  author: {
    name: string;
    url?: string;
  };

  // Distribution
  downloadUrl: string;           // JSON file with color definitions
  packageSize: number;

  // Theme Data (for preview)
  colors: {
    primary: string;
    background: string;
    text: string;
    accent: string;
    // ... full palette
  };

  // Preview
  screenshotUrl: string;         // Shows theme applied

  // Metrics
  downloads: number;
  rating: number;

  publishedAt: Date;
  featured: boolean;
}
```

---

## User Flows

### Flow 1: Download Icon Pack (GitScribe Status)

```
User installs GitScribe Status
  ↓
Right-click on folder → "GitScribe Status Settings"
  ↓
Settings dialog: "Icon Packs" tab
  ↓
Click "Browse More Icon Packs" button
  ↓
Opens browser → gitscribe.dev/marketplace/icon-packs
  ↓
User selects pack → Click "Download"
  ↓
Browser downloads .zip (e.g., neon-city-icons.zip)
  ↓
User opens GitScribe Status Settings → "Import Icon Pack"
  ↓
Selects downloaded .zip → Extracts to %APPDATA%/GitScribe/icon-packs/
  ↓
Settings dialog shows new pack → Select & Apply
  ↓
Explorer refreshes → Icons updated
```

**Alternative Flow: In-App Download (Future)**
```
Settings dialog → "Icon Packs" tab
  ↓
Embedded marketplace browser (webview)
  ↓
Click "Install" → Downloads directly via HTTP
  ↓
Auto-extracts and applies
```

### Flow 2: Install Plugin (GitScribe Full)

```
GitScribe Full app → Settings → Plugins
  ↓
Click "Browse Marketplace"
  ↓
Opens in-app marketplace (embedded webview or external browser)
  ↓
User searches/browses → Finds plugin
  ↓
Click "Install"
  ↓
Shows permissions dialog → User confirms
  ↓
Downloads .gspl file (packaged plugin)
  ↓
Validates checksum → Extracts to %APPDATA%/GitScribe/plugins/
  ↓
Plugin appears in "Installed Plugins" list
  ↓
User enables plugin → onActivate() called
```

---

## Marketplace UI Pages

### Home Page (`/marketplace`)

**Hero Section:**
- Search bar
- Featured plugins carousel
- Categories (Plugins, Icon Packs, Themes)

**Sections:**
1. **Trending This Week** (Top 5 by downloads)
2. **Staff Picks** (Curated by GitScribe team)
3. **New Releases** (Latest 10 uploads)
4. **Categories** (Grid of category cards)

### Browse Plugins (`/marketplace/plugins`)

**Layout:**
- Sidebar filters:
  - Category
  - Permissions
  - Verified only
  - Free/Paid
- Main area: Plugin cards (grid/list toggle)

**Plugin Card:**
```
┌─────────────────────────────────┐
│ [Icon] Plugin Name         ⭐ 4.8│
│ Short description...            │
│ By Author • 12k downloads       │
│ [Category Badge] [Install]      │
└─────────────────────────────────┘
```

### Plugin Detail (`/marketplace/plugins/:slug`)

**Page Layout:**
```
┌─────────────────────────────────────────┐
│ [Icon] Plugin Name              ⭐ 4.8 │
│ By Author (verified ✓)                  │
│                                         │
│ [Install Plugin]  [View Source]         │
│                                         │
│ ── Description ──                       │
│ Detailed markdown description...        │
│                                         │
│ ── Screenshots ──                       │
│ [Image Gallery]                         │
│                                         │
│ ── Permissions ──                       │
│ • Network access (for API calls)        │
│ • Filesystem read (for config)          │
│                                         │
│ ── Details ──                           │
│ Version: 1.2.3                          │
│ Updated: 2 days ago                     │
│ Size: 45 KB                             │
│ Min GitScribe: v1.0.0                   │
│                                         │
│ ── Reviews ──                           │
│ [User reviews with ratings]             │
└─────────────────────────────────────────┘
```

### Browse Icon Packs (`/marketplace/icon-packs`)

**Grid Layout:**
```
┌────────┐ ┌────────┐ ┌────────┐
│[Preview]│ │[Preview]│ │[Preview]│
│ Name   │ │ Name   │ │ Name   │
│ 🔽 5k  │ │ 🔽 3k  │ │ 🔽 1k  │
└────────┘ └────────┘ └────────┘
```

**Icon Pack Card:**
- Preview image (composite of all 6 icons)
- Name
- Download count
- File size
- [Download] button

### Icon Pack Detail (`/marketplace/icon-packs/:slug`)

```
┌─────────────────────────────────────────┐
│ Neon City Icons                         │
│ Vibrant neon aesthetic for night owls   │
│                                         │
│ [Download Pack (12 KB)]                 │
│                                         │
│ ── Preview ──                           │
│ Modified:   [Icon Preview]              │
│ Added:      [Icon Preview]              │
│ Deleted:    [Icon Preview]              │
│ Conflicted: [Icon Preview]              │
│ Untracked:  [Icon Preview]              │
│ Ignored:    [Icon Preview]              │
│                                         │
│ ── Installation ──                      │
│ 1. Download the .zip file               │
│ 2. Right-click folder → GitScribe       │
│    Status Settings                      │
│ 3. Import icon pack                     │
│                                         │
│ ── Details ──                           │
│ Style: Neon                             │
│ Author: @designer                       │
│ Downloads: 5,243                        │
│ Rating: ⭐⭐⭐⭐⭐ 4.9 (87 reviews)      │
└─────────────────────────────────────────┘
```

### Submit Content (`/marketplace/submit`)

**Tabs:**
- Submit Plugin
- Submit Icon Pack
- Submit Theme

**Plugin Submission Form:**
```
Plugin Name: [________________]
Slug: [________________] (auto-generated)
Description: [________________]
Category: [Dropdown]
Tags: [Tag input]

Upload .gspl file: [Browse...]
Source code URL: [________________] (GitHub)

Permissions:
☐ Network
☐ Filesystem read
☐ Filesystem write
☐ Shell execute
☐ Clipboard

Icon (256x256 PNG): [Upload]
Screenshots: [Upload multiple]

[Submit for Review]
```

**Review Process:**
1. Automated checks (malware scan, permission validation)
2. Manual review (for high-risk permissions)
3. Approval email sent
4. Published to marketplace

---

## API Endpoints

### Public API

**Get All Plugins**
```http
GET /api/marketplace/plugins
Query Params:
  - category: string
  - tag: string
  - search: string
  - sort: 'popular' | 'recent' | 'rating'
  - limit: number
  - offset: number

Response:
{
  "items": Plugin[],
  "total": number,
  "hasMore": boolean
}
```

**Get Plugin Details**
```http
GET /api/marketplace/plugins/:slug

Response: Plugin
```

**Get All Icon Packs**
```http
GET /api/marketplace/icon-packs
Query Params:
  - style: string
  - search: string
  - sort: 'popular' | 'recent'

Response:
{
  "items": IconPack[],
  "total": number
}
```

**Get Icon Pack Details**
```http
GET /api/marketplace/icon-packs/:slug

Response: IconPack
```

**Track Download**
```http
POST /api/marketplace/downloads
Body:
{
  "itemId": string,
  "itemType": "plugin" | "icon-pack" | "theme",
  "version": string
}

Response: { success: boolean }
```

**Submit Review**
```http
POST /api/marketplace/reviews
Body:
{
  "itemId": string,
  "rating": number,      // 1-5
  "comment": string
}

Response: { success: boolean }
```

### Admin API

**Submit Plugin**
```http
POST /api/marketplace/submit/plugin
Headers: Authorization: Bearer <token>
Body: FormData with plugin package + metadata

Response:
{
  "id": string,
  "status": "pending_review",
  "estimatedReviewTime": "2-3 days"
}
```

**Approve/Reject Submission**
```http
POST /api/marketplace/admin/review/:id
Headers: Authorization: Bearer <admin_token>
Body:
{
  "action": "approve" | "reject",
  "reason"?: string
}
```

---

## Integration Guide

### For Landing Page (React/Next.js)

**1. Add Marketplace Routes**
```typescript
// app/marketplace/layout.tsx
export default function MarketplaceLayout({ children }) {
  return (
    <div className="marketplace">
      <MarketplaceNav />
      {children}
    </div>
  );
}

// app/marketplace/plugins/page.tsx
export default async function PluginsPage() {
  const plugins = await fetchPlugins();
  return <PluginGrid plugins={plugins} />;
}

// app/marketplace/icon-packs/page.tsx
export default async function IconPacksPage() {
  const packs = await fetchIconPacks();
  return <IconPackGrid packs={packs} />;
}
```

**2. Create API Routes**
```typescript
// app/api/marketplace/plugins/route.ts
import { getPlugins } from '@/lib/marketplace';

export async function GET(request: Request) {
  const { searchParams } = new URL(request.url);
  const category = searchParams.get('category');
  const search = searchParams.get('search');

  const plugins = await getPlugins({ category, search });

  return Response.json(plugins);
}
```

**3. Database Setup (Supabase Example)**
```sql
-- plugins table
CREATE TABLE plugins (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  slug TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  description TEXT,
  long_description TEXT,
  version TEXT NOT NULL,
  author_id UUID REFERENCES users(id),
  download_url TEXT NOT NULL,
  package_size INTEGER,
  checksum TEXT NOT NULL,
  category TEXT NOT NULL,
  tags TEXT[],
  permissions TEXT[],
  downloads INTEGER DEFAULT 0,
  rating DECIMAL(2,1),
  review_count INTEGER DEFAULT 0,
  published_at TIMESTAMP,
  updated_at TIMESTAMP DEFAULT NOW(),
  icon_url TEXT,
  screenshots TEXT[],
  featured BOOLEAN DEFAULT false,
  verified BOOLEAN DEFAULT false,
  deprecated BOOLEAN DEFAULT false
);

-- icon_packs table
CREATE TABLE icon_packs (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  slug TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  description TEXT,
  version TEXT NOT NULL,
  author_id UUID REFERENCES users(id),
  download_url TEXT NOT NULL,
  package_size INTEGER,
  checksum TEXT NOT NULL,
  preview_url TEXT,
  style TEXT,
  tags TEXT[],
  downloads INTEGER DEFAULT 0,
  rating DECIMAL(2,1),
  published_at TIMESTAMP,
  featured BOOLEAN DEFAULT false,
  is_default BOOLEAN DEFAULT false
);

-- reviews table
CREATE TABLE reviews (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  item_id UUID NOT NULL,
  item_type TEXT NOT NULL,  -- 'plugin' or 'icon_pack'
  user_id UUID REFERENCES users(id),
  rating INTEGER CHECK (rating >= 1 AND rating <= 5),
  comment TEXT,
  created_at TIMESTAMP DEFAULT NOW()
);

-- downloads tracking
CREATE TABLE downloads (
  id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
  item_id UUID NOT NULL,
  item_type TEXT NOT NULL,
  version TEXT,
  ip_address TEXT,
  user_agent TEXT,
  downloaded_at TIMESTAMP DEFAULT NOW()
);
```

### For GitScribe Status (Icon Pack Downloads)

**C++ Code to Download Icon Packs:**
```cpp
// IconPackDownloader.h
class IconPackDownloader {
public:
    struct DownloadResult {
        bool success;
        std::wstring filePath;
        std::string error;
    };

    // Download icon pack from marketplace
    static DownloadResult Download(const std::string& packSlug);

    // Extract zip to icon-packs directory
    static bool Extract(const std::wstring& zipPath, const std::wstring& destDir);

    // Verify checksum
    static bool VerifyChecksum(const std::wstring& filePath, const std::string& expectedHash);
};

// Usage:
auto result = IconPackDownloader::Download("neon-city");
if (result.success) {
    IconPackDownloader::Extract(result.filePath, L"%APPDATA%/GitScribe/icon-packs/");
}
```

### For GitScribe Full (Plugin Management)

**Electron Plugin Manager:**
```typescript
// main/PluginManager.ts
import { app, net } from 'electron';
import path from 'path';
import fs from 'fs-extra';
import crypto from 'crypto';

export class PluginManager {
  private pluginsDir = path.join(app.getPath('userData'), 'plugins');

  async installPlugin(pluginSlug: string): Promise<void> {
    // Fetch plugin metadata
    const metadata = await this.fetchPluginMetadata(pluginSlug);

    // Download plugin package
    const downloadPath = await this.downloadPlugin(metadata.downloadUrl);

    // Verify checksum
    if (!this.verifyChecksum(downloadPath, metadata.checksum)) {
      throw new Error('Checksum verification failed');
    }

    // Extract to plugins directory
    const pluginDir = path.join(this.pluginsDir, pluginSlug);
    await this.extractPlugin(downloadPath, pluginDir);

    // Load plugin
    await this.loadPlugin(pluginDir);
  }

  private async fetchPluginMetadata(slug: string) {
    const response = await net.fetch(
      `https://gitscribe.dev/api/marketplace/plugins/${slug}`
    );
    return response.json();
  }

  private async downloadPlugin(url: string): Promise<string> {
    const response = await net.fetch(url);
    const buffer = await response.arrayBuffer();

    const tempPath = path.join(app.getPath('temp'), `plugin-${Date.now()}.gspl`);
    await fs.writeFile(tempPath, Buffer.from(buffer));

    return tempPath;
  }

  private verifyChecksum(filePath: string, expectedHash: string): boolean {
    const fileBuffer = fs.readFileSync(filePath);
    const hash = crypto.createHash('sha256').update(fileBuffer).digest('hex');
    return hash === expectedHash;
  }
}
```

---

## Icon Pack Distribution Strategy

### Default Packs (Shipped with Installer)

**Ship 2 packs:**
1. **Classic** (10 KB) - Windows 10/11 aesthetic, high contrast
2. **Minimal** (8 KB) - Subtle, low-profile icons

**Total size: ~20 KB** (vs. 200 KB for all 10 packs)

### Downloadable Packs (Marketplace)

**8 additional packs:**
- Neon City (12 KB)
- Coral Reef (11 KB)
- Arctic Monochrome (9 KB)
- Sunset Gradient (14 KB)
- Forest Organic (13 KB)
- Cosmic Purple (12 KB)
- Retro Pixel (10 KB)
- Aztec Bold (15 KB)

**Benefits:**
- Installer size: 2 MB → 1.8 MB (180 KB saved)
- Users only download what they want
- Easy to add new packs without releasing new installer
- Community can contribute packs

---

## Analytics & Metrics

### Track These Metrics

**For Each Plugin/Icon Pack:**
- Total downloads
- Downloads per version
- Active installations (if client sends telemetry)
- Average rating
- Review count
- Daily/weekly/monthly download trends

**Marketplace Overall:**
- Total unique visitors
- Search queries (optimize SEO)
- Conversion rate (views → downloads)
- Top categories
- Geographic distribution

**Dashboard:**
```typescript
interface MarketplaceStats {
  totalPlugins: number;
  totalIconPacks: number;
  totalDownloads: number;
  activeUsers: number;
  topPlugins: {
    name: string;
    downloads: number;
    rating: number;
  }[];
  recentSubmissions: number;
  pendingReviews: number;
}
```

---

## Security & Moderation

### Automated Checks (Pre-Publishing)

1. **Malware Scan**: VirusTotal API, ClamAV
2. **Code Analysis**: Detect obfuscated code, eval() usage
3. **Permission Validation**: Ensure manifest matches actual usage
4. **Size Limits**: Plugins <10 MB, Icon packs <50 KB
5. **Name Conflicts**: No duplicate slugs

### Manual Review (For High-Risk Items)

**Triggers manual review if:**
- Requests `shell:execute` permission
- Requests `filesystem:write` permission
- Code is minified/obfuscated
- First submission from new developer
- Reported by users

**Review Checklist:**
- [ ] Code is readable (not obfuscated)
- [ ] Permissions are justified
- [ ] No known vulnerabilities
- [ ] Screenshots match functionality
- [ ] Description is accurate

### User Reporting

**Report Reasons:**
- Malware/security issue
- Doesn't work as described
- Inappropriate content
- Copyright violation
- Spam

**Actions:**
- Flag for review
- Temporary suspension
- Permanent ban (with appeal process)

---

## Launch Strategy

### Phase 1: Soft Launch (Week 1-2)

**Content:**
- 5 official plugins (Conventional Commits, AI Message Generator, etc.)
- 10 icon packs (all existing packs)
- 3 themes

**Audience:**
- GitScribe beta testers
- Discord community
- Email to early adopters

**Goals:**
- Test infrastructure
- Gather feedback
- Fix critical bugs

### Phase 2: Public Launch (Week 3)

**Announcements:**
- Blog post on gitscribe.dev
- Social media (Twitter, Reddit, HN)
- Newsletter to all users

**Content:**
- 10 plugins (5 official + 5 community)
- 15 icon packs
- 5 themes

**Promotions:**
- Feature top 3 plugins on homepage
- "Plugin of the Week" spotlight
- Developer spotlight interviews

### Phase 3: Growth (Month 2+)

**Initiatives:**
- Plugin development contest (prizes)
- Documentation/tutorials
- Community Discord channel for developers
- Monthly newsletter with featured plugins

**Metrics Targets:**
- 50 plugins by Month 3
- 100 plugins by Month 6
- 10k total downloads by Month 3

---

## Future Enhancements

### Version 2.0 Features

1. **In-App Marketplace**: Embedded webview in GitScribe Full (no external browser)
2. **Auto-Updates**: Plugins auto-update when new versions available
3. **Paid Plugins**: Stripe integration for premium plugins
4. **Enterprise Marketplace**: Private plugin distribution for companies
5. **Plugin Bundles**: "Security Suite", "Productivity Pack" bundles
6. **Social Features**: Follow developers, like/bookmark plugins
7. **Plugin Analytics**: Developers see usage stats for their plugins
8. **A/B Testing**: Test plugin descriptions/screenshots
9. **Recommended For You**: ML-based personalized recommendations
10. **Compatibility Checker**: Auto-detect if plugin works with user's setup

---

## Appendix: File Formats

### Plugin Package Format (.gspl)

```
plugin-name-1.2.3.gspl (ZIP archive)
├─ manifest.json
├─ index.js          (Entry point)
├─ icon.png          (256x256)
├─ README.md
├─ LICENSE
└─ lib/
   └─ (other JS files)
```

**manifest.json:**
```json
{
  "name": "Conventional Commits",
  "version": "1.2.3",
  "description": "Format commit messages to Conventional Commits standard",
  "author": {
    "name": "John Doe",
    "email": "john@example.com"
  },
  "main": "index.js",
  "permissions": [
    "git:operations"
  ],
  "gitscribe": {
    "minVersion": "1.0.0"
  }
}
```

### Icon Pack Format (.zip)

```
neon-city-icons.zip
├─ manifest.json
├─ modified.ico
├─ added.ico
├─ deleted.ico
├─ conflicted.ico
├─ untracked.ico
└─ ignored.ico
```

**manifest.json:**
```json
{
  "name": "Neon City",
  "version": "1.0.0",
  "description": "Vibrant neon aesthetic for night owls",
  "author": {
    "name": "Jane Designer"
  },
  "style": "neon",
  "icons": {
    "modified": "modified.ico",
    "added": "added.ico",
    "deleted": "deleted.ico",
    "conflicted": "conflicted.ico",
    "untracked": "untracked.ico",
    "ignored": "ignored.ico"
  }
}
```

---

**Last Updated:** October 4, 2025
**Next Review:** November 1, 2025
