# Framework

This our framework, which contains all the code, that runs on the robots and makes them play soccer.

The code is split into `backend` and `frontend` which communicate over [shared memory](https://en.wikipedia.org/wiki/Shared_memory) with each other.

There are two `backend` versions, one for `LoLA` which runs on Nao V6 robots and the old `naoqi` one for Nao V5.
Both `backends` do the same tasks: reading sensor values (joint positions, [IMU](https://en.wikipedia.org/wiki/Inertial_measurement_unit), buttons, etc.), 
report them to the `frontend` and setting new actuator values (joint positions, LEDs, etc.) sent from the `frontend`.

The `frontend` does all the complex tasks, such as image processing, calculating walk/motions, localisation, and behavior/strategy.

# How to build

## Common requirements

To cross-compile our framework for the Nao robots, you need to install `make` and `docker`.
On Debian/Ubuntu based systems this can be done with the following commands:

``` sh
sudo apt install make docker.io docker-compose
sudo adduser $USER docker # give your user permission to use docker
# now either log out and back in or reboot
```

After docker is installed you can use our docker containers using:

``` sh
docker login git.bembelbots.de
```

## Using Visual Studio Code (recommended)

Install [Visual Studio Code](https://code.visualstudio.com/) (please use official installer, other packages may cause issues).

Once all the common requirements are installed and configured you should be able to open
the RoboCup soccer codebase (soccer folder of this repository) with vscode. After opening
vscode should prompt you, asking to open the folder in a "Dev Container". Here you need
to select "Reopen in Dev Container".

When successfully reopened vscode should display "Dev Container: Bembelbots" on the bottom left
of the window. If this is the case then you can complete the setup by entering
'task bembelbots: setup' after pressing 'ctrl + p' then hitting enter.
This will run all setup tasks required for the compilation of the framework.
When this process is completed you can build our framework with 'ctrl + shift + b'.

## Using the `make` wrapper

Open a terminal in this folder and run `make`.
On the first run, you are required to login to our docker registry. Either use your git.bembelbots.de username & password or create an [API key](https://git.bembelbots.de/user/settings/applications) that you can use instead of your password.

After compiling the docker container will keep running in the background. You can stop it by running `make shutdown`.
To update the docker build container, run `make update`.

# How to deploy code to a robot

Once you have sucessfully compiled our code, you need to push the binaries to the robot.
This task is automated by our [sync script](https://git.bembelbots.de/Bembelbots/RoboCup/wiki/2-HowTos-%23-Sync-Script).

To deploy your code, you can either use the `Sync frontend` or `Sync all` buttons in [bembelDbug](https://git.bembelbots.de/Bembelbots/RoboCup/wiki/3-Documentation-%23-BembelDbug)
or run the following command:
``` sh
./scripts/sync_nao.sh <robot name/ip>
```

When you are in our lab, connected to our `jrl` wifi, you can use the robot's name, for example `./scripts/sync_nao.sh buddy`, when using a direct ethernet cable connection,
use the robot's IP address instead (see sticker on the back of the head).

If the `backend` has been changed since the last time the robot was synced, use the `-a` option of the sync script or the `Sync all` button in `bembelDbug`.


# Content

| File / Folder          | Description                                                                                                                        |
|------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| build                  | Created once you run make. Compiled binaries for Nao V6 will end up in `build/v6/bin`                                              |
| [[CMakeLists.txt]]     | Main [CMake](https://cmake.org/) project file, that specifies how to compile our C++ to executable programs                        |
| [[cmake]]              | Additional [CMake](https://cmake.org/) macros used by our project                                                                  |
| [[external]]           | Third-party libraries that we use and which are not already installed on the robot                                                 |
| [[firmware]]           | `lola-backend` for Nao V6 robots, that reads sensor data & sets actuators as well as `jrlmonitor` that relays status information   |
| [[frontend]]           | This is all the code reponsible for playing soccer. Communicates with `backend` to get sensor values & set actuators               |
| [[scripts]]            | Shell scripts for invoking cmake, deploying your code to a robot & other tasks                                                     |
| [[tools]]              | Various tools for debugging & testing                                                                                              |
| [[STYLEGUIDE.md]]      | Please follow these style conventions for writing C++ code!                                                                        |
