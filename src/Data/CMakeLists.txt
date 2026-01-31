file(GLOB_RECURSE DATA_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE DATA_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")


add_library(cadutils_data SHARED ${DATA_SRC} ${DATA_HDR})

target_include_directories(cadutils_data PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
  ${CMAKE_SOURCE_DIR}/src/Geometry/Public
  ${CMAKE_SOURCE_DIR}/src/Common/Public
)

target_link_libraries(cadutils_data PRIVATE
    cadutils_geometry
	cadutils_common
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${DATA_SRC} ${DATA_HDR})
