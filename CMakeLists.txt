cmake_minimum_required(VERSION 3.10)
project(game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WIN32_EXECUTABLE FALSE)

# SFML 配置
set(SFML_DIR "${PROJECT_SOURCE_DIR}/SFML-2.6.2/lib/cmake/SFML")
find_package(SFML 2.6.2 COMPONENTS graphics window system REQUIRED)

# 源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
    "src/*.h"
)

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/SFML-2.6.2/include
)

# 链接 SFML
target_link_libraries(${PROJECT_NAME} 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# 复制 SFML DLL 文件到输出目录
if(WIN32)
    set(SFML_DLL_DIR "${PROJECT_SOURCE_DIR}/SFML-2.6.2/bin")
    file(GLOB SFML_DLLS "${SFML_DLL_DIR}/*.dll")
    foreach(SFML_DLL ${SFML_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SFML_DLL}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach()
endif()