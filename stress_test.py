import subprocess
import sys
import threading
import time

# --- Configuration ---
# The command to run your test suite.
# We use the existing run_tests.py to keep things consistent.
TEST_COMMAND = [sys.executable, "run_tests.py"]
# -------------------

# A flag to signal the test loop to stop.
# It's an object to be mutable across threads.
stop_signal = {"stop": False}

def listen_for_keypress(stop_flag):
    """
    Waits for a single keypress from the user and sets a flag to stop the main loop.
    This runs in a separate thread.
    """
    try:
        # This works on Unix-like systems (macOS, Linux)
        import tty
        import termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setcbreak(sys.stdin.fileno())
            sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    except (ImportError, termios.error):
        # This is a fallback for Windows or other environments
        input()
    finally:
        stop_flag["stop"] = True
        print("\n--- Keypress detected. Stopping after current test run... ---")

def main():
    """
    Runs the test command in a loop until a failure is detected or the user
    presses a key.
    """
    run_count = 0
    has_failed = False

    print("--- Starting stress test. Press any key to stop gracefully. ---")
    print(f"--- Test command: {' '.join(TEST_COMMAND)} ---")

    # Start the keypress listener in a background thread
    keypress_thread = threading.Thread(target=listen_for_keypress, args=(stop_signal,))
    keypress_thread.daemon = True  # Allows main thread to exit even if this one is running
    keypress_thread.start()

    while not stop_signal["stop"]:
        run_count += 1
        print(f"\n--- [ Run {run_count} ] ---")

        try:
            # Execute the test command
            process = subprocess.run(
                TEST_COMMAND,
                capture_output=True,
                text=True,
                check=False  # We handle the return code manually
            )

            # Check the result
            if process.returncode == 0:
                print(f"--- Result: PASSED ---")
            else:
                print(f"--- Result: FAILED (Exit Code: {process.returncode}) ---")
                print("\n--- STDOUT ---")
                print(process.stdout)
                print("\n--- STDERR ---")
                print(process.stderr)
                has_failed = True
                break  # Exit the loop on failure

        except FileNotFoundError:
            print(f"--- ERROR: Command not found: '{' '.join(TEST_COMMAND)}' ---")
            print("--- Please ensure you are in the correct directory and the test script exists. ---")
            has_failed = True
            break
        except Exception as e:
            print(f"--- An unexpected error occurred: {e} ---")
            has_failed = True
            break

    print("\n--- Stress test finished. ---")
    if has_failed:
        print("--- Failure detected. ---")
        sys.exit(1)
    else:
        print(f"--- All {run_count} runs passed successfully. ---")
        sys.exit(0)

if __name__ == "__main__":
    main()
