cmake_minimum_required(VERSION 3.14)

project(my-first-projectTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(GTest REQUIRED)
include(GoogleTest)

add_subdirectory(UnitTests/iterator_mutex_UT)