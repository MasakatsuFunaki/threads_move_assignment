#include "iterator_mutex_move_operations.hpp"
#include <iostream>

namespace mfp {
    int get_version() {
        return 1;
    }
    
    SimpleClass::SimpleClass() : value_(0) {
        // Constructor implementation
    }
    
    SimpleClass::~SimpleClass() {
        // Destructor implementation
    }
    
    void SimpleClass::do_something() {
        // Simple implementation using fmt
        std::cout << "SimpleClass is doing something!\n";
        value_++;
    }
}
