add_subdirectory(Common)
add_subdirectory(Application)
add_subdirectory(Data)
add_subdirectory(Platform)
add_subdirectory(Geometry)
add_subdirectory(Render)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(TARGET_NAME cadutils_demo)

add_executable(${TARGET_NAME} WIN32
  main.cpp
)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(THIRDPARTY ${ROOT_DIR}/third_party)

#qt
set(THIRDPARTY_QT ${THIRDPARTY}/qt-5.14.0/5.14.0/msvc2017_64)
set(THIRDPARTY_QT_INC ${THIRDPARTY_QT}/include)
set(THIRDPARTY_QT_INC_GUI_PRIVATE ${THIRDPARTY_QT}/include/QtGui/5.14.0)
set(THIRDPARTY_QT_INC_CORE_PRIVATE ${THIRDPARTY_QT}/include/QtCore/5.14.0)
set(THIRDPARTY_QT_LIB ${THIRDPARTY_QT}/lib)
set(THIRDPARTY_QT_BIN ${THIRDPARTY_QT}/bin)

get_target_property(_qt_qmake_location Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qt_qmake_location}" DIRECTORY)

# add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    # COMMAND "${_qt_bin_dir}/windeployqt.exe"
            # "$<TARGET_FILE:${TARGET_NAME}>"
# )


# 你的公共 include
target_include_directories(${TARGET_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/Application/UI
)

# 链接你自己的模块库
target_link_libraries(${TARGET_NAME}
    PRIVATE
	cadutils_application
)

function(copy_qt_dlls target)
  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${THIRDPARTY_QT_BIN}/$<IF:$<CONFIG:Debug>,Qt5Cored.dll,Qt5Core.dll>"
      "$<TARGET_FILE_DIR:${target}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${THIRDPARTY_QT_BIN}/$<IF:$<CONFIG:Debug>,Qt5Guid.dll,Qt5Gui.dll>"
      "$<TARGET_FILE_DIR:${target}>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${THIRDPARTY_QT_BIN}/$<IF:$<CONFIG:Debug>,Qt5Widgetsd.dll,Qt5Widgets.dll>"
      "$<TARGET_FILE_DIR:${target}>"

	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target}>/platforms"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  "${THIRDPARTY_QT}/plugins/platforms/$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>"
	  "$<TARGET_FILE_DIR:${target}>/platforms"
  )
endfunction()

copy_qt_dlls(${TARGET_NAME})

# Qt（如果开启）
if(USE_QT)
  target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
endif()

# VS 里默认启动项（可选）
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})

# source_group：让 VS 的文件树按目录显示（可选）
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES main.cpp)