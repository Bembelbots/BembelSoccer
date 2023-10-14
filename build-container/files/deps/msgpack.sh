#!/bin/bash -xe

PKG=msgpack

# install msgpack
VERSION="2.1.5"
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-${VERSION}/msgpack-${VERSION}.tar.gz
tar xf msgpack-${VERSION}.tar.gz
mkdir -p msgpack-2.1.5/build
cd msgpack-2.1.5/build
cmake -Wno-dev -DCMAKE_C_FLAGS="${NAO_CFLAGS}" -DCMAKE_CXX_FLAGS="${NAO_CFLAGS}" \
	-DMSGPACK_CXX11=ON -DMSGPACK_BUILD_TESTS=OFF -DMSGPACK_BUILD_EXAMPLES=OFF \
	-DMSGPACK_ENABLE_CXX=ON -DMSGPACK_ENABLE_SHARED=OFF \
	..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd
rm -rf master.zip CompiledNN-master
