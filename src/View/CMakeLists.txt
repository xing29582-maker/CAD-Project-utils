# Qt
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

file(GLOB_RECURSE VIEW_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE VIEW_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

if(VIEW_SRC)
	add_library(cadutils_view STATIC ${VIEW_SRC} ${VIEW_HDR})
endif()

target_include_directories(cadutils_view PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
)

target_compile_features(cadutils_view PUBLIC cxx_std_17)

target_include_directories(cadutils_view PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(cadutils_view PRIVATE ${OPENSCENEGRAPH_LIBRARIES})
target_link_libraries(cadutils_view PUBLIC cadutils_platform)
target_link_libraries(cadutils_view PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${VIEW_SRC} ${VIEW_HDR})
