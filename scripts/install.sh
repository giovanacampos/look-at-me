#!/bin/bash

echo "OpenCV & OpenCV Contrib"

if [ ! -d "/opencv_build" ]; then
    echo "Installing build tools..." && \
    sudo apt install -y build-essential cmake git pkg-config libgtk-3-dev \
        libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
        libxvidcore-dev libx264-dev libjpeg-dev libpng-dev libtiff-dev \
        gfortran openexr libatlas-base-dev \
        libtbb2 libtbb-dev libdc1394-22-dev libopenexr-dev \
        libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev && \

    echo "Downloading source code..." && \
    mkdir /opencv_build && cd /opencv_build && \
    git clone https://github.com/opencv/opencv.git && \
    git clone https://github.com/opencv/opencv_contrib.git && \

    echo "Setting up build folder..." && \
    cd /opencv_build/opencv && \
    mkdir -p build && cd build && \

    echo "Generating make file..." && \
    cmake -D WITH_IPP=ON .. \
        -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local \
        -D INSTALL_C_EXAMPLES=ON \
        -D INSTALL_PYTHON_EXAMPLES=ON \
        -D OPENCV_GENERATE_PKGCONFIG=ON \
        -D OPENCV_EXTRA_MODULES_PATH=/opencv_build/opencv_contrib/modules \
        -D BUILD_EXAMPLES=ON .. && \

    echo "Building source code..." && \
    make -j$(nproc) && \

    echo "Installing built files..." && \
    sudo make install
fi

echo "Testing C++ bindings..." && \
pkg-config --modversion opencv4 && \

# echo "Testing Python bindings..." && \
# python3 -c "import cv2; print(cv2.__version__)" && \

echo "Goodbyte!"