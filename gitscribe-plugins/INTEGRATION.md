# Landing Page Integration Guide

**How to integrate the GitScribe Marketplace into your landing page**

## Overview

This guide explains how to add the marketplace to your existing landing page (Next.js, React, or any modern framework). The marketplace will serve:

1. **Plugins** - For GitScribe Full users
2. **Icon Packs** - For both GitScribe Status and Full users
3. **Themes** - For GitScribe Full users

## Prerequisites

- **Frontend Framework**: Next.js 14+ (App Router) or React 18+
- **Backend**: Serverless functions (Vercel/Netlify) or Node.js API
- **Database**: PostgreSQL (Supabase recommended) or Firebase Firestore
- **Storage**: CDN for downloads (Cloudflare R2, AWS S3, or similar)
- **Search** (optional): Algolia or Meilisearch for fast search

## Directory Structure

```
your-landing-page/
├─ app/
│  ├─ marketplace/
│  │  ├─ layout.tsx              # Marketplace layout
│  │  ├─ page.tsx                # Marketplace home
│  │  ├─ plugins/
│  │  │  ├─ page.tsx             # Browse plugins
│  │  │  └─ [slug]/
│  │  │     └─ page.tsx          # Plugin detail
│  │  ├─ icon-packs/
│  │  │  ├─ page.tsx             # Browse icon packs
│  │  │  └─ [slug]/
│  │  │     └─ page.tsx          # Icon pack detail
│  │  ├─ themes/
│  │  │  ├─ page.tsx             # Browse themes
│  │  │  └─ [slug]/
│  │  │     └─ page.tsx          # Theme detail
│  │  └─ submit/
│  │     └─ page.tsx             # Submit content
│  ├─ api/
│  │  └─ marketplace/
│  │     ├─ plugins/
│  │     │  ├─ route.ts          # GET /api/marketplace/plugins
│  │     │  └─ [slug]/
│  │     │     └─ route.ts       # GET /api/marketplace/plugins/:slug
│  │     ├─ icon-packs/
│  │     │  ├─ route.ts          # GET /api/marketplace/icon-packs
│  │     │  └─ [slug]/
│  │     │     └─ route.ts       # GET /api/marketplace/icon-packs/:slug
│  │     ├─ downloads/
│  │     │  └─ route.ts          # POST /api/marketplace/downloads
│  │     └─ reviews/
│  │        └─ route.ts          # POST /api/marketplace/reviews
│  └─ ...
├─ lib/
│  └─ marketplace/
│     ├─ db.ts                   # Database client
│     ├─ queries.ts              # SQL queries
│     └─ types.ts                # TypeScript types
└─ components/
   └─ marketplace/
      ├─ PluginCard.tsx
      ├─ IconPackCard.tsx
      ├─ SearchBar.tsx
      └─ FilterSidebar.tsx
```

## Step 1: Database Setup

### Option A: Supabase (Recommended)

**1. Create Supabase project**

```bash
npx supabase init
npx supabase start
```

**2. Create database schema**

```sql
-- plugins table
CREATE TABLE plugins (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  slug TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  description TEXT NOT NULL,
  long_description TEXT,
  version TEXT NOT NULL,
  author_id UUID REFERENCES auth.users(id),
  author_name TEXT NOT NULL,
  author_email TEXT,
  author_url TEXT,
  author_verified BOOLEAN DEFAULT false,
  download_url TEXT NOT NULL,
  package_size INTEGER NOT NULL,
  checksum TEXT NOT NULL,
  category TEXT NOT NULL,
  tags TEXT[] DEFAULT '{}',
  keywords TEXT[] DEFAULT '{}',
  permissions TEXT[] DEFAULT '{}',
  min_version TEXT NOT NULL,
  max_version TEXT,
  platform TEXT DEFAULT 'windows',
  downloads INTEGER DEFAULT 0,
  rating DECIMAL(2,1) DEFAULT 0,
  review_count INTEGER DEFAULT 0,
  published_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW(),
  icon_url TEXT,
  screenshots TEXT[] DEFAULT '{}',
  video_url TEXT,
  featured BOOLEAN DEFAULT false,
  verified BOOLEAN DEFAULT false,
  deprecated BOOLEAN DEFAULT false,
  repository_type TEXT,
  repository_url TEXT,
  homepage TEXT,
  license TEXT
);

-- icon_packs table
CREATE TABLE icon_packs (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  slug TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  description TEXT NOT NULL,
  version TEXT NOT NULL,
  author_id UUID REFERENCES auth.users(id),
  author_name TEXT NOT NULL,
  author_url TEXT,
  download_url TEXT NOT NULL,
  package_size INTEGER NOT NULL,
  checksum TEXT NOT NULL,
  preview_url TEXT NOT NULL,
  style TEXT NOT NULL,
  tags TEXT[] DEFAULT '{}',
  downloads INTEGER DEFAULT 0,
  rating DECIMAL(2,1) DEFAULT 0,
  review_count INTEGER DEFAULT 0,
  published_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW(),
  featured BOOLEAN DEFAULT false,
  is_default BOOLEAN DEFAULT false,
  colors JSONB
);

-- themes table
CREATE TABLE themes (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  slug TEXT UNIQUE NOT NULL,
  name TEXT NOT NULL,
  description TEXT NOT NULL,
  version TEXT NOT NULL,
  author_id UUID REFERENCES auth.users(id),
  author_name TEXT NOT NULL,
  author_url TEXT,
  download_url TEXT NOT NULL,
  package_size INTEGER NOT NULL,
  screenshot_url TEXT NOT NULL,
  colors JSONB NOT NULL,
  downloads INTEGER DEFAULT 0,
  rating DECIMAL(2,1) DEFAULT 0,
  review_count INTEGER DEFAULT 0,
  published_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW(),
  featured BOOLEAN DEFAULT false
);

-- reviews table
CREATE TABLE reviews (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  item_id UUID NOT NULL,
  item_type TEXT NOT NULL CHECK (item_type IN ('plugin', 'icon-pack', 'theme')),
  user_id UUID REFERENCES auth.users(id),
  user_name TEXT NOT NULL,
  rating INTEGER NOT NULL CHECK (rating >= 1 AND rating <= 5),
  comment TEXT,
  created_at TIMESTAMPTZ DEFAULT NOW(),
  updated_at TIMESTAMPTZ DEFAULT NOW(),
  helpful_count INTEGER DEFAULT 0,
  UNIQUE(item_id, user_id)
);

-- downloads table (for analytics)
CREATE TABLE downloads (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  item_id UUID NOT NULL,
  item_type TEXT NOT NULL,
  version TEXT NOT NULL,
  ip_address TEXT,
  user_agent TEXT,
  country_code TEXT,
  downloaded_at TIMESTAMPTZ DEFAULT NOW()
);

-- Indexes
CREATE INDEX idx_plugins_category ON plugins(category);
CREATE INDEX idx_plugins_featured ON plugins(featured) WHERE featured = true;
CREATE INDEX idx_plugins_downloads ON plugins(downloads DESC);
CREATE INDEX idx_icon_packs_style ON icon_packs(style);
CREATE INDEX idx_icon_packs_featured ON icon_packs(featured) WHERE featured = true;
CREATE INDEX idx_reviews_item ON reviews(item_id, item_type);
CREATE INDEX idx_downloads_item ON downloads(item_id, downloaded_at);

-- Enable Row Level Security (RLS)
ALTER TABLE plugins ENABLE ROW LEVEL SECURITY;
ALTER TABLE icon_packs ENABLE ROW LEVEL SECURITY;
ALTER TABLE themes ENABLE ROW LEVEL SECURITY;
ALTER TABLE reviews ENABLE ROW LEVEL SECURITY;

-- Public read access
CREATE POLICY "Public read plugins" ON plugins FOR SELECT USING (true);
CREATE POLICY "Public read icon_packs" ON icon_packs FOR SELECT USING (true);
CREATE POLICY "Public read themes" ON themes FOR SELECT USING (true);
CREATE POLICY "Public read reviews" ON reviews FOR SELECT USING (true);

-- Authenticated users can create reviews
CREATE POLICY "Authenticated create reviews" ON reviews FOR INSERT
  WITH CHECK (auth.uid() = user_id);

-- Users can update their own reviews
CREATE POLICY "Users update own reviews" ON reviews FOR UPDATE
  USING (auth.uid() = user_id);
```

**3. Create Supabase client**

```typescript
// lib/marketplace/db.ts
import { createClient } from '@supabase/supabase-js';

const supabaseUrl = process.env.NEXT_PUBLIC_SUPABASE_URL!;
const supabaseKey = process.env.NEXT_PUBLIC_SUPABASE_ANON_KEY!;

export const supabase = createClient(supabaseUrl, supabaseKey);
```

## Step 2: API Routes

### Get All Plugins

```typescript
// app/api/marketplace/plugins/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/marketplace/db';

export async function GET(request: NextRequest) {
  const { searchParams } = new URL(request.url);

  const category = searchParams.get('category');
  const tag = searchParams.get('tag');
  const search = searchParams.get('search');
  const sort = searchParams.get('sort') || 'popular';
  const verified = searchParams.get('verified') === 'true';
  const limit = parseInt(searchParams.get('limit') || '20', 10);
  const offset = parseInt(searchParams.get('offset') || '0', 10);

  let query = supabase
    .from('plugins')
    .select('*', { count: 'exact' });

  // Filters
  if (category) query = query.eq('category', category);
  if (tag) query = query.contains('tags', [tag]);
  if (search) {
    query = query.or(`name.ilike.%${search}%,description.ilike.%${search}%`);
  }
  if (verified) query = query.eq('verified', true);

  // Sorting
  switch (sort) {
    case 'popular':
      query = query.order('downloads', { ascending: false });
      break;
    case 'recent':
      query = query.order('published_at', { ascending: false });
      break;
    case 'rating':
      query = query.order('rating', { ascending: false });
      break;
    case 'name':
      query = query.order('name', { ascending: true });
      break;
  }

  // Pagination
  query = query.range(offset, offset + limit - 1);

  const { data, error, count } = await query;

  if (error) {
    return NextResponse.json({ error: error.message }, { status: 500 });
  }

  return NextResponse.json({
    items: data,
    total: count || 0,
    hasMore: (count || 0) > offset + limit,
    limit,
    offset,
  });
}
```

### Get Single Plugin

```typescript
// app/api/marketplace/plugins/[slug]/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/marketplace/db';

export async function GET(
  request: NextRequest,
  { params }: { params: { slug: string } }
) {
  const { data, error } = await supabase
    .from('plugins')
    .select('*')
    .eq('slug', params.slug)
    .single();

  if (error) {
    return NextResponse.json({ error: 'Plugin not found' }, { status: 404 });
  }

  return NextResponse.json(data);
}
```

### Track Download

```typescript
// app/api/marketplace/downloads/route.ts
import { NextRequest, NextResponse } from 'next/server';
import { supabase } from '@/lib/marketplace/db';

export async function POST(request: NextRequest) {
  const { itemId, itemType, version } = await request.json();

  // Get IP and user agent
  const ip = request.headers.get('x-forwarded-for') || request.ip;
  const userAgent = request.headers.get('user-agent');

  // Insert download record
  const { error } = await supabase.from('downloads').insert({
    item_id: itemId,
    item_type: itemType,
    version,
    ip_address: ip,
    user_agent: userAgent,
  });

  if (error) {
    return NextResponse.json({ error: error.message }, { status: 500 });
  }

  // Increment download count
  const table = itemType === 'plugin' ? 'plugins' :
                itemType === 'icon-pack' ? 'icon_packs' : 'themes';

  await supabase.rpc('increment_downloads', {
    table_name: table,
    row_id: itemId,
  });

  return NextResponse.json({ success: true }, { status: 201 });
}
```

**Create SQL function for incrementing:**

```sql
CREATE OR REPLACE FUNCTION increment_downloads(table_name TEXT, row_id UUID)
RETURNS VOID AS $$
BEGIN
  EXECUTE format('UPDATE %I SET downloads = downloads + 1 WHERE id = $1', table_name)
  USING row_id;
END;
$$ LANGUAGE plpgsql;
```

## Step 3: Frontend Pages

### Marketplace Home

```typescript
// app/marketplace/page.tsx
import { supabase } from '@/lib/marketplace/db';
import { PluginCard } from '@/components/marketplace/PluginCard';
import { IconPackCard } from '@/components/marketplace/IconPackCard';

export default async function MarketplacePage() {
  // Fetch featured items
  const { data: featuredPlugins } = await supabase
    .from('plugins')
    .select('*')
    .eq('featured', true)
    .limit(5);

  const { data: featuredPacks } = await supabase
    .from('icon_packs')
    .select('*')
    .eq('featured', true)
    .limit(5);

  return (
    <div className="marketplace">
      <header className="hero">
        <h1>GitScribe Marketplace</h1>
        <p>Plugins, icon packs, and themes to customize your Git workflow</p>
        <SearchBar />
      </header>

      <section className="featured-plugins">
        <h2>Featured Plugins</h2>
        <div className="grid">
          {featuredPlugins?.map((plugin) => (
            <PluginCard key={plugin.id} plugin={plugin} />
          ))}
        </div>
      </section>

      <section className="featured-icon-packs">
        <h2>Popular Icon Packs</h2>
        <div className="grid">
          {featuredPacks?.map((pack) => (
            <IconPackCard key={pack.id} pack={pack} />
          ))}
        </div>
      </section>
    </div>
  );
}
```

### Browse Plugins

```typescript
// app/marketplace/plugins/page.tsx
'use client';

import { useState, useEffect } from 'react';
import { PluginCard } from '@/components/marketplace/PluginCard';

export default function PluginsPage() {
  const [plugins, setPlugins] = useState([]);
  const [filters, setFilters] = useState({
    category: '',
    search: '',
    sort: 'popular',
  });

  useEffect(() => {
    async function fetchPlugins() {
      const params = new URLSearchParams(filters);
      const response = await fetch(`/api/marketplace/plugins?${params}`);
      const data = await response.json();
      setPlugins(data.items);
    }
    fetchPlugins();
  }, [filters]);

  return (
    <div className="plugins-page">
      <FilterSidebar filters={filters} onChange={setFilters} />
      <div className="plugins-grid">
        {plugins.map((plugin) => (
          <PluginCard key={plugin.id} plugin={plugin} />
        ))}
      </div>
    </div>
  );
}
```

### Plugin Detail

```typescript
// app/marketplace/plugins/[slug]/page.tsx
import { supabase } from '@/lib/marketplace/db';
import { DownloadButton } from '@/components/marketplace/DownloadButton';
import { ReviewList } from '@/components/marketplace/ReviewList';
import Markdown from 'react-markdown';

export default async function PluginDetailPage({
  params,
}: {
  params: { slug: string };
}) {
  const { data: plugin } = await supabase
    .from('plugins')
    .select('*')
    .eq('slug', params.slug)
    .single();

  if (!plugin) {
    return <div>Plugin not found</div>;
  }

  return (
    <div className="plugin-detail">
      <header>
        <img src={plugin.icon_url} alt={plugin.name} />
        <div>
          <h1>{plugin.name}</h1>
          <p>{plugin.description}</p>
          <div className="meta">
            <span>⭐ {plugin.rating.toFixed(1)}</span>
            <span>📥 {plugin.downloads.toLocaleString()} downloads</span>
            <span>v{plugin.version}</span>
          </div>
        </div>
        <DownloadButton plugin={plugin} />
      </header>

      <section className="description">
        <h2>About</h2>
        <Markdown>{plugin.long_description}</Markdown>
      </section>

      <section className="permissions">
        <h2>Permissions</h2>
        <ul>
          {plugin.permissions.map((perm) => (
            <li key={perm}>{perm}</li>
          ))}
        </ul>
      </section>

      <section className="reviews">
        <h2>Reviews</h2>
        <ReviewList itemId={plugin.id} itemType="plugin" />
      </section>
    </div>
  );
}
```

## Step 4: CDN Setup

### Cloudflare R2 Example

**1. Create R2 bucket**

```bash
npx wrangler r2 bucket create gitscribe-marketplace
```

**2. Upload files**

```typescript
// scripts/upload-to-cdn.ts
import { S3Client, PutObjectCommand } from '@aws-sdk/client-s3';
import fs from 'fs';

const s3 = new S3Client({
  region: 'auto',
  endpoint: `https://${process.env.CLOUDFLARE_ACCOUNT_ID}.r2.cloudflarestorage.com`,
  credentials: {
    accessKeyId: process.env.R2_ACCESS_KEY_ID!,
    secretAccessKey: process.env.R2_SECRET_ACCESS_KEY!,
  },
});

async function uploadFile(filePath: string, key: string) {
  const fileContent = fs.readFileSync(filePath);

  await s3.send(
    new PutObjectCommand({
      Bucket: 'gitscribe-marketplace',
      Key: key,
      Body: fileContent,
      ContentType: 'application/zip',
    })
  );

  return `https://cdn.gitscribe.dev/${key}`;
}

// Usage:
const downloadUrl = await uploadFile(
  './neon-city-icons.zip',
  'icon-packs/neon-city/1.0.0/neon-city-icons.zip'
);
```

## Step 5: Search Integration (Optional)

### Algolia

```typescript
// lib/marketplace/search.ts
import algoliasearch from 'algoliasearch';

const client = algoliasearch(
  process.env.ALGOLIA_APP_ID!,
  process.env.ALGOLIA_API_KEY!
);

const pluginsIndex = client.initIndex('plugins');

export async function searchPlugins(query: string) {
  const { hits } = await pluginsIndex.search(query);
  return hits;
}
```

## Step 6: Deploy

### Vercel

```bash
# Install Vercel CLI
npm i -g vercel

# Deploy
vercel

# Set environment variables
vercel env add NEXT_PUBLIC_SUPABASE_URL
vercel env add NEXT_PUBLIC_SUPABASE_ANON_KEY
vercel env add SUPABASE_SERVICE_KEY
```

## Next Steps

1. **Seed Database**: Add initial plugins and icon packs
2. **Admin Panel**: Create UI for reviewing submissions
3. **Analytics**: Track downloads, popular items
4. **SEO**: Add metadata, sitemap, structured data
5. **Testing**: End-to-end tests for browsing and downloading

## Resources

- [MARKETPLACE.md](./MARKETPLACE.md) - Full marketplace specification
- [marketplace.openapi.yaml](./api/marketplace.openapi.yaml) - API specification
- [MarketplaceClient.ts](./client/MarketplaceClient.ts) - TypeScript client library
- [IconPackDownloader.h](./client/IconPackDownloader.h) - C++ client library

---

**Last Updated:** October 4, 2025
