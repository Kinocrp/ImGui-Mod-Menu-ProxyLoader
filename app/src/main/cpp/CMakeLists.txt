cmake_minimum_required(VERSION 3.22.1)

project("proxy-loader")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        xdl/include
        imgui
        imgui/backends
        menu
        il2cpp
)

aux_source_directory(xdl xdl-src)
aux_source_directory(imgui imgui-src)
aux_source_directory(imgui/backends imgui-backends-src)
aux_source_directory(menu menu-src)
aux_source_directory(il2cpp il2cpp-src)

add_library(${CMAKE_PROJECT_NAME} SHARED
        main.cpp
        ${xdl-src}
        ${imgui-src}
        ${imgui-backends-src}
        ${menu-src}
        ${il2cpp-src})

find_library(egl EGL)
find_library(android android)
find_library(LZ_LIB z)

if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    find_library(OPENGL_LIB GLESv2)
else ()
    find_library(OPENGL_LIB GLESv3)
endif ()

add_subdirectory(dobby)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
        android
        log
        dobby_static
        ${OPENGL_LIB}
        ${egl}
        ${LZ_LIB})

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            COMMAND ${CMAKE_STRIP} --remove-section=.comment "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            COMMENT "Stripping symbols and comments from $<TARGET_FILE_NAME:${CMAKE_PROJECT_NAME}>"
            VERBATIM
    )
endif()