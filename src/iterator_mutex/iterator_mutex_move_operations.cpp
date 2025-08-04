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
{
    // Lock both mutexes to prevent deadlock and ensure safe transfer.
    // std::scoped_lock is preferred for locking multiple mutexes.
    std::scoped_lock lock(mru_mutex_, other.mru_mutex_);

    // 1. Move the vector.
    blocks_ = std::move(other.blocks_);

    // 2. The iterator from 'other' is now invalid. Initialize our iterator.
    mru_block_iterator_ = blocks_.cbegin();

    // 3. Reset the moved-from object to a valid, empty state.
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

    // Lock both mutexes to prevent deadlock and ensure safe transfer.
    std::scoped_lock lock(mru_mutex_, other.mru_mutex_);

    // 1. Move the vector's contents.
    blocks_ = std::move(other.blocks_);

    // 2. Re-initialize our iterator to be valid for the new data.
    mru_block_iterator_ = blocks_.cbegin();

    // 3. Reset the moved-from object to a valid, empty state.
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
