file(GLOB_RECURSE GEO_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE GEO_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

include(ThirdParty)
tp_add_occt()

add_library(cadutils_geometry SHARED ${GEO_SRC} ${GEO_HDR})


target_include_directories(cadutils_geometry PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
  ${CMAKE_SOURCE_DIR}/src/Render/Public
  ${CMAKE_SOURCE_DIR}/src/Common/Public
)

target_compile_features(cadutils_geometry PUBLIC cxx_std_17)

target_link_libraries(cadutils_geometry PUBLIC
    cadutils_common
	cadutils_render
)

target_link_libraries(cadutils_geometry PRIVATE
  thirdparty::occt
)

tp_occt_runtime(cadutils_geometry)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${GEO_SRC} ${GEO_HDR})

