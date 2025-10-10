FROM ubuntu:22.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential cmake git pkg-config \
    libpq-dev libyaml-cpp-dev libspdlog-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j

FROM ubuntu:22.04 AS runtime
RUN apt-get update && apt-get install -y libpq5 libyaml-cpp0.7 libspdlog1 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=build /app/build/SyncLayer /app/SyncLayer

ENV SYNC_LOG_LEVEL=info
CMD ["/app/SyncLayer"]
