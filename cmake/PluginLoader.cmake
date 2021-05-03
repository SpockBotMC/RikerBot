# Library
add_library(PluginLoaderLib SHARED src/plugin_loader.cpp src/plugin_base.cpp)
set_target_properties(PluginLoaderLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libPluginLoader
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(PluginLoaderLib PRIVATE cxx_std_20)
target_compile_options(PluginLoaderLib PRIVATE ${OPTIONS})
target_include_directories(PluginLoaderLib PRIVATE ${INCLUDES})
add_dependencies(PluginLoaderLib swig_runtime)

# Module
set_property(SOURCE swig/PluginLoader.i PROPERTY CPLUSPLUS ON)
swig_add_library(PluginLoader
  LANGUAGE python
  OUTPUT_DIR ${RKR_PACKAGE_ROOT}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/PluginLoader.i
)
set_target_properties(PluginLoader PROPERTIES OUTPUT_NAME CPluginLoader
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}
    INSTALL_RPATH $ORIGIN/lib)
target_compile_features(PluginLoader PRIVATE cxx_std_20)
target_compile_options(PluginLoader PRIVATE ${OPTIONS})
target_include_directories(PluginLoader PRIVATE ${INCLUDES})
target_link_libraries(PluginLoader PRIVATE PluginLoaderLib)
list(APPEND RIKER_DEPENDS PluginLoader)
