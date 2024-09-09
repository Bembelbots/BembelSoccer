#!/bin/bash -xe

PKG=compilednn

# install CompiledNN
wget https://github.com/bhuman/CompiledNN/archive/refs/heads/master.zip
unzip master.zip
mkdir CompiledNN-master/build
cd CompiledNN-master/build
cmake -Wno-dev -DCMAKE_C_FLAGS="${NAO_CFLAGS}" -DCMAKE_CXX_FLAGS="${NAO_CFLAGS}" ..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd
rm -rf master.zip CompiledNN-master
