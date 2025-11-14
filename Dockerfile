# -------------------------------------------------
# 1️⃣ Builder – pre‑install system deps & cache xmake pkgs
# -------------------------------------------------
FROM debian:latest AS builder
ENV XMAKE_ROOT=y
ENV XMAKE_GLOBALDIR=/opt/xmake

RUN apt-get update && apt-get install -y --no-install-recommends \
    curl git build-essential zip unzip ca-certificates \
    gawk pkg-config autoconf automake libx11-dev libtool \
    python3 python3-pip asciidoc xmlto \
    libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    protobuf-compiler libprotobuf-dev libgrpc++-dev \
    libcurl4-openssl-dev zlib1g-dev meson ninja-build && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Install Emscripten SDK
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk && \
    cd /opt/emsdk && \
    ./emsdk install latest && \
    ./emsdk activate latest && \
    /bin/bash -c "source /opt/emsdk/emsdk_env.sh && echo 'source /opt/emsdk/emsdk_env.sh' >> /etc/profile"

ENV PATH="/opt/emsdk:/opt/emsdk/node/12.18.1_64bit/bin:/opt/emsdk/upstream/emscripten:$PATH"

RUN curl -fsSL https://raw.githubusercontent.com/tboox/xmake/master/scripts/get.sh | bash && \
    ln -s /root/.local/bin/xmake /usr/local/bin/xmake && \
    xmake --version

WORKDIR /tmp/xmake-cache
COPY xmake.lua .

# Source Emscripten environment and run all xmake wasm config and dependency steps in one block
RUN bash -c "source /opt/emsdk/emsdk_env.sh && xmake f -p wasm -y && xmake require -y"

# -------------------------------------------------
# 2️⃣ Build – compile the project using the cached pkgs
# -------------------------------------------------
FROM builder AS build
WORKDIR /app
COPY ./src ./src
COPY ./resources ./resources
COPY ./web ./web
COPY xmake.lua .
ENV XMAKE_SKIP_CONFIRM=1

# Source Emscripten environment and build for wasm
RUN bash -c "source /opt/emsdk/emsdk_env.sh && xmake f -c -m release -p wasm -y && xmake build -y"

# -------------------------------------------------
# 3️⃣ Runtime – minimal image with the front end only
# -------------------------------------------------
FROM alpine:latest AS runtime
WORKDIR /app
RUN apk add --no-cache python3
COPY --from=build /app/build/web/ ./http
WORKDIR /app/http

EXPOSE 80/tcp

CMD [ "python3", "-m", "http.server", "80" ]