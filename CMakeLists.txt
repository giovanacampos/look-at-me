cmake_minimum_required(VERSION 3.16)

project(LookAtMe VERSION 1.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

set(
    OPENCV_LIBS
    -lopencv_calib3d
    -lopencv_core
    -lopencv_highgui
    -lopencv_imgcodecs
    -lopencv_imgproc
    -lopencv_objdetect
    -lopencv_videoio
    )

configure_file(LookAtMe.h.in LookAtMe.h)

include(FetchContent)
FetchContent_Declare(dlib
    GIT_REPOSITORY https://github.com/davisking/dlib.git
    GIT_TAG        v19.22
)
FetchContent_MakeAvailable(dlib)

add_executable(
    look-at-me
    src/main.cpp
    )

target_include_directories(
    look-at-me PUBLIC
    "${PROJECT_BINARY_DIR}"
    /usr/local/include/opencv4
    src
    )

target_link_libraries(
    look-at-me
    dlib::dlib
    ${OPENCV_LIBS}
    )
