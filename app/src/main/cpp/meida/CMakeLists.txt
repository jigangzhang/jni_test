cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_GLEXT_PROTOTYPES")

add_library(
        media-lib
        SHARED
        media.cpp
        Common.cpp
        media-avi.cpp
        media-render.cpp)

add_library(
        avi-lib
        STATIC
        ${ANDROID_NDK}/sources/transcode-1.1.7/avilib/avilib.c
        ${ANDROID_NDK}/sources/transcode-1.1.7/avilib/wavlib.c
        ${ANDROID_NDK}/sources/transcode-1.1.7/avilib/platform_posix.c)

find_library(
        log-lib
        log)

find_library(
        bitmap-lib
        jnigraphics)

find_library(
        opengl-lib
        GLESv1_CM)

find_library(
        android-lib
        android)

find_library(
        opensl-lib
        OpenSLES)

target_link_libraries(
        media-lib
        avi-lib
        ${log-lib}
        ${bitmap-lib}
        ${opengl-lib}
        ${android-lib}
        ${opensl-lib})