#pragma once

// Simple library interface
namespace mfp {
    // Basic function declaration
    int get_version();
    
    // Simple class for demonstration
    class SimpleClass {
    public:
        SimpleClass();
        ~SimpleClass();
        void do_something();
    private:
        int value_;
    };
}
