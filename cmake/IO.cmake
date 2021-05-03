# Library
add_library(IOLib SHARED src/io_core.cpp)
set_target_properties(IOLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libIO
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(IOLib PRIVATE cxx_std_20)
target_compile_options(IOLib PRIVATE ${OPTIONS})
target_include_directories(IOLib PRIVATE ${INCLUDES})
target_link_libraries(IOLib PUBLIC ${Boost_LOG_LIBRARY} ${LIBBOTAN} ProtoLib
    PluginLoaderLib EventLib)

# Module
set_property(SOURCE swig/IOCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(IOCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/IOCore.i)
set_target_properties(IOCore PROPERTIES OUTPUT_NAME CIOCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(IOCore PRIVATE cxx_std_20)
target_compile_options(IOCore PRIVATE ${OPTIONS})
target_include_directories(IOCore PRIVATE ${INCLUDES})
target_link_libraries(IOCore PRIVATE IOLib)
list(APPEND RIKER_DEPENDS IOCore)
