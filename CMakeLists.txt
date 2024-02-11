# cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake . -B cmake-build
# cmake --build cmake-build --target help
# cmake --build cmake-build --config Release --target HKGraphApp
# cmake --build cmake-build --config Release --target HKGraphMidi_VST3


cmake_minimum_required(VERSION 3.15)

project(HKPlugins VERSION 0.0.1)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

# uncomment the following line if you have checked out JUCE locally in the current directory
#add_subdirectory(JUCE)

# use the following if you've installed JUCE globally or using a package manager, `vcpkg` for example
find_package(JUCE CONFIG REQUIRED)
add_subdirectory(HKGraph)
