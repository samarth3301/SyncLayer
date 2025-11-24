FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential cmake git pkg-config \
    libpq-dev libyaml-cpp-dev libspdlog-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j$(nproc)

FROM ubuntu:22.04 AS runtime

LABEL maintainer="samarth3301"
LABEL description="SyncLayer - PostgreSQL replication microservice"
LABEL version="1.0"

RUN apt-get update && apt-get install -y libpq5 libyaml-cpp0.7 libspdlog1 curl && rm -rf /var/lib/apt/lists/*
WORKDIR /app
RUN mkdir -p /app/config
COPY --from=build /app/build/SyncLayer /app/SyncLayer

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
  CMD curl -f http://localhost:${SYNC_HEALTH_PORT:-8080}/health || exit 1

ENV SYNC_LOG_LEVEL=info
ENV SYNC_HEALTH_PORT=8080
CMD ["/app/SyncLayer"]
