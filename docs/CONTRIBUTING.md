# Contributing to PoorCraft

We welcome contributions to the PoorCraft game engine! This document outlines the process for contributing to the project.

## Code of Conduct

Please be respectful, inclusive, and constructive when interacting with the community. We follow the [Contributor Covenant](https://www.contributor-covenant.org/) code of conduct.

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- **Git** installed and configured
- **CMake 3.15+** for building
- **C++17 compatible compiler**
- **Development environment** set up (see [Build Documentation](BUILD.md))

### Development Setup

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/yourusername/PoorCraft.git
   cd PoorCraft
   ```

3. **Initialize submodules**:
   ```bash
   git submodule update --init --recursive
   ```

4. **Create a feature branch**:
   ```bash
   git checkout -b feature/amazing-feature
   ```

5. **Set up development environment**:
   ```bash
   # Configure for your platform
   cmake -B build -DCMAKE_BUILD_TYPE=Debug

   # Build the project
   cmake --build build
   ```

## Development Workflow

### Making Changes

1. **Create an issue** describing your planned changes
2. **Discuss the approach** with maintainers
3. **Implement the feature** following coding standards
4. **Add tests** if applicable
5. **Update documentation** as needed
6. **Test thoroughly** across platforms

### Code Style

The project uses `clang-format` for consistent code formatting:

```bash
# Format all code files
clang-format -i src/**/*.cpp src/**/*.h include/**/*.h

# Check formatting without modifying files
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h include/**/*.h
```

A `.clang-format` configuration file is included in the repository.

### Commit Guidelines

Use **conventional commits** for clear commit messages:

```
feat: add new rendering system
fix: resolve memory leak in texture manager
docs: update API documentation
refactor: optimize shader compilation
test: add unit tests for physics system
```

**Commit message format**:
```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `refactor`: Code restructuring
- `test`: Adding or updating tests
- `chore`: Maintenance tasks
- `ci`: CI/CD changes
- `perf`: Performance improvements

## Pull Request Process

### Before Submitting

1. **Test your changes**:
   ```bash
   # Build in Debug mode
   cmake --build build --config Debug

   # Run tests (when available)
   ctest --build-config Debug

   # Test on multiple platforms if possible
   ```

2. **Update documentation** if your changes affect public APIs

3. **Check formatting**:
   ```bash
   clang-format --dry-run --Werror src/**/*.cpp src/**/*.h include/**/*.h
   ```

4. **Ensure no warnings** with strict compiler flags

### Creating a Pull Request

1. **Rebase** your branch on the latest main:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push** your changes:
   ```bash
   git push origin feature/amazing-feature
   ```

3. **Create a pull request** on GitHub

4. **Fill out the PR template** with:
   - Description of changes
   - Motivation and context
   - Testing performed
   - Screenshots (if applicable)
   - Related issues

### Pull Request Review

- **Maintainers will review** your PR within a few days
- **Address feedback** by making additional commits
- **CI/CD must pass** for merge eligibility
- **At least one approval** required from maintainers

## Testing

### Running Tests

```bash
# Build and run tests
cmake -B build -DENABLE_TESTS=ON
cmake --build build
ctest --build-config Debug

# Run specific test suite
ctest -R "core" --build-config Debug

# Verbose output
ctest -V --build-config Debug
```

### Writing Tests

- **Use Google Test framework** (planned for future)
- **Test public APIs** thoroughly
- **Include edge cases** and error conditions
- **Mock external dependencies** where appropriate
- **Aim for high coverage** of new code

## Documentation

### When to Update Docs

- **New public APIs** require documentation
- **Changed behavior** needs explanation
- **New features** need user guides
- **Architecture changes** need technical documentation

### Documentation Locations

- **API docs**: Inline comments in header files
- **User guides**: `docs/` directory
- **Build instructions**: `docs/BUILD.md`
- **Architecture**: `docs/ARCHITECTURE.md`

## Issue Reporting

### Bug Reports

**Before reporting**:
- Check existing issues for duplicates
- Try the latest development version
- Include complete error messages and stack traces

**Good bug report**:
- **Clear title** describing the issue
- **Steps to reproduce** with minimal example
- **Expected vs actual behavior**
- **Environment details** (OS, compiler, etc.)
- **Error logs** and crash dumps

### Feature Requests

**Good feature request**:
- **Clear description** of the proposed feature
- **Use case explanation** and motivation
- **Proposed implementation approach**
- **Alternative solutions** considered
- **Impact on existing systems**

## Development Best Practices

### Code Quality

- **Write self-documenting code** with clear variable and function names
- **Keep functions small** and focused on single responsibilities
- **Use const correctness** for immutable data
- **Avoid global state** where possible
- **Handle errors gracefully** with appropriate logging

### Performance

- **Profile before optimizing** to identify real bottlenecks
- **Use appropriate data structures** for the task
- **Consider cache locality** for data layout
- **Avoid unnecessary allocations** in hot paths
- **Use move semantics** where appropriate

### Cross-Platform Development

- **Test on multiple platforms** (Windows, Linux, macOS)
- **Use platform abstraction** layer for OS-specific code
- **Handle platform differences** in CMake configuration
- **Consider different compiler behaviors** and standard library implementations

## Community

### Communication

- **GitHub Discussions** for questions and ideas
- **Discord server** for real-time chat (if available)
- **Mailing list** for announcements (if available)
- **Maintainers** available for technical discussions

### Recognition

Contributors are recognized through:
- **GitHub contributor graph**
- **Release notes** mentioning major contributions
- **Special thanks** in documentation
- **Community roles** for active contributors

## Legal

### Licensing

- All contributions must be compatible with the **MIT License**
- Contributors retain copyright but grant usage rights
- **Attribution** must be maintained in distributed versions

### Intellectual Property

- **Respect third-party licenses** for dependencies
- **Original work** is preferred over copied code
- **Proper attribution** for borrowed ideas or algorithms
- **No patent-encumbered code** without explicit approval

## Advanced Contributing

### Becoming a Maintainer

Long-term contributors may be invited to become maintainers with:
- **Write access** to the repository
- **Issue triage** and PR review responsibilities
- **Release management** participation
- **Community moderation** duties

### Architectural Decisions

Significant changes should be discussed as **Architecture Decision Records (ADRs)**:
1. **Context and problem statement**
2. **Considered options** with pros/cons
3. **Chosen solution** with rationale
4. **Consequences** of the decision

## Getting Help

### Resources

- **[Build Documentation](BUILD.md)** - Platform-specific setup
- **[Architecture Documentation](ARCHITECTURE.md)** - System design overview
- **[API Documentation](API.md)** - Public interface reference
- **CMake Documentation** - Build system configuration

### Asking Questions

1. **Check existing documentation** first
2. **Search GitHub issues** and discussions
3. **Ask in appropriate channels** (Discussions vs Issues)
4. **Provide context** and be specific about your question

### Reporting Issues

- **Use issue templates** when available
- **Provide complete information** for faster resolution
- **Follow up** on existing issues rather than creating duplicates
- **Be patient** - maintainers are volunteers

---

Thank you for contributing to PoorCraft! Your help makes the engine better for everyone. 🚀
