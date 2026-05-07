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

# O servidor le/grava artefatos sob `config/` relativo ao cwd
# (config.conf, basic.{bin,hex}, detail.{bin,hex}, log/Frame_*.log,
# log/frame_duration.txt). Sem WORKDIR + diretorio pre-criado, o
# write_frame_duration_file e o save() do ATSC_Config falham silenciosamente
# e o frontend nao recebe duracao de frame nem bitrate.
WORKDIR /app
RUN mkdir -p config/log

EXPOSE 6000

CMD ["/usr/local/bin/server_generator"]
