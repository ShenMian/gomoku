#include "gomoku.hpp"

int main()
{
	Gomoku gomoku;

	try
	{
		gomoku.run();
	}
	catch(std::runtime_error& e)
	{
		std::cerr << "ERROR: " << e.what() << "\n";
		std::cerr << "Press enter to exit...\n";
		std::string line;
		std::getline(std::cin, line);
		return 1;
	}

	return 0;
}
