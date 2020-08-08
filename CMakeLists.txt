cmake_minimum_required(VERSION 3.18)
project(RikerBot CXX)

set(MC_VERSION "1.16.1" CACHE STRING "Minecraft version to target")
STRING(REGEX REPLACE [\.] _ MC_USCORE ${MC_VERSION})

find_path(NET net.hpp PATH_SUFFIXES boost/asio/ts REQUIRED)
find_path(BOTAN botan/botan.h PATH_SUFFIXES botan-2 REQUIRED)
find_library(LIBBOTAN botan-2 REQUIRED)

find_package(Boost COMPONENTS log REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

find_package(SWIG 4.0 REQUIRED)
include(UseSWIG)


set(INCLUDES ${CMAKE_CURRENT_BINARY_DIR} ${Python3_INCLUDE_DIRS} ${NET}
    ${BOTAN} include)

set(PROTO_INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/proto_${MC_USCORE}.i)
set(PROTO_IMPL ${CMAKE_CURRENT_BINARY_DIR}/proto_${MC_USCORE}.cpp)
set(PROTO_GEN_FILES ${PROTO_INTERFACE} ${PROTO_IMPL}
    ${CMAKE_CURRENT_BINARY_DIR}/proto_${MC_USCORE}.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/particletypes.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/minecraft_protocol.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins/MinecraftProtocol.py)


add_custom_command(
  OUTPUT ${PROTO_GEN_FILES}
  COMMAND ${Python3_EXECUTABLE}
  ARGS ${CMAKE_CURRENT_SOURCE_DIR}/generate.py ${MC_VERSION}
  COMMAND mv
  ARGS MinecraftProtocol.py ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating protocol files"
  VERBATIM
)
add_custom_target(proto_gen DEPENDS ${PROTO_GEN_FILES})


add_custom_command(
  OUTPUT swigpyrun.hpp
  COMMAND ${SWIG_EXECUTABLE}
  ARGS -c++ -python -py3 -external-runtime swigpyrun.hpp
  COMMENT "Generating SWIG runtime header"
  VERBATIM
)
add_custom_target(swig_runtime DEPENDS swigpyrun.hpp)


# Protocol
set_property(SOURCE ${PROTO_INTERFACE} PROPERTY CPLUSPLUS ON)
swig_add_library(Proto
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${PROTO_INTERFACE} ${PROTO_IMPL} src/datautils.cpp
)
set_target_properties(Proto PROPERTIES OUTPUT_NAME Proto${MC_USCORE}
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins)
target_compile_features(Proto PRIVATE cxx_std_20)
target_include_directories(Proto PRIVATE ${INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/swig)
add_dependencies(Proto proto_gen)


# Plugin Loader
set_property(SOURCE swig/PluginLoader.i PROPERTY CPLUSPLUS ON)
swig_add_library(PluginLoader
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/PluginLoader.i src/plugin_loader.cpp
)
set_target_properties(PluginLoader PROPERTIES OUTPUT_NAME CPluginLoader
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins)
target_compile_features(PluginLoader PRIVATE cxx_std_20)
target_include_directories(PluginLoader PRIVATE ${INCLUDES})
add_dependencies(PluginLoader swig_runtime)


# Event Core
set_property(SOURCE swig/EventCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(EventCore
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/EventCore.i src/event_core.cpp src/plugin_loader.cpp
      src/plugin_base.cpp
)
set_target_properties(EventCore PROPERTIES OUTPUT_NAME CEventCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins)
target_compile_features(EventCore PRIVATE cxx_std_20)
target_include_directories(EventCore PRIVATE ${INCLUDES})
add_dependencies(EventCore swig_runtime)


# IO Core
set_property(SOURCE swig/IOCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(IOCore
  LANGUAGE python
  OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/IOCore.i src/io_core.cpp src/plugin_loader.cpp
      src/plugin_base.cpp src/event_core.cpp src/datautils.cpp ${PROTO_IMPL}
)
set_target_properties(IOCore PROPERTIES OUTPUT_NAME CIOCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/riker/plugins)
target_compile_features(IOCore PRIVATE cxx_std_20)
target_include_directories(IOCore PRIVATE ${INCLUDES})
target_compile_options(IOCore PRIVATE -DBOOST_ASIO_DISABLE_CONCEPTS
    -DBOOST_LOG_DYN_LINK)
target_link_libraries(IOCore ${Boost_LOG_LIBRARY} ${LIBBOTAN})
add_dependencies(IOCore proto_gen swig_runtime)
