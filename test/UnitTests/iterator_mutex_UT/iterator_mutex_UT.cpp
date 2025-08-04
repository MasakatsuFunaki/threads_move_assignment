#include <gtest/gtest.h>
#include "iterator_mutex_move_operations.hpp"

TEST(DataBlockSequenceTest, GetValue) {
    std::vector<int> values = {1, 2, 3, 4, 5};
    iterator_mutex::DataBlockSequence seq(values);

    EXPECT_EQ(seq.get_total_size(), 5);
    EXPECT_EQ(seq.get_value(0).value_or(-1), 1);
    EXPECT_EQ(seq.get_value(4).value_or(-1), 5);
    EXPECT_EQ(seq.get_value(5).has_value(), false);
}
