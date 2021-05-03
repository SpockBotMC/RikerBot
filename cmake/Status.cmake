set_property(SOURCE swig/StatusCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(StatusCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/StatusCore.i src/status_core.cpp
)
set_target_properties(StatusCore PROPERTIES OUTPUT_NAME CStatusCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(StatusCore PRIVATE cxx_std_20)
target_compile_options(StatusCore PRIVATE ${OPTIONS})
target_include_directories(StatusCore PRIVATE ${INCLUDES})
target_link_libraries(StatusCore PluginLoaderLib IOLib EventLib)
list(APPEND RIKER_DEPENDS StatusCore)
