file(GLOB_RECURSE COMMON_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE COMMON_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")


add_library(cadutils_common SHARED ${COMMON_SRC} ${COMMON_HDR})

target_include_directories(cadutils_common PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_compile_features(cadutils_common PUBLIC cxx_std_17)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${COMMON_SRC} ${COMMON_HDR})
