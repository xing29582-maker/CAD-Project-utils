set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(ThirdParty)
tp_add_osg()

# 方案1：自动收集（省事）
file(GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)
file(GLOB_RECURSE APP_HDR CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

add_library(cadutils_application SHARED ${APP_SRC} ${APP_HDR})

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

target_compile_definitions(cadutils_application
    PRIVATE CADUTILS_BUILD_SHARED CADUTILS_APPLICATION_EXPORTS
)

target_include_directories(cadutils_application PUBLIC
  ${CMAKE_SOURCE_DIR}/src/Data
  ${CMAKE_SOURCE_DIR}/src/Common/Public
  ${CMAKE_SOURCE_DIR}/src/Geometry/Public
  ${CMAKE_SOURCE_DIR}/src/Render/Public
)

target_compile_features(cadutils_application PUBLIC cxx_std_17)

target_link_libraries(cadutils_application PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(cadutils_application PUBLIC
    cadutils_data
	cadutils_render
	cadutils_geometry
)
target_link_libraries(cadutils_application PRIVATE
  thirdparty::osg
)

# 运行必需：拷 dll + data
tp_osg_runtime(cadutils_application)

# VS 里按目录分组（可选）
source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${APP_SRC} ${APP_HDR})