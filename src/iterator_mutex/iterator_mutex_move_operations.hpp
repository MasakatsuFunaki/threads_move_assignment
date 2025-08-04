#pragma once

#include <vector>
#include <mutex>
#include <optional>

namespace iterator_mutex {

class DataBlockSequence
{
public:
    DataBlockSequence(const std::vector<int>& values);

    // Delete copy constructor and assignment operator
    DataBlockSequence(const DataBlockSequence&) = delete;
    DataBlockSequence& operator=(const DataBlockSequence&) = delete;
    // Allow move constructor and assignment operator
    DataBlockSequence(DataBlockSequence&& other) noexcept;
    DataBlockSequence& operator=(DataBlockSequence&& other) noexcept;

    std::optional<int> get_value(int value) const;

    size_t get_total_size() const;

private:
    std::vector<int> blocks_;
    mutable std::vector<int>::const_iterator mru_block_iterator_;
    mutable std::mutex mru_mutex_;
};

}
