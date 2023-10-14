#!/bin/bash -xe

PKG=libzip
VERSION=1.10.1

wget "https://github.com/nih-at/libzip/releases/download/v${VERSION}/${PKG}-${VERSION}.tar.xz"
tar xf "${PKG}-${VERSION}.tar.xz"
DIR="${PKG}-${VERSION}"

mkdir "${DIR}/build"
cd "${DIR}/build"
cmake -Wno-dev -DBUILD_DOC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_REGRESS=OFF ..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd
rm -rf "${PKG}-${VERSION}.tar.gz" "$DIR"
