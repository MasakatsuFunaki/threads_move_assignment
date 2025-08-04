cmake_minimum_required(VERSION 3.14)

project(iterator_mutex_UT LANGUAGES CXX)


# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

# ---- Test executable ----

add_executable(iterator_mutex_UT
    iterator_mutex_UT.cpp
)

target_link_libraries(iterator_mutex_UT PRIVATE 
    my-first-project
    GTest::gtest_main
    GTest::gmock_main
)

target_include_directories(iterator_mutex_UT PRIVATE .)

target_compile_definitions(iterator_mutex_UT PRIVATE TESTING)

gtest_discover_tests(iterator_mutex_UT)
