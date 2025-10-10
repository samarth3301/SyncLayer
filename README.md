# SyncLayer

Production-ready modular C++ microservice to replicate a local PostgreSQL database to a hosted PostgreSQL database in near real-time.

**Note**: The sync process only handles INSERT and UPDATE operations. DELETE operations are not replicated to maintain data integrity and prevent accidental data loss in the hosted database.

## Production Features

- **Health Checks**: Validates database connectivity before sync operations.
- **Retry Logic**: Exponential backoff for failed database queries (up to 3 attempts).
- **Efficient Sync**: Initial data sync performed only once at startup, subsequent runs handle incremental changes.
- **Containerized**: Docker image available on Docker Hub for easy deployment.
- **Configurable**: YAML-based configuration for databases, sync parameters, and logging.

## Build Requirements

- CMake >= 3.16
- C++17 compiler
- libpq, libpqxx, yaml-cpp, spdlog

## Local Build and Run

### Build
```bash
cmake -S . -B build
cmake --build build -j
```

### Configuration
Edit `config/sync-config.yaml` or set environment variables:
- `SYNC_LOCAL_HOST`, `SYNC_LOCAL_PORT`, `SYNC_LOCAL_USER`, `SYNC_LOCAL_PASSWORD`, `SYNC_LOCAL_DBNAME`
- `SYNC_HOSTED_HOST`, `SYNC_HOSTED_PORT`, `SYNC_HOSTED_USER`, `SYNC_HOSTED_PASSWORD`, `SYNC_HOSTED_DBNAME`
- `SYNC_INTERVAL_SECONDS`, `SYNC_BATCH_SIZE`, `SYNC_TABLES` (comma-separated)
- `SYNC_LOG_LEVEL`, `SYNC_LOG_FILE`

For detailed YAML configuration, see the Docker section below.

### Run
```bash
./build/SyncLayer
```

## Docker Setup

The microservice is containerized and available on Docker Hub as `samarth3301/synclayer:latest`.

### Pull the Image
```bash
docker pull samarth3301/synclayer:latest
```

### Run with Docker Compose
1. Configure the service by editing `config/sync-config.yaml` with your database details.
2. From the `docker/` directory:
   ```bash
   docker-compose up -d
   ```
   This starts the service in detached mode, running the sync process every hour.

### Configuration

The microservice is configured via `config/sync-config.yaml`. Edit this file to set up your databases and sync parameters:

```yaml
databases:
  local:
    host: your-local-postgres-host
    port: 5432
    user: your-local-username
    password: your-local-password
    dbname: your-local-database
  hosted:
    host: your-hosted-postgres-host
    port: 5432
    user: your-hosted-username
    password: your-hosted-password
    dbname: your-hosted-database

sync:
  interval_seconds: 5  # Internal sync interval (not used in hourly mode)
  batch_size: 50
  auto_fetch: true  # Set to true for auto-discovery of all tables, false to use manual list
  tables: []  # List of tables to sync when auto_fetch is false

logging:
  level: info  # debug, info, warn, error
  file: logs/synclayer.log
```

- **Databases**: Specify connection details for local and hosted PostgreSQL databases.
- **Sync**: Set batch size and table discovery mode. Use `auto_fetch: true` to automatically discover all tables in the public schema, or set to `false` and list specific tables in the `tables` array. The service runs every hour regardless of `interval_seconds`.
- **Logging**: Adjust log level and output file.

### Run Directly with Docker
```bash
docker run -d --name synclayer -v $(pwd)/config:/app/config samarth3301/synclayer:latest
```

### Stop the Service
```bash
docker-compose down
```

## Modules

- **core**: Engine orchestrates lifecycle
- **config**: YAML + env overrides
- **logging**: spdlog setup
- **db**: libpq connection wrapper
- **tracker**: table discovery and change detection stubs
- **queue**: in-memory queue with drain stub
- **replication**: orchestrates sync batches
- **utils**: retry/backoff
