cmake_minimum_required(VERSION 3.20)

project(CADProjectUtils
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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


# 一个可执行程序 demo
add_executable(cadutils_demo
    src/main.cpp
	src/include/cadutils/distance.h
)

# 按真实目录结构在 VS 中分组
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    FILES
        src/main.cpp
        src/include/cadutils/distance.h
)

target_include_directories(cadutils_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include/cadutils
)

target_link_libraries(cadutils_demo PUBLIC nlohmann_json)
#target_link_libraries(cadutils_demo PRIVATE doctest)

# 常用的 warning（MSVC / GCC / Clang）
if (MSVC)
    target_compile_options(cadutils_demo PRIVATE /W4 /permissive-)
else()
    target_compile_options(cadutils_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 设置 Visual Studio 默认启动项目
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT cadutils_demo
)
