# ===== Build stage =====
FROM debian:12-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY server_generator/ /src/

RUN cmake -S . -B build -DBUILD_TESTS=OFF \
 && cmake --build build --parallel

# ===== Runtime stage =====
FROM debian:12-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/server_generator /usr/local/bin/server_generator

EXPOSE 6000

CMD ["/usr/local/bin/server_generator"]
