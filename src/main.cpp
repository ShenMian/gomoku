#include "board.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cassert>
#include <format>
#include <iostream>
#include <optional>

enum class Status
{
	Ready,
	Wait,
	Initial
};

Status status = Status::Initial;
Chess  chess  = Chess::Null;

Board board;

void reset()
{
	status = Status::Initial;
	chess  = Chess::Null;
	board.reset();
}

void place_chess(sf::RenderWindow& window, sf::Vector2i position, Chess chess)
{
	board.place_chess(position, chess);

	if(board.is_full())
	{
		sf::sleep(sf::seconds(5.f));
		reset();
		return;
	}

	const auto chesses = board.get_five_chesses_in_a_row(position);
	if(!chesses.has_value())
		return;

	for(int i = 0; i < 10; i++)
	{
		for(const auto& pos : chesses.value())
			board.place_chess(pos, i % 2 == 0 ? Chess::Green : chess);
		board.place_chess(position, i % 2 == 0 ? Chess::Green : chess);

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		window.display();
		sf::sleep(sf::seconds(0.5f));
	}

	reset();
}

int online(sf::RenderWindow& window)
{
	std::cout << R"(
          1. client
          2. server

)";
	std::string choice;
	std::getline(std::cin, choice);

	sf::TcpSocket socket;

	if(choice == "1")
	{
		std::cout << "host ip: ";
		std::string ip;
		std::cin >> ip;
		while(socket.connect(ip, 1234) != sf::Socket::Status::Done)
			std::cout << "retrying...\n";
	}
	else if(choice == "2")
	{
		std::cout << "local ip: " << sf::IpAddress::getLocalAddress() << "\n";
		std::cout << "waiting for connections...\n";
		sf::TcpListener listener;
		listener.listen(1234);
		assert(listener.accept(socket) == sf::Socket::Status::Done);
	}
	else
		return 1;
	socket.setBlocking(false);

	window.setVisible(true);

	while(window.isOpen())
	{
		for(auto event = sf::Event{}; window.pollEvent(event);)
		{
			if(event.type == sf::Event::Closed)
				window.close();
		}

		if(status == Status::Ready || status == Status::Initial)
		{
			if(window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				const auto result = board.window_to_board_position(sf::Mouse::getPosition(window));
				if(!result.has_value())
					continue;
				const auto position = result.value();

				if(board.get_chess(position) != Chess::Null)
					continue;

				if(chess == Chess::Null)
					chess = Chess::Black;

				sf::Packet packet;
				packet.append(&position, sizeof(position));
				assert(socket.send(packet) == sf::Socket::Status::Done);

				status = Status::Wait;
				place_chess(window, position, chess);
			}
		}

		if(status == Status::Wait || status == Status::Initial)
		{
			sf::Packet packet;
			socket.receive(packet);
			if(!packet.endOfPacket())
			{
				const sf::Vector2i position = *static_cast<const sf::Vector2i*>(packet.getData());
				assert(packet.getDataSize() == sizeof(sf::Vector2i));

				if(chess == Chess::Null)
					chess = Chess::White;

				status = Status::Ready;
				place_chess(window, position, chess == Chess::Black ? Chess::White : Chess::Black);
			}
		}

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		window.display();
	}

	return 0;
}

int offline(sf::RenderWindow& window)
{
	chess = Chess::Black;

	window.setVisible(true);

	while(window.isOpen())
	{
		for(auto event = sf::Event{}; window.pollEvent(event);)
		{
			if(event.type == sf::Event::Closed)
				window.close();
		}

		if(window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			const auto result = board.window_to_board_position(sf::Mouse::getPosition(window));
			if(!result.has_value())
				continue;
			const auto position = result.value();

			if(board.get_chess(position) != Chess::Null)
				continue;

			if(chess == Chess::Null)
				chess = Chess::Black;

			place_chess(window, position, chess);

			chess = chess == Chess::Black ? Chess::White : Chess::Black;
		}

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		window.display();
	}

	return 0;
}

int main()
{
	reset();

	const sf::Vector2u size(board.position().x * 2 + (board.size().x - 1) * 46,
	                        board.position().y * 2 + (board.size().y - 1) * 46);
	auto               window = sf::RenderWindow{{size.x, size.y}, "Gomoku", sf::Style::Close};
	window.setVisible(false);
	window.setSize(size);
	window.setFramerateLimit(60);


	std::cout << R"(
  _____                __       
 / ___/__  __ _  ___  / /____ __
/ (_ / _ \/  ' \/ _ \/  '_/ // /
\___/\___/_/_/_/\___/_/\_\\_,_/ 
)";
	std::cout << R"(
          1. online
          2. offline

)";

	std::string choice;
	std::getline(std::cin, choice);

	if(choice == "1")
		online(window);
	else if(choice == "2")
		return offline(window);
	else
		return 1;
}
