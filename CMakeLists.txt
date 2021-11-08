cmake_minimum_required(VERSION 3.1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_STANDARD 14)

project(QSkinny)
#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets Quick REQUIRED)

set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_FEATURES "")

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS} ${Qt5Core_PRIVATE_INCLUDE_DIRS}  ${Qt5Quick_PRIVATE_INCLUDE_DIRS})

add_subdirectory(src)
