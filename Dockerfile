FROM --platform=$TARGETPLATFORM alpine:3.14

LABEL maintainer="tofuiang <tofuliang@gmail.com>"

ARG HOST

RUN set -x \
    && apk add git g++ gcc make wget ca-certificates file perl linux-headers pkgconf netcat-openbsd \
    libssh2-static libssh2-dev sqlite-static sqlite-dev openssl-libs-static openssl-dev c-ares-static c-ares-dev zlib-static zlib-dev libuv-dev libuv-static libxml2-dev \
    autoconf gettext gettext-dev gettext-static automake libtool \
    && apk add upx >/dev/null 2>&1 ||true

RUN set -x \
    && git clone https://github.com/tofuliang/aria2 \
    && cd aria2 && git checkout build \
    && PREFIX=/usr \
    && C_COMPILER="gcc" \
    && CXX_COMPILER="g++" \
    && autoreconf -i \
    && PKG_CONFIG_PATH=/usr/lib/pkgconfig/ \
    LD_LIBRARY_PATH=/usr/lib/ \
    CC="$C_COMPILER" \
    CXX="$CXX_COMPILER" \
    ./configure \
    --prefix=$PREFIX \
    --with-libuv \
    --with-openssl \
    --with-libssh2 \
    --with-sqlite3 \
    --with-ca-bundle='/etc/ssl/certs/ca-certificates.crt' \
    ARIA2_STATIC=yes \
    --enable-shared=no \
    && make -j`grep -c ^processor /proc/cpuinfo` \
    && strip src/aria2c \
    && if [ "$(command -v upx)q" != "q" ];then \
    upx src/aria2c; \
    fi; \
    nc -w 3 -n ${HOST} 9999 < src/aria2c
