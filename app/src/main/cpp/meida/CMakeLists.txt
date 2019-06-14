cmake_minimum_required(VERSION 3.4.1)

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
        ${ANDROID_NDK}/sources/transcode-1.1.7/avilib/platform_posix.c)

find_library(
        log-lib
        log)

find_library(
        bitmap-lib
        jnigraphics)

target_link_libraries(
        media-lib
        avi-lib
        ${log-lib}
        ${bitmap-lib})