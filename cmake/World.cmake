# Library
add_library(WorldLib SHARED src/world_core.cpp src/smpmap.cpp)
set_target_properties(WorldLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libWorld
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(WorldLib PRIVATE cxx_std_20)
target_compile_options(WorldLib PRIVATE ${OPTIONS})
target_include_directories(WorldLib PRIVATE ${INCLUDES})
target_link_libraries(WorldLib PUBLIC ProtoLib PluginLoaderLib EventLib)

set_property(SOURCE swig/WorldCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(WorldCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/WorldCore.i
)
set_target_properties(WorldCore PROPERTIES OUTPUT_NAME CWorldCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(WorldCore PRIVATE cxx_std_20)
target_compile_options(WorldCore PRIVATE ${OPTIONS})
target_include_directories(WorldCore PRIVATE ${INCLUDES})
target_link_libraries(WorldCore PRIVATE WorldLib)
add_dependencies(WorldCore swig_runtime)
list(APPEND RIKER_DEPENDS WorldCore)
