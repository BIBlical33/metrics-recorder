# Metrics Recorder

[![CMake](https://img.shields.io/badge/CMake-3.20+-blue.svg)](https://cmake.org)
[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

## Overview
**Metrics Recorder** is a modern, header-only C++20 library for collecting and logging time-stamped metrics in a lock-free, multi-threaded environment. It is designed for scenarios where event metrics (such as CPU usage or HTTP requests per second) must be recorded with minimal performance overhead and serialized to a human-readable log file.

## Features
- Header-only C++20 library — easy to integrate
- Lock-free metric updates using [libcds](https://github.com/khizmax/libcds) skip-list
- Supports any value type (`float`, `int`, etc.)
- Clean, timestamped text log format
- Zero interference with application threads that produce events

## Get started
[Build and Usage Guide](https://github.com/BIBlical33/metrics-recorder/wiki)

## Log Format
Each log line includes:
- A timestamp with millisecond precision
- Quoted metric names
- Corresponding values
**Example:**
```
2025-06-01 15:00:01.653 "CPU" 0.97 "HTTP requests RPS" 42
2025-06-01 15:00:02.653 "CPU" 1.12 "HTTP requests RPS" 30
```
