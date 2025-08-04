cmake_minimum_required(VERSION 3.14)

project(
    my-first-project
    VERSION 0.1.0
    DESCRIPTION "my description"
    LANGUAGES CXX
)

add_subdirectory(src/iterator_mutex)

# ---- Examples ----



# ---- Developer mode ----

  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()


