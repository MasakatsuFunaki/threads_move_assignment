#include "iterator_mutex_move_operations.hpp"
#include <iostream>
#include <algorithm>

namespace iterator_mutex {

DataBlockSequence::DataBlockSequence(const std::vector<int>& values)
    : blocks_(values) {
    if (!blocks_.empty()) {
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
    if (this == &other) {
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

std::optional<int> DataBlockSequence::get_value(size_t index) const {
    std::lock_guard<std::mutex> lock(mru_mutex_);

    if (index >= blocks_.size()) {
        return std::nullopt;
    }

    if (mru_block_iterator_ != blocks_.cend() && std::distance(blocks_.cbegin(), mru_block_iterator_) == index) {
        return *mru_block_iterator_;
    }

    mru_block_iterator_ = blocks_.cbegin() + index;
    return *mru_block_iterator_;
}

size_t DataBlockSequence::get_total_size() const {
    return blocks_.size();
}

} // namespace iterator_mutex
