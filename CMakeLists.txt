cmake_minimum_required(VERSION 3.14)

project(
    my-first-project
    VERSION 0.1.0
    DESCRIPTION "my description"
    LANGUAGES CXX
)

# ---- Declare library ----

add_library(my-first-project src/iterator_mutex/iterator_mutex_move_operations.cpp)

target_include_directories(
    my-first-project
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/iterator_mutex>"
)

target_include_directories(
    my-first-project SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(my-first-project PUBLIC cxx_std_17)


# ---- Examples ----



# ---- Developer mode ----

  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()


