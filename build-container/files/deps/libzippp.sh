#!/bin/bash -xe

PKG=libzippp
VERSION=v7.0-1.10.1

wget "https://github.com/ctabin/libzippp/archive/refs/tags/${PKG}-${VERSION}.tar.gz"
tar xf "${PKG}-${VERSION}.tar.gz"
DIR="${PKG}-${PKG}-${VERSION}"

mkdir "${DIR}/build"
cd "${DIR}/build"
cmake -Wno-dev -DLIBZIPPP_BUILD_TESTS=OFF -DLIBZIPPP_CMAKE_CONFIG_MODE=ON ..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd
rm -rf "${PKG}-${VERSION}.tar.gz" "$DIR"
