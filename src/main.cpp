// Copyright 2023 ShenMian
// License(Apache-2.0)

#include "gomoku.hpp"

int main() {
    try {
        Gomoku gomoku;
        gomoku.run();
        return 0;
    } catch (const std::runtime_error& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Unknown exception\n";
    }
    std::cerr << "Press enter to exit...\n";
    std::string line;
    std::getline(std::cin, line);
    std::getline(std::cin, line);
    return 1;
}
