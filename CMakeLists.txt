cmake_minimum_required(VERSION 3.19)
project(RikerBot CXX)

set(MC_VERSION "1.16.5" CACHE STRING "Minecraft version to target")
STRING(REGEX REPLACE [\.] _ MC_USCORE ${MC_VERSION})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
find_package(Botan2 REQUIRED)

find_package(ZLIB REQUIRED)
find_package(Boost COMPONENTS log REQUIRED)
find_package(Python COMPONENTS Interpreter Development Development.Module REQUIRED)

find_package(SWIG 4.0 REQUIRED)
include(UseSWIG)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(INCLUDES ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${Python_INCLUDE_DIRS} ${BOTAN2_INCLUDE_DIRS} include)
set(OPTIONS -Wall -Wextra -Wpedantic)

set(RIKER_DEPENDS "")


set(RKR_PACKAGE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/rikerbot)
set(RKR_PLUGIN_DIR ${RKR_PACKAGE_ROOT}/plugins)
set(RKR_PROTO_DIR ${RKR_PACKAGE_ROOT}/proto)

set(PROTO_INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/Proto${MC_USCORE}.i)
set(PROTO_IMPL ${CMAKE_CURRENT_BINARY_DIR}/proto_${MC_USCORE}.cpp)

set(PROTO_GEN_SOURCES ${PROTO_IMPL}
    ${CMAKE_CURRENT_BINARY_DIR}/proto_${MC_USCORE}.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/particletypes.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/minecraft_protocol.hpp
    ${RKR_PROTO_DIR}/MinecraftProtocol.py)
set(PROTO_GEN_FILES ${PROTO_GEN_SOURCES} ${PROTO_INTERFACE})

set(BLOCK_IMPL ${CMAKE_CURRENT_BINARY_DIR}/block_data.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/shape_data.cpp)
set(BLOCK_GEN_FILES ${BLOCK_IMPL}
    ${CMAKE_CURRENT_BINARY_DIR}/block_data.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/shape_data.hpp)

set(MCD2CPP_GEN ${PROTO_GEN_FILES} ${BLOCK_GEN_FILES})

add_custom_command(
  OUTPUT ${MCD2CPP_GEN}
  COMMAND ${Python_EXECUTABLE}
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/generate.py ${MC_VERSION}
  COMMAND mv
  ARGS MinecraftProtocol.py ${RKR_PROTO_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS generate.py
  COMMENT "Generating mcd2cpp files"
  VERBATIM
)
add_custom_target(proto_gen DEPENDS ${PROTO_GEN_FILES})
add_custom_target(block_gen DEPENDS ${BLOCK_GEN_FILES})


add_custom_command(
  OUTPUT swigpyrun.hpp
  COMMAND ${SWIG_EXECUTABLE}
  ARGS -c++ -python -py3 -external-runtime swigpyrun.hpp
  COMMENT "Generating SWIG runtime header"
  VERBATIM
)
add_custom_target(swig_runtime DEPENDS swigpyrun.hpp)


include(cmake/Proto.cmake)

include(cmake/PluginLoader.cmake)

include(cmake/Event.cmake)

include(cmake/Exec.cmake)

include(cmake/IO.cmake)

include(cmake/Logger.cmake)

include(cmake/World.cmake)

include(cmake/Status.cmake)

include(cmake/Timer.cmake)

add_custom_target(rikerbot_all DEPENDS ${RIKER_DEPENDS})
