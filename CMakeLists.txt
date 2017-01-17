cmake_minimum_required(VERSION 2.8.11)

project(haselnuss)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set qt specific options
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# set c++ 14
set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(OpenCV REQUIRED)
find_package(fann REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

# Source Files
FILE(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/*.cpp")
# Header files
FILE(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/*.h")

set(INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}")
set(INCLUDE_DIRS ${INCLUDE_DIRS} "${FANN_INCLUDE_DIR}")

INCLUDE_DIRECTORIES("${INCLUDE_DIRS}")

# Libraries
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${OpenCV_LIBS})
set(LIBRARIES ${LIBRARIES} ${FANN_LIBRARIES})
set(LIBRARIES ${LIBRARIES} ${JSONCPP_LIBRARIES})

add_executable(haselnuss ${SOURCE_FILES} ${HEADER_FILES})
qt5_use_modules(haselnuss Widgets)
target_link_libraries(haselnuss ${LIBRARIES})
