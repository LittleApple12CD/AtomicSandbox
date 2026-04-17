cmake_minimum_required(VERSION 3.15)
project(AtomicSandbox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# 源文件列表（glad.c 必须包含）
set(SOURCES
    src/main.cpp
    src/Simulation.cpp
    src/Renderer.cpp
    src/Particle.cpp
    src/PhysicsEngine.cpp
    src/LODSystem.cpp
    src/TimeManager.cpp
    src/glad.c
)

add_executable(AtomicSandbox ${SOURCES})

# 包含目录
target_include_directories(AtomicSandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad   # glad 头文件路径
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(AtomicSandbox
    glfw
    ${GLFW_LIBRARIES}
)

# 复制着色器到构建目录
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# 设置 C++ 编译选项
target_compile_options(AtomicSandbox PRIVATE -Wall -Wextra -O3)