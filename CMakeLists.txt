# cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake . -B cmake-build
# cmake --build cmake-build --target help
# cmake --build cmake-build --config Release --target HKGraphApp
# cmake --build cmake-build --config Release --target HKGraphMidi_VST3


cmake_minimum_required(VERSION 3.15)

project(HKPlugins VERSION 0.0.1)

# on macOS Sonoma and LLVM Clang 17.0.6, CMake invokes `ar` and `ranlib` under `/usr/bin` which causes corrupt libs.
# force the use of `llvm-ar` and `llvm-ranlib` if we are using LLVM
# export LLVM=$(brew --prefix llvm)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("LLVM : $ENV{LLVM}")
    set(CMAKE_AR "$ENV{LLVM}/bin/llvm-ar" CACHE FILEPATH "LLVM ar" FORCE)
    set(CMAKE_RANLIB "$ENV{LLVM}/bin/llvm-ranlib" CACHE FILEPATH "LLVM ranlib" FORCE)
    message("AR : ${CMAKE_AR}")
    message("RANLIB : ${CMAKE_RANLIB}")
endif ()

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

# uncomment the following line if you have checked out JUCE locally in the current directory
#add_subdirectory(JUCE)

# use the following if you've installed JUCE globally or using a package manager, `vcpkg` for example
find_package(JUCE CONFIG REQUIRED)
add_subdirectory(HKGraph)
