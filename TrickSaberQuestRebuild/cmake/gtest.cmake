include_guard()

message("Compiling with GTest")

# GTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Run at end to link with project
cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL _setup_gtest_project())

function(_setup_gtest_project)
    # recursively get all src files
    RECURSE_FILES(cpp_test_file_list ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
    RECURSE_FILES(c_test_file_list ${CMAKE_CURRENT_SOURCE_DIR}/test/*.c)

    add_executable(
        ${PROJECT_NAME}_test
        ${cpp_test_file_list}
        ${c_test_file_list}
    )
    target_link_libraries(
        ${PROJECT_NAME}_test
        PRIVATE ${PROJECT_NAME}
        GTest::gtest_main
    )
    
    # Add external dependencies for tests
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${EXTERN_DIR}/includes)
    target_include_directories(${PROJECT_NAME}_test SYSTEM PRIVATE ${EXTERN_DIR}/includes/libil2cpp/il2cpp/libil2cpp)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${EXTERN_DIR}/includes/bs-cordl/include)
    target_include_directories(${PROJECT_NAME}_test SYSTEM PRIVATE ${EXTERN_DIR}/includes/fmt/fmt/include/)
    target_include_directories(${PROJECT_NAME}_test SYSTEM PRIVATE ${EXTERN_DIR}/includes/paper/shared/utfcpp/source)
    
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wno-invalid-offsetof)
    target_compile_options(${PROJECT_NAME}_test PRIVATE -DNEED_UNSAFE_CSHARP)
    target_compile_options(${PROJECT_NAME}_test PRIVATE -fdeclspec)
    target_compile_options(${PROJECT_NAME}_test PRIVATE -DUNITY_2021)
    target_compile_options(${PROJECT_NAME}_test PRIVATE -DHAS_CODEGEN)
    target_compile_options(${PROJECT_NAME}_test PRIVATE -DFMT_HEADER_ONLY)

    # Add same include directories as main project
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shared)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/test)

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test)
endfunction(_setup_gtest_project)
