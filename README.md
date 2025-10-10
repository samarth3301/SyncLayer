SyncLayer

Production-ready modular C++ microservice to replicate a local PostgreSQL database to a hosted PostgreSQL database in near real-time.

Build requirements: CMake >= 3.16, C++17 compiler, libpq, libpqxx, yaml-cpp, spdlog.

Build:
cmake -S . -B build
cmake --build build -j

Run:
Edit config/sync-config.yaml or set env vars:
- SYNC_LOCAL_HOST, SYNC_LOCAL_PORT, SYNC_LOCAL_USER, SYNC_LOCAL_PASSWORD, SYNC_LOCAL_DBNAME
- SYNC_HOSTED_HOST, SYNC_HOSTED_PORT, SYNC_HOSTED_USER, SYNC_HOSTED_PASSWORD, SYNC_HOSTED_DBNAME
- SYNC_INTERVAL_SECONDS, SYNC_BATCH_SIZE, SYNC_TABLES (comma-separated)
- SYNC_LOG_LEVEL, SYNC_LOG_FILE

Execute:
./build/SyncLayer

Modules:
- core: Engine orchestrates lifecycle
- config: YAML + env overrides
- logging: spdlog setup
- db: libpq connection wrapper
- tracker: table discovery and change detection stubs
- queue: in-memory queue with drain stub
- replication: orchestrates sync batches
- utils: retry/backoff
