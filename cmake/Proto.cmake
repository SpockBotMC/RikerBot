# Library
add_library(ProtoLib SHARED ${PROTO_GEN_SOURCES} src/datautils.cpp)
set_target_properties(ProtoLib PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_NAME libProto
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PACKAGE_ROOT}/lib
    INSTALL_RPATH $ORIGIN)
target_compile_features(ProtoLib PRIVATE cxx_std_20)
target_compile_options(ProtoLib PRIVATE ${OPTIONS})
target_include_directories(ProtoLib PRIVATE ${INCLUDES})
add_dependencies(ProtoLib proto_gen)


# Module
set_property(SOURCE ${PROTO_INTERFACE} PROPERTY CPLUSPLUS ON)
swig_add_library(Proto
  LANGUAGE python
  OUTPUT_DIR ${RKR_PROTO_DIR}
  OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}
  SOURCES ${PROTO_INTERFACE}
)
set_target_properties(Proto PROPERTIES OUTPUT_NAME Proto${MC_USCORE}
    SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE SWIG_COMPILE_OPTIONS -py3
    LIBRARY_OUTPUT_DIRECTORY ${RKR_PROTO_DIR}
    INSTALL_RPATH $ORIGIN/../lib)
target_compile_features(Proto PRIVATE cxx_std_20)
target_compile_options(Proto PRIVATE ${OPTIONS})
target_include_directories(Proto PRIVATE ${INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/swig)
target_link_libraries(Proto PRIVATE ProtoLib)
list(APPEND RIKER_DEPENDS Proto)
