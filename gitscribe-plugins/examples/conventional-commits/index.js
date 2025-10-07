/**
 * Conventional Commits Plugin for GitScribe
 *
 * Formats commit messages to follow the Conventional Commits standard:
 * https://www.conventionalcommits.org/
 *
 * Format: <type>(<scope>): <description>
 *
 * Example: feat(auth): add OAuth2 login support
 */

export default {
  name: 'Conventional Commits',
  version: '1.0.0',

  /**
   * Called when plugin is activated
   */
  async onActivate(api) {
    console.log('Conventional Commits plugin activated');

    // Hook into commit message input
    api.commits.onBeforeCommit(async (context) => {
      const { message, files } = context;

      // Format message if not already formatted
      const formatted = this.formatMessage(message, files);

      return {
        ...context,
        message: formatted,
      };
    });

    // Add UI button to CommitView
    api.ui.addButton({
      location: 'commit-view',
      label: 'Format Commit',
      icon: 'format',
      onClick: async () => {
        const currentMessage = await api.commits.getMessage();
        const formatted = this.formatMessage(currentMessage);
        await api.commits.setMessage(formatted);
        api.ui.showNotification('Message formatted to Conventional Commits', 'success');
      },
    });

    // Add settings panel
    api.settings.registerSection({
      id: 'conventional-commits',
      title: 'Conventional Commits',
      fields: [
        {
          id: 'autoFormat',
          type: 'boolean',
          label: 'Automatically format commit messages',
          default: true,
        },
        {
          id: 'allowedTypes',
          type: 'multiselect',
          label: 'Allowed commit types',
          options: [
            { value: 'feat', label: 'feat - New feature' },
            { value: 'fix', label: 'fix - Bug fix' },
            { value: 'docs', label: 'docs - Documentation' },
            { value: 'style', label: 'style - Code style' },
            { value: 'refactor', label: 'refactor - Code refactoring' },
            { value: 'perf', label: 'perf - Performance improvement' },
            { value: 'test', label: 'test - Tests' },
            { value: 'chore', label: 'chore - Maintenance' },
            { value: 'ci', label: 'ci - CI/CD' },
            { value: 'build', label: 'build - Build system' },
          ],
          default: ['feat', 'fix', 'docs', 'style', 'refactor', 'perf', 'test', 'chore'],
        },
        {
          id: 'requireScope',
          type: 'boolean',
          label: 'Require scope (e.g., feat(auth): ...)',
          default: false,
        },
      ],
    });
  },

  /**
   * Called when plugin is deactivated
   */
  async onDeactivate() {
    console.log('Conventional Commits plugin deactivated');
  },

  /**
   * Format commit message to Conventional Commits standard
   */
  formatMessage(message, files = []) {
    if (!message || message.trim().length === 0) {
      return this.suggestMessage(files);
    }

    // Check if already formatted
    const conventionalPattern = /^(feat|fix|docs|style|refactor|perf|test|chore|ci|build)(\([a-z0-9-]+\))?:\s.+/;
    if (conventionalPattern.test(message)) {
      return message; // Already formatted
    }

    // Try to infer type from message
    const type = this.inferType(message, files);
    const scope = this.inferScope(files);

    // Extract description (remove common prefixes)
    let description = message
      .replace(/^(add|added|fix|fixed|update|updated|remove|removed)\s+/i, '')
      .trim();

    // Ensure description starts with lowercase
    description = description.charAt(0).toLowerCase() + description.slice(1);

    // Build formatted message
    let formatted = type;
    if (scope) {
      formatted += `(${scope})`;
    }
    formatted += `: ${description}`;

    return formatted;
  },

  /**
   * Infer commit type from message and changed files
   */
  inferType(message, files) {
    const lower = message.toLowerCase();

    // Check for keywords
    if (lower.includes('fix') || lower.includes('bug') || lower.includes('issue')) {
      return 'fix';
    }
    if (lower.includes('doc') || lower.includes('readme')) {
      return 'docs';
    }
    if (lower.includes('test') || lower.includes('spec')) {
      return 'test';
    }
    if (lower.includes('refactor')) {
      return 'refactor';
    }
    if (lower.includes('perf') || lower.includes('optimize')) {
      return 'perf';
    }
    if (lower.includes('style') || lower.includes('format')) {
      return 'style';
    }
    if (lower.includes('ci') || lower.includes('pipeline')) {
      return 'ci';
    }
    if (lower.includes('build') || lower.includes('deps')) {
      return 'build';
    }
    if (lower.includes('chore')) {
      return 'chore';
    }

    // Infer from file changes
    if (files.length > 0) {
      const hasTests = files.some(f => f.path.includes('test') || f.path.includes('spec'));
      const hasDocs = files.some(f => f.path.match(/\.(md|txt)$/i));
      const hasConfig = files.some(f => f.path.match(/\.(json|yml|yaml|toml)$/i));

      if (hasTests) return 'test';
      if (hasDocs) return 'docs';
      if (hasConfig) return 'chore';
    }

    // Default to 'feat' (new feature)
    return 'feat';
  },

  /**
   * Infer scope from changed files
   */
  inferScope(files) {
    if (files.length === 0) return null;

    // Extract common directory
    const paths = files.map(f => f.path);
    const commonDir = this.getCommonDirectory(paths);

    if (!commonDir || commonDir === '.') return null;

    // Use first segment as scope
    const segments = commonDir.split('/').filter(s => s.length > 0);
    if (segments.length === 0) return null;

    return segments[0];
  },

  /**
   * Get common directory from file paths
   */
  getCommonDirectory(paths) {
    if (paths.length === 0) return '';
    if (paths.length === 1) {
      const segments = paths[0].split('/');
      segments.pop(); // Remove filename
      return segments.join('/');
    }

    const segments = paths[0].split('/');
    for (let i = 0; i < segments.length; i++) {
      const segment = segments[i];
      if (!paths.every(p => p.split('/')[i] === segment)) {
        return segments.slice(0, i).join('/');
      }
    }

    return segments.join('/');
  },

  /**
   * Suggest a commit message based on changed files
   */
  suggestMessage(files) {
    if (files.length === 0) {
      return 'chore: update';
    }

    const type = this.inferType('', files);
    const scope = this.inferScope(files);

    // Generate description
    const added = files.filter(f => f.status === 'added').length;
    const modified = files.filter(f => f.status === 'modified').length;
    const deleted = files.filter(f => f.status === 'deleted').length;

    let description = 'update';
    if (added > 0 && modified === 0 && deleted === 0) {
      description = `add ${added} file${added > 1 ? 's' : ''}`;
    } else if (modified > 0 && added === 0 && deleted === 0) {
      description = `update ${modified} file${modified > 1 ? 's' : ''}`;
    } else if (deleted > 0 && added === 0 && modified === 0) {
      description = `remove ${deleted} file${deleted > 1 ? 's' : ''}`;
    } else {
      description = `update ${files.length} file${files.length > 1 ? 's' : ''}`;
    }

    let message = type;
    if (scope) {
      message += `(${scope})`;
    }
    message += `: ${description}`;

    return message;
  },
};
