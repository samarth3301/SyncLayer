# SyncLayer

[![Docker Pulls](https://img.shields.io/docker/pulls/samarth3301/synclayer)](https://hub.docker.com/r/samarth3301/synclayer)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)

Production-ready modular C++ microservice to replicate a local PostgreSQL database to a hosted PostgreSQL database in near real-time.

**Note**: The sync process only handles INSERT and UPDATE operations. DELETE operations are not replicated to maintain data integrity and prevent accidental data loss in the hosted database.

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Health Check API](#health-check-api)
- [Development](#development)
- [Contributing](#contributing)
- [Code of Conduct](#code-of-conduct)
- [License](#license)
- [Support](#support)

## Features

- **🔄 Real-time Replication**: Near real-time sync between PostgreSQL databases
- **⚡ High Performance**: Batched operations and optimized queries for maximum throughput
- **🏥 Health Monitoring**: Comprehensive health checks with detailed database metrics
- **🐳 Container Ready**: Docker-first deployment with multi-instance support
- **🔧 Highly Configurable**: YAML-based configuration with environment variable overrides
- **🛡️ Production Ready**: Retry logic, error handling, and graceful degradation
- **📊 Monitoring**: HTTP health endpoint with JSON metrics
- **🧪 Well Tested**: Comprehensive unit test suite with Google Test

## Quick Start

```bash
# Pull the latest image
docker pull samarth3301/synclayer:latest

# Run with your config
docker run -d --name synclayer \
  -p 8080:8080 \
  -v $(pwd)/config:/app/config \
  samarth3301/synclayer:latest
```

## Installation

### Docker (Recommended)

The easiest way to run SyncLayer is using Docker:

```bash
# Pull from Docker Hub
docker pull samarth3301/synclayer:latest

# Or build locally
docker build -t synclayer -f docker/Dockerfile .
```

### Local Build

#### Prerequisites

- CMake >= 3.16
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- PostgreSQL development libraries
- yaml-cpp, spdlog

#### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y cmake build-essential libpq-dev libyaml-cpp-dev libspdlog-dev
```

#### macOS

```bash
brew install cmake postgresql yaml-cpp spdlog
```

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/samarth3301/SyncLayer.git
cd SyncLayer

# Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run tests
cd build && ctest

# Install (optional)
sudo cmake --install build
```

## Configuration

SyncLayer uses YAML configuration files. Create a `config/sync-config.yaml` file:

```yaml
databases:
  local:
    host: localhost
    port: 5432
    user: your-user
    password: your-password
    dbname: source-db
  hosted:
    host: your-hosted-db.com
    port: 5432
    user: hosted-user
    password: hosted-password
    dbname: target-db

sync:
  interval_seconds: 5
  batch_size: 100
  auto_fetch: true
  tables: []

logging:
  level: info
  file: logs/synclayer.log

health:
  port: 8080
```

### Environment Variables

All configuration options can be overridden with environment variables:

- `SYNC_CONFIG_PATH`: Path to config file (default: `./config/sync-config.yaml`)
- `SYNC_LOCAL_HOST`, `SYNC_LOCAL_PORT`, etc.: Database connection details
- `SYNC_BATCH_SIZE`: Batch size for operations
- `SYNC_LOG_LEVEL`: Logging level (debug, info, warn, error)
- `SYNC_HEALTH_PORT`: Health check port

## Usage

### Docker Compose (Multiple Instances)

```yaml
version: '3.8'
services:
  synclayer-prod:
    image: samarth3301/synclayer:latest
    ports:
      - "8080:8080"
    volumes:
      - ./config/prod:/app/config
    environment:
      - SYNC_HEALTH_PORT=8080

  synclayer-staging:
    image: samarth3301/synclayer:latest
    ports:
      - "8081:8081"
    volumes:
      - ./config/staging:/app/config
    environment:
      - SYNC_HEALTH_PORT=8081
```

### Direct Docker Run

```bash
# Single instance
docker run -d --name synclayer \
  -p 8080:8080 \
  -v $(pwd)/config:/app/config \
  samarth3301/synclayer:latest

# Multiple instances
docker run -d --name synclayer1 -p 8081:8081 \
  -e SYNC_HEALTH_PORT=8081 \
  -v $(pwd)/config1:/app/config \
  samarth3301/synclayer:latest
```

### Local Execution

```bash
./build/SyncLayer
```

## Health Check API

SyncLayer provides a comprehensive health check endpoint:

```bash
curl http://localhost:8080/health
```

**Response:**
```json
{
  "status": "healthy",
  "databases": {
    "local": {
      "status": "healthy",
      "connections": 2,
      "size_mb": 150,
      "active_queries": 1
    },
    "hosted": {
      "status": "healthy",
      "connections": 1,
      "size_mb": 75,
      "active_queries": 0
    }
  }
}
```

The health check monitors:
- Database connectivity
- Connection pool status
- Database size and performance metrics
- Active query counts

## Development

### Building with Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

### Code Structure

```
├── include/           # Header files
│   ├── core/         # Engine orchestration
│   ├── config/       # Configuration management
│   ├── db/           # Database connections
│   ├── health/       # Health monitoring
│   ├── logging/      # Logging utilities
│   ├── replication/  # Core sync logic
│   ├── tracker/      # Table discovery
│   ├── queue/        # Operation queuing
│   └── utils/        # Utilities (retry, etc.)
├── src/              # Implementation files
├── test/             # Unit tests
├── config/           # Example configurations
├── docker/           # Containerization files
└── CMakeLists.txt    # Build configuration
```

### Architecture

SyncLayer follows a modular architecture:

1. **Engine**: Main orchestration component
2. **ReplicationManager**: Handles sync operations
3. **HealthServer**: HTTP health endpoint
4. **DBConnection**: PostgreSQL connection wrapper
5. **TableTracker**: Schema discovery and change detection
6. **QueueHandler**: Operation batching and queuing

## Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Quick Setup for Contributors

```bash
# Fork and clone
git clone https://github.com/your-username/SyncLayer.git
cd SyncLayer

# Set up development environment
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Run tests
cd build && ctest

# Make your changes...
```

### Development Guidelines

- Follow C++17 best practices
- Write comprehensive unit tests
- Update documentation for API changes
- Ensure all tests pass before submitting PRs

## Code of Conduct

This project has adopted a code of conduct to ensure a welcoming and inclusive environment for all contributors. Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before participating.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

- 📖 [Documentation](https://github.com/samarth3301/SyncLayer/wiki)
- 🐛 [Issue Tracker](https://github.com/samarth3301/SyncLayer/issues)
- 💬 [Discussions](https://github.com/samarth3301/SyncLayer/discussions)
- 📧 Contact: samarth3301@example.com

## Acknowledgments

- Built with [spdlog](https://github.com/gabime/spdlog) for logging
- Configuration powered by [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- Testing framework: [Google Test](https://github.com/google/googletest)
- HTTP server: [cpp-httplib](https://github.com/yhirose/cpp-httplib)

---

<p align="center">Made with ❤️ by <a href="https://github.com/samarth3301">samarth3301</a></p>
