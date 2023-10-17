cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ -fexperimental-library")

project(GeoLDMViz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJC)
endif()

add_subdirectory(lib)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

# Create a dummy output that depends on all resource files,
# so that changes in resource files trigger a re-copy of the changed files to the build directory.
file(GLOB_RECURSE RES_FILES LIST_DIRECTORIES false "res/*.*")
set(DUMMY_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dummy_res")
add_custom_command(
    OUTPUT ${DUMMY_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E copy_directory "../res" "${CMAKE_CURRENT_BINARY_DIR}/res"
    DEPENDS ${RES_FILES}
    COMMENT "Copying resource files to build directory"
)
add_custom_target(CopyResources ALL DEPENDS ${DUMMY_OUTPUT})

set(IMGUI_DIR lib/imgui)

add_executable(${PROJECT_NAME}
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    lib/ImGuizmo/ImGuizmo.cpp
    ${SOURCES}
)
add_dependencies(${PROJECT_NAME} CopyResources)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    lib/glm
    lib/ImGuizmo
    src
)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL GLEW::GLEW SDL3::SDL3 nfd)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-elaborated-enum-base -DIMGUI_IMPL_OPENGL_LOADER_GLEW)
