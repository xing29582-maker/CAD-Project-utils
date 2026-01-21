set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE RENDER_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE RENDER_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

include(ThirdParty)
tp_add_osg()


add_library(cadutils_render SHARED ${RENDER_SRC} ${RENDER_HDR})

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

target_include_directories(cadutils_render PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_compile_features(cadutils_render PUBLIC cxx_std_17)

target_link_libraries(cadutils_render PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(cadutils_render PRIVATE
    cadutils_common
)

target_link_libraries(cadutils_render PRIVATE
  thirdparty::osg
)

tp_osg_runtime(cadutils_render)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${RENDER_SRC} ${RENDER_HDR})
