file(GLOB_RECURSE GEO_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx")
file(GLOB_RECURSE GEO_HDR CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

if(GEO_SRC)
	add_library(cadutils_geometry STATIC ${GEO_SRC} ${GEO_HDR})
endif()


target_include_directories(cadutils_geometry PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
)

target_compile_features(cadutils_geometry PUBLIC cxx_std_17)

# 如果 Geometry 将来用 OCC，就在这里 link OCC（示例，先注释）
# target_link_libraries(cadutils_geometry PUBLIC OpenCASCADE::TKBRep ...)

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${GEO_SRC} ${GEO_HDR})

