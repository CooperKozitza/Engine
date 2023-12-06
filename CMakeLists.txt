cmake_minimum_required(VERSION 3.0)
project(Engine)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Vulkan SDK path as input
set(VULKAN_SDK_PATH "" CACHE PATH "Path to Vulkan SDK")

# Include directories
include_directories(${VULKAN_SDK_PATH}/Include)
include_directories(${CMAKE_SOURCE_DIR}/lib/glm)
include_directories(${CMAKE_SOURCE_DIR}/lib/glfw/include)

# Library directories
link_directories(${VULKAN_SDK_PATH}/Lib)
link_directories(${CMAKE_SOURCE_DIR}/lib/glfw/lib-mingw-w64)

# Automatically include all .cpp, .h, and .hpp files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Add the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Compiler options based on configuration
if(MSVC)
    # Debug configuration
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W3 /D_DEBUG /D_CONSOLE>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )

    # Release configuration
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/O2 /W3 /DNDEBUG /D_CONSOLE>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
    )
endif()

if(GCC)
    # Debug configuration
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/W3 /D_DEBUG /D_CONSOLE>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:_DEBUG>
    )

    # Release configuration
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/O2 /W3 /DNDEBUG /D_CONSOLE>
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
    )
endif()

# Link libraries
# Note: Update these library paths as per your project setup
target_link_libraries(${PROJECT_NAME}
    $<$<CONFIG:Debug>:glfw3.lib;vulkan-1.lib>
    $<$<CONFIG:Release>:glfw3.lib;vulkan-1.lib>
)

# Set the runtime output directory (optional)
set_target_properties(${PROJECT_NAME} PROPERTIES
	CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/obj
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
