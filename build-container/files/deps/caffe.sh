#!/bin/bash -xe

PKG=caffe

# install caffe
wget "https://github.com/tkalbitz/caffe/archive/refs/heads/nao-optimized-caffe.zip"
unzip nao-optimized-caffe.zip
cd caffe-nao-optimized-caffe
patch -p1 < /opt/deps/nao-caffe.patch
mkdir build
cd build
cmake -Wno-dev -DBLAS=Open -DBUILD_docs=OFF -DBUILD_python=OFF -DBUILD_SHARED_LIBS=ON \
	-DUSE_LEVELDB=ON -DUSE_LMDB=ON -DUSE_NCCL=OFF \
	-DCPU_ONLY=ON -DCMAKE_INSTALL_PREFIX=/usr/local ..
make -j$(nproc) install
checkinstall -D -y --pkgname=$PKG make install
cp *.deb /dpkg

cd
rm -rf nao-optimized-caffe.zip caffe-nao-optimized-caffe
