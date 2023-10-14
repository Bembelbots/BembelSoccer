#!/bin/bash -xe

PKG=flatbuffers
FBS_VERSION="23.5.26"
ZIP_FILE="v${FBS_VERSION}.zip"
SRC_DIR="flatbuffers-${FBS_VERSION}"

# install CompiledNN
wget "https://github.com/google/flatbuffers/archive/refs/tags/v${FBS_VERSION}.zip"
unzip "${ZIP_FILE}"
cd "${SRC_DIR}"
mkdir build
cd build
cmake -Wno-dev -DCMAKE_C_FLAGS="${NAO_CFLAGS}" -DCMAKE_CXX_FLAGS="${NAO_CFLAGS}" \
   -DFLATBUFFERS_BUILD_CPP17=ON -DFLATBUFFERS_BUILD_SHAREDLIB=ON -DFLATBUFFERS_ENABLE_PCH=ON \
   ..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd ../..
rm -rf "${ZIP_FILE}" "${SRC_DIR}"
