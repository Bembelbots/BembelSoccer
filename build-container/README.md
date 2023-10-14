# Bembelbots Build Container

This folder contains everything to build the BembelBuild docker container, that is used for cross-compiling the Bembelbots soccer framework for our Debian 12 (bookworm) 64bit Nao V6 OS.

To build the container, install docker & docker-compose and run the `build.sh` script.
For faster re-builds, you can install `apt-cacher-ng` on the host, which will cache all debian packages downloaded while building the container.
