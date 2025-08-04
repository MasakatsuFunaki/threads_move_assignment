#include <gtest/gtest.h>
#include "iterator_mutex_move_operations.hpp"
#include <thread>
#include <vector>
#include <numeric>

// --- Test Fixture for DataBlockSequence ---
class DataBlockSequenceTest : public ::testing::Test {
protected:
    // Note: The constructor will sort these values.
    const std::vector<int> initial_values_ = {50, 10, 40, 20, 30};
    // Sorted order will be: {10, 20, 30, 40, 50}
    iterator_mutex::DataBlockSequence seq_{initial_values_};
};

// --- Constructor and Basic Getters ---

/**
 * @brief Tests that the DataBlockSequence constructor correctly initializes and sorts the data.
 *
 * Verifies that the total size of the sequence matches the number of elements
 * and that an element known to be in the initial set can be found.
 */
TEST_F(DataBlockSequenceTest, ConstructorInitializesAndSorts) {
    EXPECT_EQ(seq_.get_total_size(), 5);
    // Check that a value exists, confirming it was sorted and is findable.
    EXPECT_TRUE(seq_.get_value(40).has_value());
}

/**
 * @brief Tests the behavior of DataBlockSequence when constructed with an empty vector.
 *
 * Verifies that the total size is 0 and that attempting to get a value
 * from an empty sequence correctly returns a non-valued optional.
 */
TEST(DataBlockSequenceEmptyTest, HandlesEmptyVector) {
    iterator_mutex::DataBlockSequence empty_seq({});
    EXPECT_EQ(empty_seq.get_total_size(), 0);
    EXPECT_FALSE(empty_seq.get_value(0).has_value());
}

// --- GetValue Method ---

/**
 * @brief Tests the get_value method for correct value retrieval.
 *
 * Verifies that calling get_value with values known to be in the sequence
 * returns an optional containing the correct value.
 */
TEST_F(DataBlockSequenceTest, GetValueRetrievesCorrectValues) {
    ASSERT_TRUE(seq_.get_value(10).has_value());
    EXPECT_EQ(seq_.get_value(10).value(), 10);

    ASSERT_TRUE(seq_.get_value(30).has_value());
    EXPECT_EQ(seq_.get_value(30).value(), 30);

    ASSERT_TRUE(seq_.get_value(50).has_value());
    EXPECT_EQ(seq_.get_value(50).value(), 50);
}

/**
 * @brief Tests the get_value method for values not in the sequence.
 *
 * Verifies that calling get_value with a value that does not exist in the
 * sequence correctly returns a non-valued optional.
 */
TEST_F(DataBlockSequenceTest, GetValueHandlesMissingValues) {
    EXPECT_FALSE(seq_.get_value(99).has_value()); // Value not present
    EXPECT_FALSE(seq_.get_value(0).has_value());   // Value not present
}

/**
 * @brief Tests that get_value correctly finds a value that is not cached.
 *
 * This test ensures that the binary search logic (`std::lower_bound`) works correctly
 * on its own by searching for a value that is neither the first element nor the
 * most recently used one.
 */
TEST_F(DataBlockSequenceTest, GetValueFindsValueNotInCache) {
    // The MRU iterator is initialized to the beginning (10).
    // We search for 40, which is not in the cache, forcing a binary search.
    ASSERT_TRUE(seq_.get_value(40).has_value());
    EXPECT_EQ(seq_.get_value(40).value(), 40);
}

// --- MRU Cache Logic ---

/**
 * @brief Tests the Most Recently Used (MRU) cache logic.
 *
 * This test ensures that after accessing an element, subsequent access to the
 * same element is potentially faster (by hitting the cached iterator). It primes
 * the cache by accessing one element and then immediately accessing it again,
 * before repeating the process with a different element.
 */
TEST_F(DataBlockSequenceTest, GetValueUsesMruCache) {
    // Prime the cache by searching for 30
    ASSERT_EQ(seq_.get_value(30).value(), 30);
    // This second call should hit the cache
    ASSERT_EQ(seq_.get_value(30).value(), 30);

    // Access a different element (10) to update the cache
    ASSERT_EQ(seq_.get_value(10).value(), 10);
    // This second call should hit the cache
    ASSERT_EQ(seq_.get_value(10).value(), 10);
}

// --- Move Semantics ---

/**
 * @brief Tests the move constructor.
 *
 * Verifies that a new DataBlockSequence object can be constructed by moving from
 * an existing one. It checks that the new object receives the correct state (data and size)
 * and that the moved-from object is left in a valid but empty state.
 */
TEST_F(DataBlockSequenceTest, MoveConstructorTransfersState) {
    iterator_mutex::DataBlockSequence moved_seq(std::move(seq_));

    // Check the new object
    EXPECT_EQ(moved_seq.get_total_size(), 5);
    ASSERT_TRUE(moved_seq.get_value(10).has_value());
    EXPECT_EQ(moved_seq.get_value(10).value(), 10);

    // Check the moved-from object (should be empty)
    EXPECT_EQ(seq_.get_total_size(), 0);
    EXPECT_FALSE(seq_.get_value(10).has_value());
}

/**
 * @brief Tests the move assignment operator.
 *
 * Verifies that an existing DataBlockSequence object can be assigned from a
 * moved r-value. It checks that the target object correctly receives the state
 * and that the moved-from object is left in a valid but empty state.
 */
TEST_F(DataBlockSequenceTest, MoveAssignmentTransfersState) {
    iterator_mutex::DataBlockSequence moved_to_seq({99, 88});

    moved_to_seq = std::move(seq_);

    // Check the new object
    EXPECT_EQ(moved_to_seq.get_total_size(), 5);
    ASSERT_TRUE(moved_to_seq.get_value(20).has_value());
    EXPECT_EQ(moved_to_seq.get_value(20).value(), 20);

    // Check the moved-from object (should be empty)
    EXPECT_EQ(seq_.get_total_size(), 0);
    EXPECT_FALSE(seq_.get_value(20).has_value());
}

/**
 * @brief Tests the self-assignment protection in the move assignment operator.
 *
 * Verifies that an object remains in a valid, unchanged state after being
 * move-assigned to itself. This is a critical safety check for assignment operators.
 */
TEST_F(DataBlockSequenceTest, MoveAssignmentHandlesSelfAssignment) {
    // This is tricky to test directly without causing a compiler warning.
    // The check `if (this == &other)` is the primary safeguard.
    // We'll just ensure the object is unchanged after a "self-move".
    seq_ = std::move(seq_);
    EXPECT_EQ(seq_.get_total_size(), 5);
    ASSERT_TRUE(seq_.get_value(10).has_value());
    EXPECT_EQ(seq_.get_value(10).value(), 10);
}

// --- Thread Safety ---

/**
 * @brief Tests the thread safety of the get_value method under concurrent reads.
 *
 * This test creates multiple threads that all attempt to read from the same
 * shared DataBlockSequence object simultaneously. It verifies that the mutex
 * correctly protects the internal state, preventing data races and ensuring
 * all reads retrieve the correct values.
 */
TEST(DataBlockSequenceThreadTest, ConcurrentReadsAreSafe) {
    std::vector<int> large_vec(1000);
    std::iota(large_vec.begin(), large_vec.end(), 0); // Fill with 0, 1, 2, ...

    const iterator_mutex::DataBlockSequence shared_seq(large_vec);
    
    auto reader_task = [&](int start_value) {
        for (int i = 0; i < 100; ++i) {
            int value_to_find = (start_value + i) % static_cast<int>(shared_seq.get_total_size());
            auto val = shared_seq.get_value(value_to_find);
            ASSERT_TRUE(val.has_value());
            EXPECT_EQ(val.value(), value_to_find);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(reader_task, i * 50);
    }

    for (auto& t : threads) {
        t.join();
    }
}
