# Framework

This our framework, which contains all the code, that runs on the robots and makes them play soccer.

The code is split into `backend` and `frontend` which communicate over [shared memory](https://en.wikipedia.org/wiki/Shared_memory) with each other.

The `backend` reads sensor values (joint positions, [IMU](https://en.wikipedia.org/wiki/Inertial_measurement_unit), buttons, etc.), 
reports them to the `frontend` and sets new actuator values (joint positions, LEDs, etc.) sent from the `frontend`.

The `frontend` does all the complex tasks, such as image processing, calculating walk/motions, localization, and behavior/strategy.

# How to build

## Prerequisits

Install [Docker](https://www.docker.com/get-started/) and [Visual Studio Code](https://code.visualstudio.com/) and clone this repository.

Build the Docker container in [../build-container](../build-container) (may require [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) on Windows).

## Building using Visual Studio Code (recommended)

Once all the common requirements are installed and configured you should be able to open the RoboCup soccer codebase (soccer folder of this repository) with vscode.
After opening VSCode and agreeing to install required extensions, you see a prompt asking to open the folder in a "Dev Container". Here you need to select "Reopen in Dev Container".

When successfully reopened VSCode should display "Dev Container: Bembelbots" on the bottom left of the window.
When this process is completed you can build our framework with 'ctrl + shift + b'.

## Using the `make` wrapper

Open a terminal in this folder and run `make`.
After compiling the docker container will keep running in the background. You can stop it by running `make shutdown`.

# Contents

| File / Folder                        | Description                                                                                                                        |
|--------------------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| build                                | Created when compiling the framework. Compiled binaries for Nao V6 will end up in `build/v6/bin`                                   |
| [CMakeLists.txt](CMakeLists.txt)     | Main [CMake](https://cmake.org/) project file, that specifies how to compile our C++ to executable programs                        |
| [cmake](cmake)                       | Additional [CMake](https://cmake.org/) macros used by our project                                                                  |
| [external](external)                 | Third-party libraries that we use and which are not already installed on the robot                                                 |
| [firmware](firmware)                 | `lola-backend` for Nao V6 robots, that reads sensor data & sets actuators as well as `jrlmonitor` that relays status information   |
| [frontend](frontend)                 | This is all the code reponsible for playing soccer. Communicates with `backend` to get sensor values & set actuators               |
