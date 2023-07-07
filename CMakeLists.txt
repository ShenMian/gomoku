# Copyright 2023 ShenMian
# License(Apache-2.0)

cmake_minimum_required(VERSION 3.12)

project(gomoku CXX)

set(BUILD_SHARED_LIBS FALSE)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/gomoku.hpp src/board.hpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

find_package(SFML CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-graphics sfml-network)
target_compile_definitions(${PROJECT_NAME} PRIVATE SFML_STATIC)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}> COMMAND_EXPAND_LISTS)
endif()

install(TARGETS ${PROJECT_NAME})
