cmake_minimum_required(VERSION 3.14)

project(my-first-projectTests LANGUAGES CXX)


# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Test executable ----

add_executable(my-first-project_test 
    source/my_first_project_test.cpp
)

target_link_libraries(my-first-project_test PRIVATE 
    my-first-project::my-first-project
    GTest::gtest_main
    GTest::gmock_main
)

target_include_directories(my-first-project_test PRIVATE source)

target_compile_definitions(my-first-project_test PRIVATE TESTING)

gtest_discover_tests(my-first-project_test)