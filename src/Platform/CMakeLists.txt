file(GLOB_RECURSE PFM_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE PFM_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_library(cadutils_platform SHARED ${PFM_SRC} ${PFM_HDR})


target_include_directories(cadutils_platform PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
  ${CMAKE_SOURCE_DIR}/src/Common/Public
  ${CMAKE_SOURCE_DIR}/src/Data/Public
)

target_compile_features(cadutils_platform PUBLIC cxx_std_17)

target_link_libraries(cadutils_platform PRIVATE
    cadutils_data
)


source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${PFM_SRC} ${PFM_HDR})





