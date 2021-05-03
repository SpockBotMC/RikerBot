set_property(SOURCE swig/Logger.i PROPERTY CPLUSPLUS ON)
swig_add_library(Logger
  LANGUAGE python
  OUTPUT_DIR ${RKR_PACKAGE_ROOT}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES swig/Logger.i src/logger.cpp
)
set_target_properties(Logger PROPERTIES OUTPUT_NAME CLogger
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT})
target_compile_features(Logger PRIVATE cxx_std_17)
target_compile_options(Logger PRIVATE ${OPTIONS})
target_include_directories(Logger PRIVATE ${INCLUDES})
target_link_libraries(Logger ${Boost_LOG_LIBRARY})
list(APPEND RIKER_DEPENDS Logger)
