#include <gtest/gtest.h>
#include "my-first-project/my-first-project.hpp"

// Basic test for the version function
TEST(MyFirstProjectTest, GetVersion) {
    EXPECT_EQ(mfp::get_version(), 1);
}

// Basic test for the SimpleClass
TEST(MyFirstProjectTest, SimpleClass) {
    mfp::SimpleClass obj;
    // Test that we can create and use the object
    EXPECT_NO_THROW(obj.do_something());
}

// Another simple test
TEST(MyFirstProjectTest, SimpleClassMultipleCalls) {
    mfp::SimpleClass obj;
    EXPECT_NO_THROW(obj.do_something());
    EXPECT_NO_THROW(obj.do_something());
}
