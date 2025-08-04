#include "iterator_mutex_move_operations.hpp"

#include <algorithm>
#include <iostream>

namespace iterator_mutex
{

DataBlockSequence::DataBlockSequence(const std::vector<int>& values) : blocks_(values)
{
    std::sort(blocks_.begin(), blocks_.end());
    if (!blocks_.empty())
    {
        mru_block_iterator_ = blocks_.cbegin();
    }
}

// Custom Move Constructor
DataBlockSequence::DataBlockSequence(DataBlockSequence&& other) noexcept
    // 1. Move the vector. This is the main resource transfer.
    : blocks_(std::move(other.blocks_))
// 2. The mutex is NOT moved. The new object's mutex is default-initialized to an unlocked state.
{
    // 3. The iterator from 'other' is now invalid. Initialize the new object's
    //    iterator to a valid state relative to its newly acquired vector.
    mru_block_iterator_ = blocks_.cbegin();

    // The 'other' object is left in a valid but empty state.
    // We can also clear its iterator to be safe.
    other.mru_block_iterator_ = other.blocks_.cbegin();
}

// Custom Move Assignment Operator
DataBlockSequence& DataBlockSequence::operator=(DataBlockSequence&& other) noexcept
{
    // Protect against self-assignment
    if (this == &other)
    {
        return *this;
    }

    // Since a move operation should only happen when there's exclusive access to the
    // source object, we don't typically need to lock the source mutex. We should
    // lock our own mutex to protect our own state during the modification.
    std::lock_guard<std::mutex> lock(mru_mutex_);

    // 1. Move the vector's contents.
    blocks_ = std::move(other.blocks_);

    // 2. Re-initialize our iterator to be valid for the new data.
    mru_block_iterator_ = blocks_.cbegin();

    // The 'other' object is now empty. We can clear its iterator for safety.
    // We would need to lock its mutex if we wanted to modify it safely, but
    // again, the assumption is no other thread is using 'other'.
    other.mru_block_iterator_ = other.blocks_.cbegin();

    return *this;
}

std::optional<int> DataBlockSequence::get_value(int value) const
{
    std::lock_guard<std::mutex> lock(mru_mutex_);

    // 1. Check the MRU cache first.
    if (mru_block_iterator_ != blocks_.cend() && *mru_block_iterator_ == value)
    {
        return *mru_block_iterator_;
    }

    // 2. If not in cache, perform a binary search.
    auto it = std::lower_bound(blocks_.cbegin(), blocks_.cend(), value);

    // 3. Check if we found the exact value.
    if (it != blocks_.cend() && *it == value)
    {
        mru_block_iterator_ = it;  // Update cache
        return *it;
    }
    // 4. Value not found.
    return std::nullopt;
}

size_t DataBlockSequence::get_total_size() const
{
    return blocks_.size();
}

}  // namespace iterator_mutex
