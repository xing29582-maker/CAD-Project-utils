cmake_minimum_required(VERSION 3.20)

project(cadutils_demo
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 可选：统一输出目录（方便找 exe/dll）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#---- Libraries (FetchContent) ----
# include(FetchContent)

# FetchContent_Declare(
  # nlohmann_json
  # GIT_REPOSITORY https://github.com/nlohmann/json.git
  # GIT_TAG v3.11.3
# )
# FetchContent_MakeAvailable(nlohmann_json)

# FetchContent_Declare(
  # doctest
  # GIT_REPOSITORY https://github.com/doctest/doctest.git
  # GIT_TAG v2.4.11
# )
# FetchContent_MakeAvailable(doctest)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann_json/include
)

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest
)

#find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgUtil osgViewer OpenThreads)
#target_include_directories(cadutils_demo PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
#target_link_libraries(cadutils_demo PRIVATE ${OPENSCENEGRAPH_LIBRARIES})

#find_package(OpenCASCADE REQUIRED)
#target_link_libraries(cadutils_demo PRIVATE ${OpenCASCADE_LIBRARIES})
#target_include_directories(cadutils_demo PRIVATE ${OpenCASCADE_INCLUDE_DIR})

# Qt 根目录（工程内）
set(QT_ROOT
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/qt-5.14.0/5.14.0/msvc2017_64
)

list(APPEND CMAKE_PREFIX_PATH ${QT_ROOT})

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)


#target_link_libraries(cadutils_demo PUBLIC nlohmann_json)
#target_link_libraries(cadutils_demo PRIVATE doctest)

# MSVC 常用设置（可选）
if(MSVC)
  add_compile_options(/W4 /permissive-)
endif()

	
# 让 VS 按文件夹显示
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 让我们能 include(cmake/ThirdParty.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)
