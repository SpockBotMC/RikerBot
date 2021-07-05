add_library(TimerLib SHARED src/timer_core.cpp)
set_target_properties(TimerLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libTimer
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(TimerLib PRIVATE cxx_std_20)
target_compile_options(TimerLib PRIVATE ${OPTIONS})
target_include_directories(TimerLib PRIVATE ${INCLUDES})
target_link_libraries(TimerLib PRIVATE PluginLoaderLib)


set_property(SOURCE swig/TimerCore.i PROPERTY CPLUSPLUS ON)
swig_add_library(TimerCore
  LANGUAGE python
  OUTPUT_DIR ${RKR_PLUGIN_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/TimerCore.i
)
set_target_properties(TimerCore PROPERTIES OUTPUT_NAME CTimerCore
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PLUGIN_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(TimerCore PRIVATE cxx_std_20)
target_compile_options(TimerCore PRIVATE ${OPTIONS})
target_include_directories(TimerCore PRIVATE ${INCLUDES})
target_link_libraries(TimerCore PRIVATE TimerLib Python::Module)
list(APPEND RIKER_DEPENDS TimerCore)
