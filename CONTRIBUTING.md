# Contributing to PoorCraft

Thank you for your interest in contributing to PoorCraft! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Code Style](#code-style)
- [Branch Strategy](#branch-strategy)
- [Pull Request Process](#pull-request-process)
- [Testing](#testing)
- [Commit Messages](#commit-messages)
- [Code Review](#code-review)
- [Issue Reporting](#issue-reporting)

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/yourusername/poorcraft.git
   cd poorcraft
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/originalowner/poorcraft.git
   ```
4. **Build the project**:
   ```bash
   ./gradlew build
   ```

## Development Setup

### Prerequisites

- JDK 17 or higher
- Git
- An IDE (IntelliJ IDEA recommended)

### IDE Configuration

**IntelliJ IDEA:**
1. Open the project root directory
2. Wait for Gradle sync to complete
3. Enable EditorConfig support (should be automatic)
4. Configure code style:
   - Settings → Editor → Code Style → Java
   - Set tab size to 4 spaces
   - Set continuation indent to 8 spaces

**Eclipse:**
1. Import as Gradle project
2. Install EditorConfig plugin
3. Run `./gradlew eclipse`

### Running Tests

```bash
# Run all tests
./gradlew test

# Run tests for a specific module
./gradlew :common:test

# Run with verbose output
./gradlew test --info
```

## Code Style

We use EditorConfig to maintain consistent code style. Your IDE should automatically apply these settings.

### Java Conventions

**Naming:**
- Classes: `PascalCase`
- Methods: `camelCase`
- Variables: `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Packages: `lowercase`

**Formatting:**
- Indentation: 4 spaces (no tabs)
- Line length: 120 characters maximum
- Braces: K&R style (opening brace on same line)
- One statement per line

**Documentation:**
- All public classes and methods must have Javadoc
- Use `@param`, `@return`, `@throws` tags appropriately
- Include usage examples for complex APIs

**Example:**
```java
package com.poorcraft.common.util;

/**
 * Utility class for mathematical operations.
 */
public class MathUtil {
    
    /**
     * Calculates the distance between two points in 3D space.
     * 
     * @param x1 X coordinate of first point
     * @param y1 Y coordinate of first point
     * @param z1 Z coordinate of first point
     * @param x2 X coordinate of second point
     * @param y2 Y coordinate of second point
     * @param z2 Z coordinate of second point
     * @return The Euclidean distance between the points
     */
    public static double distance(double x1, double y1, double z1,
                                   double x2, double y2, double z2) {
        double dx = x2 - x1;
        double dy = y2 - y1;
        double dz = z2 - z1;
        return Math.sqrt(dx * dx + dy * dy + dz * dz);
    }
}
```

## Branch Strategy

We use a simplified Git Flow:

- **main** - Stable releases only
- **develop** - Active development branch
- **feature/*** - New features (branch from develop)
- **bugfix/*** - Bug fixes (branch from develop)
- **hotfix/*** - Critical fixes (branch from main)

### Creating a Feature Branch

```bash
git checkout develop
git pull upstream develop
git checkout -b feature/my-new-feature
```

### Keeping Your Branch Updated

```bash
git checkout develop
git pull upstream develop
git checkout feature/my-new-feature
git rebase develop
```

## Pull Request Process

1. **Create a feature branch** from `develop`
2. **Make your changes** following the code style guidelines
3. **Write tests** for new functionality
4. **Ensure all tests pass**: `./gradlew test`
5. **Update documentation** if needed
6. **Commit your changes** with descriptive messages
7. **Push to your fork**:
   ```bash
   git push origin feature/my-new-feature
   ```
8. **Open a Pull Request** on GitHub
9. **Fill out the PR template** with:
   - Description of changes
   - Related issue numbers
   - Testing performed
   - Screenshots (if UI changes)
10. **Address review feedback** promptly

### PR Checklist

- [ ] Code follows the project's style guidelines
- [ ] Self-review of code completed
- [ ] Comments added for complex logic
- [ ] Documentation updated
- [ ] No new warnings introduced
- [ ] Tests added for new features
- [ ] All tests pass locally
- [ ] Dependent changes merged

## Testing

### Writing Tests

- Place tests in `src/test/java` matching the package structure
- Use JUnit 5 for all tests
- Aim for high code coverage (>80% for new code)
- Test edge cases and error conditions

**Example:**
```java
package com.poorcraft.common.util;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

class MathUtilTest {
    
    @Test
    void testDistance() {
        double result = MathUtil.distance(0, 0, 0, 3, 4, 0);
        assertEquals(5.0, result, 0.001);
    }
    
    @Test
    void testDistanceSamePoint() {
        double result = MathUtil.distance(1, 2, 3, 1, 2, 3);
        assertEquals(0.0, result, 0.001);
    }
}
```

### Test Categories

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test interaction between components
- **Performance Tests**: Verify performance requirements (for critical paths)

## Commit Messages

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Code style changes (formatting, no logic change)
- **refactor**: Code refactoring
- **perf**: Performance improvements
- **test**: Adding or updating tests
- **chore**: Build process or auxiliary tool changes

### Examples

```
feat(rendering): add chunk frustum culling

Implement frustum culling to avoid rendering chunks outside the camera view.
This improves performance by 30% in typical scenarios.

Closes #123
```

```
fix(networking): resolve packet deserialization error

Fixed NullPointerException when deserializing empty packets.

Fixes #456
```

## Code Review

All submissions require review before merging.

### For Contributors

- Be responsive to feedback
- Don't take criticism personally
- Ask questions if feedback is unclear
- Make requested changes promptly

### For Reviewers

- Be respectful and constructive
- Explain the reasoning behind suggestions
- Approve when changes meet standards
- Use GitHub's suggestion feature for small fixes

## Issue Reporting

### Bug Reports

When reporting bugs, include:

1. **Description**: Clear description of the bug
2. **Steps to Reproduce**: Detailed steps to reproduce the issue
3. **Expected Behavior**: What should happen
4. **Actual Behavior**: What actually happens
5. **Environment**:
   - OS and version
   - Java version
   - PoorCraft version
   - Graphics card and driver version
6. **Logs**: Relevant log files from `logs/poorcraft.log`
7. **Screenshots**: If applicable

### Feature Requests

When requesting features, include:

1. **Description**: Clear description of the feature
2. **Use Case**: Why this feature is needed
3. **Proposed Solution**: How you envision it working
4. **Alternatives**: Other solutions you've considered
5. **Additional Context**: Any other relevant information

## Community Guidelines

- Be respectful and inclusive
- Help others learn and grow
- Give credit where credit is due
- Follow the [Code of Conduct](CODE_OF_CONDUCT.md)

## Questions?

- **Discord**: Join our [Discord server](https://discord.gg/yourserver)
- **Discussions**: Use [GitHub Discussions](https://github.com/yourusername/poorcraft/discussions)
- **Email**: dev@poorcraft.com

Thank you for contributing to PoorCraft! 🎮
