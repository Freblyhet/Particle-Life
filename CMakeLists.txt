cmake_minimum_required(VERSION 3.16)
project(ParticleLife)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(ParticleLife
    src/main.cpp
    src/glad.c
    
    # Simulation
    src/simulation/ParticleSystem.cpp
    
    # Rendering
    src/rendering/Renderer.cpp
    src/rendering/ShaderManager.cpp
    
    # UI
    src/ui/Interface.cpp
    
    # ImGui
    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)

# Include directories
target_include_directories(ParticleLife PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/glad
    ${CMAKE_CURRENT_SOURCE_DIR}/include/KHR
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/backends
)

# Find GLM (header-only library)
find_path(GLM_INCLUDE_DIR glm/glm.hpp
    PATHS
        /usr/local/include
        /opt/homebrew/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external
)

if(GLM_INCLUDE_DIR)
    target_include_directories(ParticleLife PRIVATE ${GLM_INCLUDE_DIR})
else()
    message(WARNING "GLM not found. Make sure it's installed or available in libs/")
endif()

# Link libraries
if(APPLE)
    target_link_libraries(ParticleLife 
        OpenGL::GL
        glfw
        "-framework Cocoa"
        "-framework IOKit" 
        "-framework CoreVideo"
        "-framework CoreFoundation"
    )
else()
    target_link_libraries(ParticleLife 
        OpenGL::GL
        glfw
        ${CMAKE_DL_LIBS}
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(ParticleLife PRIVATE /W4)
else()
    target_compile_options(ParticleLife PRIVATE -Wall -Wextra)
endif()

# Set output directory
set_target_properties(ParticleLife PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources 
     DESTINATION ${CMAKE_BINARY_DIR})

# Custom target for cleaning build files
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)