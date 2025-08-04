cmake_minimum_required(VERSION 3.14)

project(
    my-first-project
    VERSION 0.1.0
    DESCRIPTION "my description"
    LANGUAGES CXX
)

# ---- Declare library ----

add_library(mfp source/my_first_project.cpp)
add_library(my-first-project::my-first-project ALIAS mfp)


target_include_directories(
    mfp
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    mfp SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(mfp PUBLIC cxx_std_17)


# ---- Examples ----



# ---- Developer mode ----

  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()


