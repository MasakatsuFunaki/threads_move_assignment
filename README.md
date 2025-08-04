# Thread-Safe Iterator and Move Semantics Demo

This project demonstrates a C++ class (`DataBlockSequence`) designed to provide thread-safe access to a sorted collection of data.

It specifically focuses on:
- Using a `std::mutex` to protect shared state.
- Implementing correct, thread-safe move semantics for a class that contains a `std::mutex`.

## Demonstrating the Data Race

This project includes tests specifically designed to fail catastrophically if the class is not thread-safe. You can see this in action by following these steps.

### 1. Build the Project
First, compile the library and the tests:
```bash
python3 build.py conan
```

### 2. Introduce a Bug
Next, open the file `src/iterator_mutex/iterator_mutex_move_operations.cpp`.

Find either the move constructor or the move assignment operator and comment out the `std::scoped_lock` line:

```cpp
// In DataBlockSequence::DataBlockSequence(DataBlockSequence&& other)
// or DataBlockSequence::operator=(DataBlockSequence&& other)

// Comment out this line to introduce the data race:
// std::scoped_lock lock(mru_mutex_, other.mru_mutex_);
```

### 3. Rebuild and Run the Stress Test to See the Crash
After modifying the code, you must rebuild the project for the changes to take effect.

```bash
python3 build.py conan
```

Now, run the stress test. With the lock disabled, the class is no longer thread-safe. Because data races are timing-dependent, a single test run might not fail. The included stress test script runs the tests in a loop to reliably trigger the race condition.

Run the script from the project's root directory:
```sh
python3 stress_test.py
```
The test suite will now crash, most likely with a segmentation fault (`SIGSEGV`). This happens because a test is designed to read from the object in one thread while another thread moves it, causing the reader to access invalid memory.

Press any key to stop the script.
