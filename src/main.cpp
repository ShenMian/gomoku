// Copyright 2023 ShenMian
// License(Apache-2.0)

#include "gomoku.hpp"

int main()
{
	try
	{
		Gomoku gomoku;
		gomoku.run();
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
		std::cerr << "Press enter to exit...\n";
		std::string line;
		std::getline(std::cin, line);
		std::getline(std::cin, line);
		return 1;
	}
	catch(...)
	{
		std::cerr << "Unknown exception\b";
		std::cerr << "Press enter to exit...\n";
		std::string line;
		std::getline(std::cin, line);
		std::getline(std::cin, line);
		return 1;
	}


	return 0;
}
