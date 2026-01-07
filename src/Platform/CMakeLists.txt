file(GLOB_RECURSE PFM_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE PFM_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

if(PFM_SRC)
	add_library(cadutils_platform STATIC ${PFM_SRC} ${PFM_HDR})
endif()

target_include_directories(cadutils_platform PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
)

target_compile_features(cadutils_platform PUBLIC cxx_std_17)
source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${PFM_SRC} ${PFM_HDR})





