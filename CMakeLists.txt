cmake_minimum_required(VERSION 3.13)

project(MyLearnOpenGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DRESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources")

add_library(ENGINE_COMMON_CONFIG INTERFACE)
target_include_directories(ENGINE_COMMON_CONFIG INTERFACE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/stb_image/include
)

# GLAD
add_library(GLAD external/glad/src/glad.c)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/external/glad/include)

# STB_IMAGE
add_library(STB_IMAGE external/stb_image/src/stb_image.cpp)
target_include_directories(STB_IMAGE PUBLIC ${CMAKE_SOURCE_DIR}/external/stb_image/include)
set(LIBS ${LIBS} STB_IMAGE)

# IMGUI
file(GLOB IMGUI_SOURCES
    external/imgui/src/*.cpp
)
add_library(IMGUI ${IMGUI_SOURCES})
target_include_directories(IMGUI
    PUBLIC ${CMAKE_SOURCE_DIR}/external/imgui/include
    PRIVATE ${CMAKE_SOURCE_DIR}/external
)

# 模型库
add_library(MODEL_LIB src/model/model_gltf.cpp)
target_link_libraries(MODEL_LIB 
    PUBLIC ENGINE_COMMON_CONFIG STB_IMAGE
    PRIVATE assimp z GLAD ANIMATION_LIB
)

# 动画库
add_library(ANIMATION_LIB src/animation/animation.cpp src/animation/bone.cpp)
target_link_libraries(ANIMATION_LIB 
    PUBLIC MODEL_LIB ENGINE_COMMON_CONFIG
)

# 渲染引擎库
add_library(RENDER_ENGINE_LIB
    src/render_engine/OpenGLRenderEngine.cpp
    src/render_engine/renderer.cpp
    src/render_engine/gui_manager.cpp
)
target_link_libraries(RENDER_ENGINE_LIB
    PUBLIC MODEL_LIB ANIMATION_LIB ENGINE_COMMON_CONFIG
    PRIVATE GLAD STB_IMAGE IMGUI
)

# 场景库
add_library(WORLD_LIB src/world/scene.cpp)
target_link_libraries(WORLD_LIB PUBLIC ENGINE_COMMON_CONFIG)

# 应用层
add_library(APPLICATION_LIB src/application/Application.cpp src/application/FPSGame.cpp)
target_link_libraries(APPLICATION_LIB 
    PUBLIC RENDER_ENGINE_LIB WORLD_LIB
    PRIVATE opengl32 glfw3
)

# DEMO 列表
set(DEMOS
    demo
)

function(create_demo DEMO)
    file(GLOB SOURCE
        src/${DEMO}/*.h
        src/${DEMO}/*.cpp
        src/${DEMO}/*.c
    )

    add_executable(${DEMO} ${SOURCE})

    target_link_libraries(${DEMO} PRIVATE 
        APPLICATION_LIB
    )

    target_link_directories(${DEMO} PRIVATE
        ${CMAKE_SOURCE_DIR}/external/libs
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