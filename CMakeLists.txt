cmake_minimum_required(VERSION 3.10)

# set the project name
project(eng LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TEST_EXECUTABLE "Build test executable" ON)

# vulkan
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK $ENV{VULKAN_SDK})
endif()

find_package(Vulkan REQUIRED)

# glfw
add_subdirectory(external/glfw)

# glm
set(GLM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glm)
if(NOT EXISTS ${GLM_INCLUDE_DIR})
    message(FATAL_ERROR "GLM not found at ${GLM_INCLUDE_DIR}")
endif()

set(SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/device.cpp"
)

add_library(eng ${SRC_FILES})

target_include_directories(eng
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${GLM_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VULKAN_SDK}/Include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(eng
    PUBLIC
        glfw
        Vulkan::Vulkan
)

install(TARGETS eng
    EXPORT eng-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT eng-targets
    FILE eng-targets.cmake
    NAMESPACE eng::
    DESTINATION lib/cmake/eng
)

if(BUILD_TEST_EXECUTABLE)
    add_executable(test "${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp")
    target_link_libraries(test PRIVATE eng)
    target_include_directories(test
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${GLM_INCLUDE_DIR}
            ${VULKAN_SDK}/Include
    )
endif()
