#!/bin/bash -xe

PKG=tensorflow-lite

cd

# install tf-lite
TF_RELEASE="2.11.0"
wget --progress=dot:giga https://github.com/tensorflow/tensorflow/archive/refs/tags/v${TF_RELEASE}.zip
unzip "v${TF_RELEASE}.zip"

mkdir tensorflow-lite-build
cd tensorflow-lite-build

cmake -Wno-dev -DCMAKE_C_FLAGS="${NAO_CFLAGS}" -DCMAKE_CXX_FLAGS="${NAO_CFLAGS}" \
	../tensorflow-${TF_RELEASE}/tensorflow/lite/
make -j$(nproc)

# merge static libs
ar -M <<-EOF
	CREATE libabsl.a
	$(find ./_deps/abseil-cpp-build/absl/ -name \*.a | sed 's/^/ADDLIB /')
	SAVE
	END
EOF
ranlib libabsl.a

ar -M <<EOF
	CREATE libruy.a
	$(find ./_deps/ruy-build/ruy/ -maxdepth 1 -name \*.a | sed 's/^/ADDLIB /')
	SAVE
	END
EOF
ranlib libruy.a

mkdir -p /usr/local/lib
install -m644 _deps/cpuinfo-build/libcpuinfo.a _deps/xnnpack-build/libXNNPACK.a \
	pthreadpool/libpthreadpool.a _deps/cpuinfo-build/deps/clog/libclog.a \
	_deps/farmhash-build/libfarmhash.a _deps/gemmlowp-build/libgemmlowp_eight_bit_int_gemm.a \
	_deps/fft2d-build/libfft2d_fftsg.a _deps/fft2d-build/libfft2d_fftsg2d.a \
	libabsl.a libruy.a libtensorflow-lite.a \
	/usr/local/lib/

# install headers
mkdir -p /usr/local/include/tensorflow
cp -r ~/tensorflow-${TF_RELEASE}/tensorflow/lite /usr/local/include/tensorflow

tar cJf /dpkg/tensorflow-lite.tar.xz /usr/local/include/tensorflow /usr/local/lib/{libcpuinfo.a,libXNNPACK.a,libpthreadpool.a,libclog.a,libfarmhash.a,libgemmlowp_eight_bit_int_gemm.a,libfft2d_fftsg.a,libfft2d_fftsg2d.a,libabsl.a,libruy.a,libtensorflow-lite.a}

# cleanup
cd
rm -rf "v${TF_RELEASE}.zip" tensorflow*
