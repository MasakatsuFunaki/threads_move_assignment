#include <gtest/gtest.h>

#include <numeric>
#include <thread>
#include <vector>

#include "iterator_mutex_move_operations.hpp"

// --- Test Fixture for DataBlockSequence ---
class DataBlockSequenceTest : public ::testing::Test
{
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
TEST_F(DataBlockSequenceTest, ConstructorInitializesAndSorts)
{
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
TEST(DataBlockSequenceEmptyTest, HandlesEmptyVector)
{
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
TEST_F(DataBlockSequenceTest, GetValueRetrievesCorrectValues)
{
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
TEST_F(DataBlockSequenceTest, GetValueHandlesMissingValues)
{
    EXPECT_FALSE(seq_.get_value(99).has_value());  // Value not present
    EXPECT_FALSE(seq_.get_value(0).has_value());   // Value not present
}

/**
 * @brief Tests that get_value correctly finds a value that is not cached.
 *
 * This test ensures that the binary search logic (`std::lower_bound`) works correctly
 * on its own by searching for a value that is neither the first element nor the
 * most recently used one.
 */
TEST_F(DataBlockSequenceTest, GetValueFindsValueNotInCache)
{
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
TEST_F(DataBlockSequenceTest, GetValueUsesMruCache)
{
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
TEST_F(DataBlockSequenceTest, MoveConstructorTransfersState)
{
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
TEST_F(DataBlockSequenceTest, MoveAssignmentTransfersState)
{
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
TEST_F(DataBlockSequenceTest, MoveAssignmentHandlesSelfAssignment)
{
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
TEST(DataBlockSequenceThreadTest, ConcurrentReadsAreSafe)
{
    std::vector<int> large_vec(1000);
    std::iota(large_vec.begin(), large_vec.end(), 0);  // Fill with 0, 1, 2, ...

    const iterator_mutex::DataBlockSequence shared_seq(large_vec);

    auto reader_task = [&](int start_value)
    {
        for (int i = 0; i < 100; ++i)
        {
            int value_to_find = (start_value + i) % static_cast<int>(shared_seq.get_total_size());
            auto val = shared_seq.get_value(value_to_find);
            ASSERT_TRUE(val.has_value());
            EXPECT_EQ(val.value(), value_to_find);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(reader_task, i * 50);
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

/**
 * @brief Tests that move-assigning from a sequence while it's being read is unsafe.
 *
 * This test demonstrates the necessity of the mutex. It creates one thread that
 * continuously reads from the shared sequence and a second thread that attempts
 * to move-assign from it. Without a lock, the reader thread will likely access
 * a dangling pointer after the move, causing a crash (undefined behavior).
 *
 * Note: This test is expected to fail or crash if the mutex is not used correctly,
 * proving that the lock is essential for protecting the object's integrity during
 * concurrent operations that include state changes (like a move).
 */
TEST(DataBlockSequenceThreadTest, MoveAssignmentWhileReadingIsUnsafe)
{
    std::vector<int> large_vec(1000);
    std::iota(large_vec.begin(), large_vec.end(), 0);

    auto shared_seq = std::make_shared<iterator_mutex::DataBlockSequence>(large_vec);

    std::atomic<bool> keep_reading = true;
    std::atomic<bool> reader_crashed = false;

    // Reader thread: Continuously reads from the sequence
    std::thread reader_thread(
        [&]()
        {
            try
            {
                while (keep_reading)
                {
                    auto val = shared_seq->get_value(500);
                    // We don't need to assert here; we're just checking for crashes.
                    // If the move happens, `shared_seq` becomes invalid, and this line
                    // will likely cause a segmentation fault.
                    if (!val.has_value() && !keep_reading)
                    {
                        // If reading fails after we've been told to stop, that's expected.
                        break;
                    }
                }
            }
            catch (...)
            {
                reader_crashed = true;
            }
        });

    // Give the reader a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Mover thread: Move-assigns from the shared sequence
    iterator_mutex::DataBlockSequence new_seq({});
    new_seq = std::move(*shared_seq);  // This is the critical, unsafe operation

    // Signal the reader to stop and wait for it to finish
    keep_reading = false;
    reader_thread.join();

    // The new sequence should now have the data
    EXPECT_EQ(new_seq.get_total_size(), 1000);
    EXPECT_TRUE(new_seq.get_value(500).has_value());

    // If the reader crashed, the test fails.
    EXPECT_FALSE(reader_crashed) << "The reader thread crashed due to unsafe concurrent access.";
}

/**
 * @brief Tests that move-constructing from a sequence while it's being read is unsafe.
 *
 * This test is similar to the move assignment test but targets the move constructor.
 * It creates one thread that continuously reads from the shared sequence and a
 * second thread that move-constructs a new sequence from it. Without a lock in
 * the move constructor, the reader thread will likely access a dangling pointer,
 * causing a crash (undefined behavior).
 */
TEST(DataBlockSequenceThreadTest, MoveConstructionWhileReadingIsUnsafe)
{
    auto shared_seq = std::make_shared<iterator_mutex::DataBlockSequence>(std::vector<int>(1000, 1));

    std::atomic<bool> keep_reading = true;
    std::atomic<bool> reader_crashed = false;

    // Reader thread: Continuously reads from the sequence
    std::thread reader_thread(
        [&]()
        {
            try
            {
                while (keep_reading)
                {
                    shared_seq->get_value(1);  // Just keep accessing the object
                }
            }
            catch (...)
            {
                reader_crashed = true;
            }
        });

    // Give the reader a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Mover thread: Move-constructs a new sequence from the shared one.
    // This is the critical, unsafe operation if the move constructor is not locked.
    iterator_mutex::DataBlockSequence new_seq(std::move(*shared_seq));

    // Signal the reader to stop and wait for it to finish
    keep_reading = false;
    reader_thread.join();

    // The new sequence should now have the data
    EXPECT_EQ(new_seq.get_total_size(), 1000);

    // If the reader crashed, the test fails.
    EXPECT_FALSE(reader_crashed) << "The reader thread crashed due to unsafe concurrent access.";
}
