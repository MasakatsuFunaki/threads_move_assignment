# Thread-Safe Iterator and Move Semantics Demo

This project demonstrates a C++ class (`DataBlockSequence`) designed to provide thread-safe access to a sorted collection of data.

It specifically focuses on:
- Using a `std::mutex` to protect an iterator that caches the most recently used (MRU) item.
- Implementing correct, thread-safe move semantics (move constructor and move assignment) for a class that contains a `std::mutex`.
- Using binary search (`std::lower_bound`) for efficient lookups when the requested item is not in the cache.

## How to Use

### Build the Project
To compile the library and the tests, run the following command:
```bash
python3 build.py conan
```

### Run the Tests
To execute the unit tests (using GoogleTest), run the following command:
```bash
python3 run_tests.py
```
