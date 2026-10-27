#!/usr/bin/env python3
"""Run host gesture/monitor tests; never communicate with the physical board."""
import os
import pathlib
import platform
import shlex
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / "build/tests"
BUILD.mkdir(parents=True, exist_ok=True)
command = shlex.split(os.environ.get("CXX", "c++"))
sdk = os.environ.get("MIATA_TEST_SDK")
# This host's newest SDK contains TBD architectures its linker cannot read.
# Use the installed compatible SDK locally without changing global Xcode settings.
compatible = pathlib.Path("/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk")
if platform.system() == "Darwin":
    if sdk:
        command += ["-isysroot", sdk]
    elif compatible.is_dir():
        command += ["-isysroot", str(compatible)]
command += ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-fsanitize=address,undefined"]
for name in ("gesture_engine", "led_feedback"):
    binary = BUILD / name
    subprocess.run(command + [str(ROOT / "tests" / (name + "_test.cpp")), "-o", str(binary)], cwd=ROOT, check=True)
    subprocess.run([str(binary)], cwd=ROOT, check=True)
subprocess.run([sys.executable, "-m", "unittest", "discover", "-s", "tests", "-p", "test_*.py", "-v"],
               cwd=ROOT, check=True)
