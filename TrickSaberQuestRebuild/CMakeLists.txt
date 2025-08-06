cmake_minimum_required(VERSION 3.22)

# Validate NDK version compatibility
if(CMAKE_ANDROID_NDK_VERSION VERSION_LESS "25.0.0")
    message(WARNING "NDK version ${CMAKE_ANDROID_NDK_VERSION} may be incompatible. Recommended: 25.0.0+")
endif()

# include some defines automatically made by qpm
include(qpm_defines.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# define the project
project(tricksaber)

# compile options used
add_compile_options(-frtti -fexceptions)
add_compile_options(-Wno-unused-variable -Wno-unused-parameter)

# build-type appropriate optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-O2 -g)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
else()
    # Default to RelWithDebInfo optimization
    add_compile_options(-O2 -g)
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-ffast-math -funroll-loops)
endif()

# compile definitions used
add_compile_definitions(VERSION=\"${CMAKE_PROJECT_VERSION}\")
add_compile_definitions(MOD_ID=\"${CMAKE_PROJECT_NAME}\")

# recursively get all src files
RECURSE_FILES(cpp_file_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
RECURSE_FILES(c_file_list ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

# add all src files to compile
add_library(
        ${CMAKE_PROJECT_NAME}
        SHARED
        ${cpp_file_list}
        ${c_file_list}
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE .)

# add src dir as include dir
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
# add include dir as include dir
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
# add shared dir as include dir
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shared)
# codegen includes
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

# Enable link-time optimization for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# add extern stuff like libs and other includes
include(extern.cmake)

# Enable testing and include GTest
option(BUILD_TESTS "Build unit tests" OFF)
option(BUILD_HOST_TESTS "Build host-native tests for macOS" OFF)

if(BUILD_TESTS AND ANDROID)
    include(cmake/gtest.cmake)
elseif(BUILD_HOST_TESTS)
    include(cmake/host-tests.cmake)
endif()

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} -d --strip-all
        "lib${CMAKE_PROJECT_NAME}.so" -o "stripped_lib${CMAKE_PROJECT_NAME}.so"
        COMMENT "Strip debug symbols done on final binary.")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory debug
        COMMENT "Rename the lib to debug_ since it has debug symbols"
        )

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "lib${CMAKE_PROJECT_NAME}.so" "debug_lib${CMAKE_PROJECT_NAME}.so"
        COMMENT "Rename the lib to debug_ since it has debug symbols"
        )

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "stripped_lib${CMAKE_PROJECT_NAME}.so" "lib${CMAKE_PROJECT_NAME}.so"
        COMMENT "Rename the stripped lib to regular"
        )

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename "debug_lib${CMAKE_PROJECT_NAME}.so" "debug/lib${CMAKE_PROJECT_NAME}.so"
        COMMENT "Move the debug lib to debug folder"
        )