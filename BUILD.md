# Setup

These instructions apply for x86_64 Linux systems.

* download our [GCC cross compiler](https://bembelbots.de/downloads/gcc102-naov6-linux64.tar.xz)
* download the Nao V6 SDK `ctc-linux64-atom-2.8.5.10.zip`
* unpack both in the `ctc` directory. After unpacking, it should contain two subdirectories `gcc` and `ctc-linux64-atom-2.8.5.10`

# Compiling

* run `./crosscompile.sh` in the root directory of the GIT repository
* change to the `build` directory: `cd build`
* run `make`
* after successful build, you will have two executables`jsfrontend` and `lola-backend` in the `bin` subdirectory, which can run on the robot or using our [Webots Controller](https://github.com/Bembelbots/WebotsLoLaController)