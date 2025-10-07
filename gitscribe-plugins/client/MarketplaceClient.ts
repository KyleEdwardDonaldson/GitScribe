/**
 * GitScribe Marketplace Client (TypeScript/Electron)
 *
 * Client library for browsing, downloading, and installing plugins and icon packs
 * from the GitScribe Marketplace.
 */

import { app, net } from 'electron';
import * as fs from 'fs-extra';
import * as path from 'path';
import * as crypto from 'crypto';
import * as AdmZip from 'adm-zip';

export interface Plugin {
  id: string;
  slug: string;
  name: string;
  description: string;
  longDescription?: string;
  version: string;
  author: {
    name: string;
    email?: string;
    url?: string;
    verified?: boolean;
  };
  downloadUrl: string;
  packageSize: number;
  checksum: string;
  category: string;
  tags: string[];
  keywords?: string[];
  permissions: string[];
  minVersion: string;
  maxVersion?: string;
  platform: 'windows' | 'all';
  downloads: number;
  rating: number;
  reviewCount: number;
  publishedAt: string;
  updatedAt: string;
  icon?: string;
  screenshots?: string[];
  videoUrl?: string;
  featured: boolean;
  verified: boolean;
  deprecated: boolean;
}

export interface IconPack {
  id: string;
  slug: string;
  name: string;
  description: string;
  version: string;
  author: {
    name: string;
    url?: string;
  };
  downloadUrl: string;
  packageSize: number;
  checksum: string;
  previewUrl: string;
  style: string;
  tags: string[];
  downloads: number;
  rating: number;
  publishedAt: string;
  featured: boolean;
  isDefault: boolean;
}

export interface Theme {
  id: string;
  slug: string;
  name: string;
  description: string;
  version: string;
  author: {
    name: string;
    url?: string;
  };
  downloadUrl: string;
  packageSize: number;
  screenshotUrl: string;
  colors: Record<string, string>;
  downloads: number;
  rating: number;
  publishedAt: string;
  featured: boolean;
}

export interface Review {
  id: string;
  itemId: string;
  itemType: 'plugin' | 'icon-pack' | 'theme';
  userId: string;
  userName: string;
  rating: number;
  comment?: string;
  createdAt: string;
  helpfulCount: number;
}

export interface DownloadProgress {
  downloaded: number;
  total: number;
  percent: number;
}

export type ProgressCallback = (progress: DownloadProgress) => void;

export class MarketplaceClient {
  private static readonly API_BASE = 'https://gitscribe.dev/api/marketplace';

  /**
   * Fetch all plugins with optional filters
   */
  static async getPlugins(filters?: {
    category?: string;
    tag?: string;
    search?: string;
    sort?: 'popular' | 'recent' | 'rating' | 'name';
    verified?: boolean;
    limit?: number;
    offset?: number;
  }): Promise<{ items: Plugin[]; total: number; hasMore: boolean }> {
    const params = new URLSearchParams();
    if (filters?.category) params.append('category', filters.category);
    if (filters?.tag) params.append('tag', filters.tag);
    if (filters?.search) params.append('search', filters.search);
    if (filters?.sort) params.append('sort', filters.sort);
    if (filters?.verified !== undefined) params.append('verified', String(filters.verified));
    if (filters?.limit) params.append('limit', String(filters.limit));
    if (filters?.offset) params.append('offset', String(filters.offset));

    const url = `${this.API_BASE}/plugins?${params.toString()}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch single plugin by slug
   */
  static async getPlugin(slug: string): Promise<Plugin> {
    const url = `${this.API_BASE}/plugins/${slug}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch featured plugins
   */
  static async getFeaturedPlugins(limit = 5): Promise<Plugin[]> {
    const url = `${this.API_BASE}/plugins/featured?limit=${limit}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch all icon packs
   */
  static async getIconPacks(filters?: {
    style?: string;
    search?: string;
    sort?: 'popular' | 'recent' | 'rating';
    limit?: number;
    offset?: number;
  }): Promise<{ items: IconPack[]; total: number; hasMore: boolean }> {
    const params = new URLSearchParams();
    if (filters?.style) params.append('style', filters.style);
    if (filters?.search) params.append('search', filters.search);
    if (filters?.sort) params.append('sort', filters.sort);
    if (filters?.limit) params.append('limit', String(filters.limit));
    if (filters?.offset) params.append('offset', String(filters.offset));

    const url = `${this.API_BASE}/icon-packs?${params.toString()}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch single icon pack by slug
   */
  static async getIconPack(slug: string): Promise<IconPack> {
    const url = `${this.API_BASE}/icon-packs/${slug}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch featured icon packs
   */
  static async getFeaturedIconPacks(): Promise<IconPack[]> {
    const url = `${this.API_BASE}/icon-packs/featured`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch all themes
   */
  static async getThemes(sort?: 'popular' | 'recent' | 'rating'): Promise<{ items: Theme[] }> {
    const params = sort ? `?sort=${sort}` : '';
    const url = `${this.API_BASE}/themes${params}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Fetch single theme by slug
   */
  static async getTheme(slug: string): Promise<Theme> {
    const url = `${this.API_BASE}/themes/${slug}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Download plugin package
   */
  static async downloadPlugin(
    slug: string,
    onProgress?: ProgressCallback
  ): Promise<{ filePath: string; checksum: string }> {
    // Fetch metadata
    const plugin = await this.getPlugin(slug);

    // Download to temp directory
    const tempDir = app.getPath('temp');
    const fileName = `${slug}-${plugin.version}.gspl`;
    const filePath = path.join(tempDir, fileName);

    await this.downloadFile(plugin.downloadUrl, filePath, onProgress);

    // Verify checksum
    const checksum = await this.calculateChecksum(filePath);
    if (checksum !== plugin.checksum) {
      await fs.remove(filePath);
      throw new Error('Checksum verification failed');
    }

    // Track download
    await this.trackDownload(plugin.id, 'plugin', plugin.version);

    return { filePath, checksum };
  }

  /**
   * Download icon pack
   */
  static async downloadIconPack(
    slug: string,
    onProgress?: ProgressCallback
  ): Promise<{ filePath: string; checksum: string }> {
    const pack = await this.getIconPack(slug);

    const tempDir = app.getPath('temp');
    const fileName = `${slug}-${pack.version}.zip`;
    const filePath = path.join(tempDir, fileName);

    await this.downloadFile(pack.downloadUrl, filePath, onProgress);

    const checksum = await this.calculateChecksum(filePath);
    if (checksum !== pack.checksum) {
      await fs.remove(filePath);
      throw new Error('Checksum verification failed');
    }

    await this.trackDownload(pack.id, 'icon-pack', pack.version);

    return { filePath, checksum };
  }

  /**
   * Download theme
   */
  static async downloadTheme(
    slug: string,
    onProgress?: ProgressCallback
  ): Promise<{ filePath: string }> {
    const theme = await this.getTheme(slug);

    const themesDir = path.join(app.getPath('userData'), 'themes');
    await fs.ensureDir(themesDir);

    const fileName = `${slug}.json`;
    const filePath = path.join(themesDir, fileName);

    await this.downloadFile(theme.downloadUrl, filePath, onProgress);

    await this.trackDownload(theme.id, 'theme', theme.version);

    return { filePath };
  }

  /**
   * Install plugin (download + extract + activate)
   */
  static async installPlugin(slug: string, onProgress?: ProgressCallback): Promise<void> {
    const { filePath } = await this.downloadPlugin(slug, onProgress);

    const pluginsDir = path.join(app.getPath('userData'), 'plugins');
    await fs.ensureDir(pluginsDir);

    const pluginDir = path.join(pluginsDir, slug);
    await this.extractZip(filePath, pluginDir);

    // Clean up temp file
    await fs.remove(filePath);
  }

  /**
   * Install icon pack (download + extract)
   */
  static async installIconPack(slug: string, onProgress?: ProgressCallback): Promise<void> {
    const { filePath } = await this.downloadIconPack(slug, onProgress);

    const iconPacksDir = path.join(app.getPath('userData'), 'icon-packs');
    await fs.ensureDir(iconPacksDir);

    const packDir = path.join(iconPacksDir, slug);
    await this.extractZip(filePath, packDir);

    await fs.remove(filePath);
  }

  /**
   * Get reviews for an item
   */
  static async getReviews(
    itemId: string,
    sort: 'recent' | 'helpful' | 'rating' = 'recent',
    limit = 10
  ): Promise<Review[]> {
    const url = `${this.API_BASE}/reviews/${itemId}?sort=${sort}&limit=${limit}`;
    const response = await net.fetch(url);
    return response.json();
  }

  /**
   * Submit a review
   */
  static async submitReview(
    itemId: string,
    itemType: 'plugin' | 'icon-pack' | 'theme',
    rating: number,
    comment?: string,
    authToken?: string
  ): Promise<void> {
    const url = `${this.API_BASE}/reviews`;
    const body = {
      itemId,
      itemType,
      rating,
      comment,
    };

    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
    };

    if (authToken) {
      headers['Authorization'] = `Bearer ${authToken}`;
    }

    await net.fetch(url, {
      method: 'POST',
      headers,
      body: JSON.stringify(body),
    });
  }

  /**
   * Get installed plugins
   */
  static async getInstalledPlugins(): Promise<string[]> {
    const pluginsDir = path.join(app.getPath('userData'), 'plugins');
    if (!(await fs.pathExists(pluginsDir))) {
      return [];
    }

    const entries = await fs.readdir(pluginsDir, { withFileTypes: true });
    return entries.filter((entry) => entry.isDirectory()).map((entry) => entry.name);
  }

  /**
   * Get installed icon packs
   */
  static async getInstalledIconPacks(): Promise<string[]> {
    const iconPacksDir = path.join(app.getPath('userData'), 'icon-packs');
    if (!(await fs.pathExists(iconPacksDir))) {
      return [];
    }

    const entries = await fs.readdir(iconPacksDir, { withFileTypes: true });
    return entries.filter((entry) => entry.isDirectory()).map((entry) => entry.name);
  }

  /**
   * Uninstall plugin
   */
  static async uninstallPlugin(slug: string): Promise<void> {
    const pluginDir = path.join(app.getPath('userData'), 'plugins', slug);
    await fs.remove(pluginDir);
  }

  /**
   * Uninstall icon pack
   */
  static async uninstallIconPack(slug: string): Promise<void> {
    const packDir = path.join(app.getPath('userData'), 'icon-packs', slug);
    await fs.remove(packDir);
  }

  // Private helper methods

  private static async downloadFile(
    url: string,
    destPath: string,
    onProgress?: ProgressCallback
  ): Promise<void> {
    const response = await net.fetch(url);
    const total = parseInt(response.headers.get('content-length') || '0', 10);

    const reader = response.body?.getReader();
    if (!reader) throw new Error('Failed to get response body reader');

    const chunks: Uint8Array[] = [];
    let downloaded = 0;

    while (true) {
      const { done, value } = await reader.read();
      if (done) break;

      chunks.push(value);
      downloaded += value.length;

      if (onProgress && total > 0) {
        onProgress({
          downloaded,
          total,
          percent: (downloaded / total) * 100,
        });
      }
    }

    const buffer = Buffer.concat(chunks);
    await fs.writeFile(destPath, buffer);
  }

  private static async calculateChecksum(filePath: string): Promise<string> {
    const buffer = await fs.readFile(filePath);
    return crypto.createHash('sha256').update(buffer).digest('hex');
  }

  private static async extractZip(zipPath: string, destDir: string): Promise<void> {
    await fs.ensureDir(destDir);

    const zip = new AdmZip(zipPath);
    zip.extractAllTo(destDir, true);
  }

  private static async trackDownload(
    itemId: string,
    itemType: 'plugin' | 'icon-pack' | 'theme',
    version: string
  ): Promise<void> {
    try {
      const url = `${this.API_BASE}/downloads`;
      await net.fetch(url, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ itemId, itemType, version }),
      });
    } catch (error) {
      // Silently fail - analytics shouldn't block downloads
      console.error('Failed to track download:', error);
    }
  }
}
