# Contributing to SyncLayer

Thank you for your interest in contributing to SyncLayer! We welcome contributions from the community. This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Reporting Issues](#reporting-issues)
- [Community](#community)

## Code of Conduct

This project follows a code of conduct to ensure a welcoming environment for all contributors. By participating, you agree to:

- Be respectful and inclusive
- Focus on constructive feedback
- Accept responsibility for mistakes
- Show empathy towards other contributors
- Help create a positive community

## Getting Started

### Prerequisites

Before you begin, ensure you have:

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- PostgreSQL development libraries
- Git

### Quick Setup

```bash
# Fork the repository on GitHub
# Clone your fork
git clone https://github.com/your-username/SyncLayer.git
cd SyncLayer

# Set up the build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Run tests to ensure everything works
cd build && ctest
```

## Development Setup

### Installing Dependencies

#### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y cmake build-essential libpq-dev libyaml-cpp-dev libspdlog-dev
```

#### macOS

```bash
brew install cmake postgresql yaml-cpp spdlog
```

#### Windows

```powershell
# Using vcpkg
vcpkg install postgresql yaml-cpp spdlog
```

### IDE Setup

#### VS Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- Docker (Microsoft)

#### CLion

CLion has built-in CMake and C++ support. Just open the project directory.

## Making Changes

### Branching Strategy

- `main`: Production-ready code
- `develop`: Integration branch for features
- `feature/*`: New features
- `bugfix/*`: Bug fixes
- `hotfix/*`: Critical fixes for production

### Commit Guidelines

We follow conventional commits:

```
type(scope): description

[optional body]

[optional footer]
```

Types:
- `feat`: New features
- `fix`: Bug fixes
- `docs`: Documentation
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Testing
- `chore`: Maintenance

Examples:
```
feat: add configurable health check port
fix: resolve memory leak in connection pool
docs: update installation instructions
```

### Code Style

#### C++ Guidelines

- Use C++17 features
- Follow [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) instead of raw pointers
- Prefer `const` correctness
- Use meaningful variable and function names
- Document complex logic with comments

#### Formatting

We use clang-format for consistent code formatting. The configuration is in `.clang-format`.

```bash
# Format all source files
find src include -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

#### Naming Conventions

- **Classes**: PascalCase (`DatabaseConnection`)
- **Functions/Methods**: camelCase (`connectToDatabase()`)
- **Variables**: camelCase (`connectionString`)
- **Constants**: SCREAMING_SNAKE_CASE (`MAX_RETRIES`)
- **Namespaces**: PascalCase (`SyncLayer::Database`)

## Testing

### Running Tests

```bash
# Build in debug mode
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Run all tests
cd build && ctest --output-on-failure

# Run specific test
cd build && ctest -R test_config
```

### Writing Tests

We use Google Test for unit testing. Tests are located in the `test/` directory.

```cpp
#include <gtest/gtest.h>
#include "your_header.hpp"

TEST(YourClassTest, TestCaseName) {
    // Arrange
    YourClass instance;

    // Act
    auto result = instance.doSomething();

    // Assert
    EXPECT_EQ(result, expected_value);
}
```

### Test Coverage

Aim for high test coverage, especially for:
- Configuration parsing
- Database operations
- Error handling
- Edge cases

## Submitting Changes

### Pull Request Process

1. **Create a branch** from `develop` (or `main` for hotfixes)
2. **Make your changes** following the guidelines above
3. **Write tests** for new functionality
4. **Ensure all tests pass**
5. **Update documentation** if needed
6. **Commit with conventional commit messages**
7. **Push your branch** and create a PR

### PR Template

When creating a pull request, please include:

- **Description**: What changes were made and why
- **Type of change**: Bug fix, feature, documentation, etc.
- **Testing**: How the changes were tested
- **Breaking changes**: Any breaking changes and migration guide
- **Screenshots**: UI changes (if applicable)

### Review Process

- All PRs require review from at least one maintainer
- CI/CD must pass all checks
- Code coverage should not decrease significantly
- Documentation must be updated for user-facing changes

## Reporting Issues

### Bug Reports

When reporting bugs, please include:

- **Description**: Clear description of the issue
- **Steps to reproduce**: Step-by-step instructions
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happens
- **Environment**: OS, compiler, versions
- **Logs**: Relevant log output
- **Additional context**: Screenshots, configuration files

### Feature Requests

For feature requests, please include:

- **Problem**: What's the problem you're trying to solve?
- **Solution**: Proposed solution
- **Alternatives**: Alternative approaches considered
- **Use case**: How would this feature be used?

## Community

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and community discussion
- **GitHub Wiki**: Documentation and guides

### Getting Help

- Check the [documentation](https://github.com/samarth3301/SyncLayer/wiki)
- Search existing issues and discussions
- Ask the community in discussions

### Recognition

Contributors are recognized in:
- The [CONTRIBUTORS](CONTRIBUTORS.md) file
- Release notes
- GitHub's contributor insights

Thank you for contributing to SyncLayer! 🎉