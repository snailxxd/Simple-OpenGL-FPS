cmake_minimum_required(VERSION 3.13)

project(MyLearnOpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DRESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources")

# 指定链接库
set(LIBS glfw3 assimp z opengl32)

add_library(MODEL_LIB src/model/model_gltf.cpp)
target_include_directories(MODEL_LIB PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(MODEL_LIB PRIVATE assimp z GLAD STB_IMAGE)
set(LIBS ${LIBS} MODEL_LIB)

add_library(ANIMATION_LIB src/animation/animation.cpp src/animation/bone.cpp)
target_include_directories(ANIMATION_LIB PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ANIMATION_LIB PRIVATE assimp z GLAD STB_IMAGE)
set(LIBS ${LIBS} ANIMATION_LIB)

add_library(GLAD src/glad.c)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/include)
set(LIBS ${LIBS} GLAD)

add_library(STB_IMAGE src/stb_image.cpp)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/include)
set(LIBS ${LIBS} STB_IMAGE)

file(GLOB IMGUI_SOURCES
    src/imgui/*.cpp
)
add_library(IMGUI ${IMGUI_SOURCES})
target_include_directories(IMGUI PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/imgui
)
set(LIBS ${LIBS} IMGUI)

# DEMO 列表
set(DEMOS
    render_engine
)

function(create_demo DEMO)
    file(GLOB SOURCE
        src/${DEMO}/*.h
        src/${DEMO}/*.cpp
        src/${DEMO}/*.c
        src/${DEMO}/*.vs
        src/${DEMO}/*.fs
    )
    set(NAME ${DEMO})

    # 创建可执行文件
    add_executable(${NAME}
        ${SOURCE}
    )

    # 包含目录
    target_include_directories(${NAME} PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
    )

    # 链接目录
    target_link_directories(${NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib
    )

    # 链接库
    target_link_libraries(${NAME} PRIVATE
        ${LIBS}
    )

    # 复制着色器文件到可执行文件目录
    file(GLOB SHADERS
        "resources/shaders/*.vs"
        "resources/shaders/*.fs"
    )
    foreach(SHADER ${SHADERS})
        add_custom_command(TARGET ${NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} $<TARGET_FILE_DIR:${NAME}>)
    endforeach(SHADER)
endfunction()

# 创建目标 
foreach(DEMO ${DEMOS})
    create_demo(${DEMO})
endforeach()