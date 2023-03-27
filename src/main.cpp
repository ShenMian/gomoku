#include "board.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cassert>
#include <iostream>
#include <optional>

enum class Status
{
	Ready,
	Wait,
	Initial
};

Status       chess_status = Status::Initial;
Chess        chess        = Chess::Null;
sf::Vector2i cursor_position;

Board board;

sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2i& position)
{
	return packet << position.x << position.y;
}

sf::Packet& operator>>(sf::Packet& packet, sf::Vector2i& position)
{
	return packet >> position.x >> position.y;
}

void reset()
{
	chess_status = Status::Initial;
	chess        = Chess::Null;
	board.reset();
	cursor_position = board.size() / 2;
}

void draw_cursor(sf::RenderWindow& window)
{
	sf::CircleShape cursor(40 / 2.f, 50);
	cursor.setOrigin(cursor.getRadius(), cursor.getRadius());
	cursor.setFillColor(sf::Color(255 / 2, 255 / 2, 255 / 2, 150));
	cursor.setPosition(board.position().x + cursor_position.x * 46, board.position().y + cursor_position.y * 46);
	window.draw(cursor);
}

void handle_over(sf::RenderWindow& window, sf::Vector2i position)
{
	if(board.is_full())
	{
		sf::sleep(sf::seconds(5.f));
		reset();
		return;
	}

	const auto chesses = board.get_five_chesses_in_a_row(position);
	if(!chesses.has_value())
		return;

	const auto winner_chess = board.get_chess(position);

	for(int i = 0; i < 10; i++)
	{
		for(const auto& pos : chesses.value())
			board.place_chess(pos, i % 2 == 0 ? Chess::Green : winner_chess);
		board.place_chess(position, i % 2 == 0 ? Chess::Green : winner_chess);

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		window.display();
		sf::sleep(sf::seconds(0.5f));
	}

	reset();
}

void handle_mouse_input(sf::RenderWindow& window)
{
	const auto result = board.window_to_board_position(sf::Mouse::getPosition(window));
	if(result.has_value())
	{
		const auto position = result.value();
		cursor_position     = position;
	}
}

void handle_keyboard_input()
{
	static sf::Clock clock;
	if(clock.getElapsedTime() < sf::seconds(0.2f))
		return;

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	{
		cursor_position.y--;
		clock.restart();
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
	{
		cursor_position.y++;
		clock.restart();
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
	{
		cursor_position.x--;
		clock.restart();
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
		cursor_position.x++;
		clock.restart();
	}

	cursor_position.x = std::clamp(cursor_position.x, 0, board.size().x - 1);
	cursor_position.y = std::clamp(cursor_position.y, 0, board.size().y - 1);
}

bool handle_input(sf::RenderWindow& window)
{
	handle_mouse_input(window);
	handle_keyboard_input();

	static sf::Clock clock;
	if(clock.getElapsedTime() < sf::seconds(0.2f))
		return false;

	if(sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		clock.restart();

		if(board.get_chess(cursor_position) != Chess::Null)
			return false;

		if(chess == Chess::Null)
			chess = Chess::Black;

		board.place_chess(cursor_position, chess);

		return true;
	}

	return false;
}

int online(sf::RenderWindow& window)
{
	const uint16_t port = 1234;

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
		while(socket.connect(ip, port) != sf::Socket::Status::Done)
			std::cout << "retrying...\n";
	}
	else if(choice == "2")
	{
		std::cout << "local ip: " << sf::IpAddress::getLocalAddress() << "\n";
		std::cout << "waiting for connections...\n";
		sf::TcpListener listener;
		listener.listen(port);
		while(listener.accept(socket) != sf::Socket::Status::Done)
			std::cout << "retrying...\n";
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

		if(chess_status == Status::Ready || chess_status == Status::Initial)
		{
			if(window.hasFocus() && handle_input(window))
			{
				sf::Packet packet;
				packet << cursor_position;
				const auto status = socket.send(packet);
				assert(status == sf::Socket::Status::Done);

				chess_status = Status::Wait;

				handle_over(window, cursor_position);
			}
		}

		if(chess_status == Status::Wait || chess_status == Status::Initial)
		{
			sf::Packet packet;
			const auto status = socket.receive(packet);
			if(status == sf::Socket::Done)
			{
				sf::Vector2i position;
				packet >> position;

				if(chess == Chess::Null)
					chess = Chess::White;

				chess_status = Status::Ready;

				board.place_chess(position, chess == Chess::Black ? Chess::White : Chess::Black);
				handle_over(window, position);
			}
			else if(status == sf::Socket::Disconnected)
				return 1;
		}

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		draw_cursor(window);
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

		if(window.hasFocus() && handle_input(window))
		{
			handle_over(window, cursor_position);
			chess = chess == Chess::Black ? Chess::White : Chess::Black;
		}

		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		draw_cursor(window);
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
          Free-style
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
