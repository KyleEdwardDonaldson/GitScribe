# Conventional Commits Plugin

Automatically format commit messages to follow the [Conventional Commits](https://www.conventionalcommits.org/) standard.

## Features

- **Auto-formatting**: Formats commit messages as you type
- **Type inference**: Intelligently infers commit type based on message and changed files
- **Scope detection**: Automatically detects scope from file paths
- **Format button**: Manually trigger formatting
- **Customizable**: Configure allowed types and scope requirements

## Commit Format

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

### Types

- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation changes
- `style` - Code style changes (formatting, semicolons, etc.)
- `refactor` - Code refactoring
- `perf` - Performance improvements
- `test` - Adding or updating tests
- `chore` - Maintenance tasks
- `ci` - CI/CD changes
- `build` - Build system or dependency changes

### Examples

```
feat(auth): add OAuth2 login support
fix(parser): resolve null pointer exception
docs(readme): update installation instructions
style(formatting): apply prettier to all files
refactor(api): simplify user authentication logic
perf(query): optimize database index usage
test(auth): add unit tests for login flow
chore(deps): update dependencies
```

## Installation

1. Download from [GitScribe Marketplace](https://gitscribe.dev/marketplace/plugins/conventional-commits)
2. Open GitScribe Full → Settings → Plugins
3. Click "Install from file" → Select downloaded `.gspl` file
4. Enable plugin

## Usage

### Automatic Formatting

When enabled (default), the plugin automatically formats your commit messages as you type in the CommitView.

**Before:**
```
Added new user authentication feature
```

**After:**
```
feat(auth): add new user authentication feature
```

### Manual Formatting

Click the "Format Commit" button in the CommitView to manually format the current message.

### Settings

Configure the plugin in Settings → Plugins → Conventional Commits:

- **Auto-format**: Enable/disable automatic formatting
- **Allowed types**: Select which commit types to use
- **Require scope**: Enforce scope in commit messages (e.g., `feat(auth):` instead of `feat:`)

## How It Works

1. **Type Inference**: Analyzes commit message keywords and changed files to determine the commit type
2. **Scope Detection**: Extracts common directory from changed files as the scope
3. **Description Cleanup**: Removes redundant prefixes and ensures lowercase start
4. **Format Application**: Builds final message in Conventional Commits format

## Permissions

This plugin requires:
- `git:operations` - To read commit information and staged files

## License

MIT

## Support

- [Documentation](https://gitscribe.dev/docs/plugins/conventional-commits)
- [Issues](https://github.com/gitscribe/plugin-conventional-commits/issues)
- [Discussions](https://github.com/gitscribe/plugin-conventional-commits/discussions)
