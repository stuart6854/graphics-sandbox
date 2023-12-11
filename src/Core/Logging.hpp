#pragma once

#include <fmt/format.h>

#include <iostream>

#define LOG_INFO(...) std::cout << "[INFO] " << fmt::format(__VA_ARGS__) << std::endl
#define LOG_WARN(...) std::cout << "[WARN] " << fmt::format(__VA_ARGS__) << std::endl
#define LOG_ERR(...) std::cerr << "[ERROR] " << fmt::format(__VA_ARGS__) << std::endl
