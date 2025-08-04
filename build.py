#!/usr/bin/env python3
import argparse
import os
import shutil
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

def handle_clean(args):
    """Removes build artifacts."""
    print(f"--- Cleaning build files in '{args.build_dir}' ---")
    if os.path.isdir(args.build_dir):
        shutil.rmtree(args.build_dir)
        print(f"Removed directory: {args.build_dir}")
    if os.path.exists("CMakeUserPresets.json"):
        os.remove("CMakeUserPresets.json")
        print("Removed CMakeUserPresets.json")
    print("--- Clean finished ---")

def configure_and_build(args):
    """Runs CMake configure and build steps."""
    toolchain_file = os.path.join(args.build_dir, "conan", "conan_toolchain.cmake")
    cmake_config_cmd = [
        "cmake", "-S", ".", f"-B{args.build_dir}",
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
        f"-DCMAKE_BUILD_TYPE={args.build_type}"
    ]
    run_command(cmake_config_cmd)
    run_command(["cmake", "--build", args.build_dir])

def handle_conan(args):
    """Runs a full build, including Conan dependency installation."""
    print(f"--- Running Conan build ({args.build_type}) ---")
    run_command([
        "conan", "install", ".", f"--output-folder={args.build_dir}",
        "--build=missing", f"-s:h", f"build_type={args.build_type}"
    ])
    configure_and_build(args)
    print("--- Conan build finished successfully ---")

def handle_cmake(args):
    """Runs a build using existing Conan dependencies."""
    print(f"--- Running CMake build ({args.build_type}) ---")
    toolchain_file = os.path.join(args.build_dir, "conan", "conan_toolchain.cmake")
    if not os.path.exists(toolchain_file):
        sys.exit(f"Error: Toolchain not found at '{toolchain_file}'. Run a 'conan' build first.")
    configure_and_build(args)
    print("--- CMake build finished successfully ---")

def main():
    parser = argparse.ArgumentParser(description="Project build script.")
    parser.add_argument("--build-type", default="Release", help="Set the build type (e.g., Debug).")
    parser.add_argument("--build-dir", default="build", help="Set the build directory.")
    
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("conan", help="Run a full build with Conan.").set_defaults(func=handle_conan)
    subparsers.add_parser("cmake", help="Run a CMake-only build.").set_defaults(func=handle_cmake)
    subparsers.add_parser("clean", help="Clean build files.").set_defaults(func=handle_clean)
    
    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    main()
