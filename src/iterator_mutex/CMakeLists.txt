add_library(my-first-project iterator_mutex_move_operations.cpp)

target_include_directories(
    my-first-project
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_include_directories(
    my-first-project SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(my-first-project PUBLIC cxx_std_17)
