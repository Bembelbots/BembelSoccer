FROM debian:bookworm-slim
LABEL Description="Bembelbots Build container"

SHELL ["/bin/bash", "-c"]

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
	apt-get -y --no-install-recommends install auto-apt-proxy busybox && \
	apt-get -y --no-install-recommends install \
		sudo build-essential gcc g++ ccache cmake cmake-curses-gui ninja-build bash-completion \
		libgtest-dev libboost-all-dev libopencv-dev libhdf5-dev libfftw3-dev libopenblas-dev \
		libgoogle-glog-dev libprotobuf-dev libgflags-dev libeigen3-dev libbz2-dev libzstd-dev \
		libasound2-dev zlib1g-dev libleveldb-dev  liblmdb-dev libsnappy-dev libspeechd-dev \
		googletest wget git git-lfs unzip ca-certificates protobuf-compiler procps psmisc \
		tmux gdb valgrind rsync

COPY files/deps /opt/deps
COPY files/*.sh /opt/

RUN /opt/deps/install.sh

CMD ["/bin/bash", "-l"]
WORKDIR /src/soccer
ENTRYPOINT ["/opt/entrypoint.sh"]
