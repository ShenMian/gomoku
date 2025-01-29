// Copyright 2023-2025 ShenMian
// License(Apache-2.0)

#include <print>

#include "gomoku.hpp"

auto wait_for_enter() -> void {
    std::println("Press enter to exit...");
    std::string line;
    std::getline(std::cin, line);
    std::getline(std::cin, line);
}

auto main() -> int {
    try {
        Gomoku gomoku;
        gomoku.run();
    } catch (const std::runtime_error& e) {
        std::println("Exception: {}", e.what());
        wait_for_enter();
    } catch (...) {
        std::println("Unknown exception");
        wait_for_enter();
    }
    return 0;
}
