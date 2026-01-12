cmake_minimum_required(VERSION 3.20)

# --------- helpers ----------
function(tp_copy_glob_post_build target src_dir pattern dst_dir)
  if (EXISTS "${src_dir}")
    file(GLOB _files "${src_dir}/${pattern}")
    if (_files)
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_files} "${dst_dir}"
        COMMENT "Copy ${pattern}: ${src_dir} -> ${dst_dir}"
      )
    endif()
  endif()
endfunction()

function(tp_copy_dir_post_build target src_dir dst_dir)
  if (EXISTS "${src_dir}")
    add_custom_command(TARGET ${target} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_dir}" "${dst_dir}"
      COMMENT "Copy dir: ${src_dir} -> ${dst_dir}"
    )
  endif()
endfunction()

# --------- OCCT (third_party/occ) ----------
function(tp_add_occt)
  if (TARGET thirdparty::occt)
    return()
  endif()

  set(OCCT_ROOT "${CMAKE_SOURCE_DIR}/third_party/occ" CACHE PATH "OCCT root")
  set(OCCT_INC  "${OCCT_ROOT}/include")
  set(OCCT_LIB  "${OCCT_ROOT}/lib")
  set(OCCT_BIN  "${OCCT_ROOT}/bin")
  set(OCCT_DATA "${OCCT_ROOT}/data")

  if (NOT EXISTS "${OCCT_INC}" OR NOT EXISTS "${OCCT_LIB}")
    message(FATAL_ERROR "OCCT not found. Expect: ${OCCT_ROOT}/{include,lib}")
  endif()

  add_library(thirdparty_occt INTERFACE)
  add_library(thirdparty::occt ALIAS thirdparty_occt)

  target_include_directories(thirdparty_occt INTERFACE "${OCCT_INC}")
  target_link_directories(thirdparty_occt INTERFACE "${OCCT_LIB}")

  # 够你跑通：造型+三角化+STEP/IGES
  target_link_libraries(thirdparty_occt INTERFACE
    TKernel
    TKMath
    TKGeomBase
    TKG3d
    TKGeomAlgo
    TKTopAlgo
    TKPrim
    TKBO
    TKBool
    TKFillet
    TKShHealing
    TKMesh
    TKService
	TKBRep
  )

  set(OCCT_BIN_DIR  "${OCCT_BIN}"  PARENT_SCOPE)
  set(OCCT_DATA_DIR "${OCCT_DATA}" PARENT_SCOPE)
endfunction()

function(tp_occt_runtime target)
  set(OCCT_ROOT "${CMAKE_SOURCE_DIR}/third_party/occ")
  tp_copy_glob_post_build(${target} "${OCCT_ROOT}/bin" "*.dll" "$<TARGET_FILE_DIR:${target}>")
  tp_copy_dir_post_build(${target} "${OCCT_ROOT}/data" "$<TARGET_FILE_DIR:${target}>/data")
endfunction()

# --------- OSG (third_party/osg-3.6.5) ----------
function(tp_add_osg)
  if (TARGET thirdparty::osg)
    return()
  endif()

  set(OSG_ROOT "${CMAKE_SOURCE_DIR}/third_party/osg-3.6.5" CACHE PATH "OSG root")
  set(OSG_INC  "${OSG_ROOT}/include")
  set(OSG_LIB  "${OSG_ROOT}/lib")
  set(OSG_BIN  "${OSG_ROOT}/bin")
  set(OSG_DATA "${OSG_ROOT}/data")

  if (NOT EXISTS "${OSG_INC}" OR NOT EXISTS "${OSG_LIB}")
    message(FATAL_ERROR "OSG not found. Expect: ${OSG_ROOT}/{include,lib}")
  endif()

  add_library(thirdparty_osg INTERFACE)
  add_library(thirdparty::osg ALIAS thirdparty_osg)

  target_include_directories(thirdparty_osg INTERFACE "${OSG_INC}")
  target_link_directories(thirdparty_osg INTERFACE "${OSG_LIB}")

  # 你跑 viewer 必备的一组
  set(_osg_libs OpenThreads osg osgDB osgUtil osgGA osgViewer osgText)

  # 自动适配 Debug 后缀 d（如果存在），否则 Debug 也用同名库
  foreach(L IN LISTS _osg_libs)
    if (EXISTS "${OSG_LIB}/${L}d.lib")
      target_link_libraries(thirdparty_osg INTERFACE
        $<$<CONFIG:Debug>:${L}d>
        $<$<NOT:$<CONFIG:Debug>>:${L}>
      )
    else()
      target_link_libraries(thirdparty_osg INTERFACE ${L})
    endif()
  endforeach()

  set(OSG_BIN_DIR  "${OSG_BIN}"  PARENT_SCOPE)
  set(OSG_DATA_DIR "${OSG_DATA}" PARENT_SCOPE)
endfunction()

function(tp_osg_runtime target)
  set(OSG_ROOT "${CMAKE_SOURCE_DIR}/third_party/osg-3.6.5")
  # bin 里包含 osgPlugins-*.dll 与依赖 dll：直接全拷
  tp_copy_glob_post_build(${target} "${OSG_ROOT}/bin" "*.dll" "$<TARGET_FILE_DIR:${target}>")

  # OSG data 可选：你如果不用 osgDB 的默认数据路径，可以不拷
  if (EXISTS "${OSG_ROOT}/data")
    tp_copy_dir_post_build(${target} "${OSG_ROOT}/data" "$<TARGET_FILE_DIR:${target}>/osg_data")
  endif()
endfunction()