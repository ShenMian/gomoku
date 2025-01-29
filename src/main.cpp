// Copyright 2023-2025 ShenMian
// License(Apache-2.0)

#include <print>

#include "gomoku.hpp"

auto main() -> int {
    try {
        Gomoku gomoku;
        gomoku.run();
        return 0;
    } catch (const std::runtime_error& e) {
        std::println("Exception: {}", e.what());
    } catch (...) {
        std::println("Unknown exception");
    }
    std::println("Press enter to exit...");
    std::string line;
    std::getline(std::cin, line);
    std::getline(std::cin, line);
    return 1;
}
