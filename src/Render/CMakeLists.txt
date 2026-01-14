file(GLOB_RECURSE RENDER_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE RENDER_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")


add_library(cadutils_render STATIC ${RENDER_SRC} ${RENDER_HDR})

target_include_directories(cadutils_render PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_compile_features(cadutils_render PUBLIC cxx_std_17)

target_link_libraries(cadutils_render PRIVATE
    cadutils_geometry
)
source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${RENDER_SRC} ${RENDER_HDR})
