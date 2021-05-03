# Library
add_library(ExecLib SHARED src/exec_core.cpp)
set_target_properties(ExecLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libExec
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(ExecLib PRIVATE cxx_std_20)
target_compile_options(ExecLib PRIVATE ${OPTIONS})
target_include_directories(ExecLib PRIVATE ${INCLUDES})
target_link_libraries(ExecLib PUBLIC ${Boost_LOG_LIBRARY} PluginLoaderLib
    EventLib)
add_dependencies(ExecLib swig_runtime)

# Module
set_property(SOURCE swig/ExecCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(ExecCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/ExecCore.i
)
set_target_properties(ExecCore PROPERTIES OUTPUT_NAME CExecCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(ExecCore PRIVATE cxx_std_20)
target_compile_options(ExecCore PRIVATE ${OPTIONS})
target_include_directories(ExecCore PRIVATE ${INCLUDES})
target_link_libraries(ExecCore PRIVATE ExecLib)
list(APPEND RIKER_DEPENDS ExecCore)
