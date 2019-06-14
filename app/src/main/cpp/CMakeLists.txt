# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# so动态库最后输出的路径 前面一个是命令，后面是路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

#设置头文件搜索路径（和此txt同个路径的头文件无需设置），可选
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)

#指定用到的系统库或者NDK库或者第三方库的搜索路径，可选。
#LINK_DIRECTORIES(/usr/local/lib)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library. 生成的so名称，在java代码中加载so库需要与此一致
        native-lib

        # Sets the library as a shared library. 将library设置为一个共享库
        SHARED

        # Provides a relative path to your source file(s).
        # 为源文件提供一个相对路径,可以使用通配符，也可以一个一个文件的引入
        native-lib.cpp
        native-thread.cpp
        Unix_wrap.cxx)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
# 使用本地系统库，在CMake默认的搜索路径就已经包含了系统库，所以直接指定名称就行，CMake会验证库是否存在。

find_library( # Sets the name of the path variable. 设置路径变量的名称,后面会引用到
        log-lib # log日志,默认都需要添加

        # Specifies the name of the NDK library that
        # you want CMake to locate. 指定NDK库的名称
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
# 链接所有的库，比如上面我们添加的第三方库，添加的本地log库

target_link_libraries( # Specifies the target library.  指定目标library
        # 通过add_library添加的直接写设置的名字，
        # 一种是SHARED,一般来说就是我们需要生成的so文件
        # 二中是STATIC IMPORTED 添加的第三方静态库
        native-lib

        # Links the target library to the log library 将目标library链接到日志库
        # included in the NDK.
        # 链接本地NDK里面的库
        ${log-lib})

#添加子目录,将会调用子目录中的CMakeLists.txt, 生成多个 so库
ADD_SUBDIRECTORY(echo)
ADD_SUBDIRECTORY(meida)