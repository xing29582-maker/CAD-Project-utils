file(GLOB_RECURSE DATA_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE DATA_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

include(ThirdParty)
tp_add_occt()
tp_add_osg()

add_library(cadutils_data SHARED ${DATA_SRC} ${DATA_HDR})

target_include_directories(cadutils_data PUBLIC
  ${CMAKE_SOURCE_DIR}/src/Geometry/Public
)

target_link_libraries(cadutils_data PRIVATE
  thirdparty::occt
  thirdparty::osg
)

target_link_libraries(cadutils_data PRIVATE
    cadutils_geometry
)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${DATA_SRC} ${DATA_HDR})
