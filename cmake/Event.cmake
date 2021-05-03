# Library
add_library(EventLib SHARED src/event_core.cpp)
set_target_properties(EventLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libEvent
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(EventLib PRIVATE cxx_std_20)
target_compile_options(EventLib PRIVATE ${OPTIONS})
target_include_directories(EventLib PRIVATE ${INCLUDES})
target_link_libraries(EventLib PUBLIC PluginLoaderLib)
add_dependencies(EventLib swig_runtime)

# Module
set_property(SOURCE swig/EventCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(EventCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/EventCore.i
)
set_target_properties(EventCore PROPERTIES OUTPUT_NAME CEventCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(EventCore PRIVATE cxx_std_20)
target_compile_options(EventCore PRIVATE ${OPTIONS})
target_include_directories(EventCore PRIVATE ${INCLUDES})
target_link_libraries(EventCore PRIVATE EventLib)
add_dependencies(EventCore swig_runtime)
list(APPEND RIKER_DEPENDS EventCore)
