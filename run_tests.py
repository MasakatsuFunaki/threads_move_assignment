#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys

def run_command(command):
    """Runs a command and exits on failure."""
    print(f"--- Running: {' '.join(command)}", flush=True)
    try:
        subprocess.run(command, check=True, text=True)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"--- Command failed: {e}", file=sys.stderr, flush=True)
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Project test runner script.")
    parser.add_argument("--build-dir", default="build", help="Set the build directory.")
    
    args = parser.parse_args()

    test_executable_path = os.path.join(args.build_dir, "test", "UnitTests", "iterator_mutex_UT", "iterator_mutex_UT")

    if not os.path.exists(test_executable_path):
        print(f"Error: Test executable not found at '{test_executable_path}'", file=sys.stderr)
        print("Please build the project first using 'python3 build.py conan'", file=sys.stderr)
        sys.exit(1)

    print(f"--- Running tests for: {test_executable_path} ---")
    run_command([test_executable_path])
    print("--- Tests finished successfully ---")

if __name__ == "__main__":
    main()
