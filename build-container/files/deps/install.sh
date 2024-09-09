#!/bin/bash -xe

apt-get install -y checkinstall

export NAO_CFLAGS="-march=silvermont -mtune=silvermont -mfpmath=sse -msse4.2"

cat <<-EOF > /etc/ccache.conf
	max_size = 1G
	sloppiness = file_macro,pch_defines,time_macros
EOF

echo "set-option -g prefix2 C-a" > /etc/tmux.conf

# allow sudo for "nao"
echo -e "\nnao ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

mkdir /dpkg

cd
/opt/deps/caffe.sh
/opt/deps/compilednn.sh
/opt/deps/flatbuffers.sh
/opt/deps/msgpack.sh
/opt/deps/tflite.sh
/opt/deps/libzip.sh
/opt/deps/libzippp.sh
/opt/deps/skel.sh

# cleanup
rm -rf /usr/share/doc /srv /opt/deps
