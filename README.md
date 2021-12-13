# Look at me

Detect faces and see if they are looking at the camera.

## Setup

Run the command on terminal to setup, download, build, and install OpenCV.

* `bash scripts/install.sh`

## Build

* `mkdir build` will create the build folder, ignore if it exists.
* `cd build` move into the build folder.
* `cmake ..` create a Makefile based on the `CMakeLists.txt` file.
  * `cmake .. -DUSE_SSE2_INSTRUCTIONS=ON` for SSE optimizations.
* `make` builds the project binary into `bin/`.

## Run

* `cd ..` exit the build folder.
* `./bin/look-at-me` displays the camera input and outline faces.
  * `./bin-look-at-me -v <arg>` will open a video file path provided for the argument and outline faces.

## Docker

Before you can run docker you need to get an `xauth` token by running `xauth list`.  Once you have the hash
listed copy and replace it for the one `a076adcfef9b24b6664d78b50ea99a4f` in the `Dockerfile`.

To run the docker image follow these commands:

* `docker build -t look-at-me:dev .`
* `docker run --net=host -e DISPLAY -v /tmp/.X11-unix look-at-me:dev`