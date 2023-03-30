// Copyright 2023 ShenMian
// License(Apache-2.0)

#pragma once

#include "board.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

inline sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2i& position)
{
	return packet << position.x << position.y;
}

inline sf::Packet& operator>>(sf::Packet& packet, sf::Vector2i& position)
{
	return packet >> position.x >> position.y;
}

enum class Status
{
	Ready,
	Wait,
	Initial
};

class Gomoku
{
public:
	void run()
	{
		reset();

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
		{
			const uint16_t port = 1234;

			std::cout << R"(
          1. client
          2. server

)";
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
				if(listener.accept(socket) != sf::Socket::Status::Done)
					throw std::runtime_error("failed to accept socket");
			}
			else
				throw std::runtime_error("invalid option");

			create_window();
			socket.setBlocking(false);

			online(socket);
		}
		else if(choice == "2")
		{
			create_window();
			chess = Chess::Black;

			offline();
		}
		else
			throw std::runtime_error("invalid option");
	}

private:
	void online(sf::TcpSocket& socket)
	{
		while(window.isOpen())
		{
			for(auto event = sf::Event{}; window.pollEvent(event);)
				if(event.type == sf::Event::Closed)
					window.close();

			if(window.hasFocus())
				handle_cursor_move();

			if(player_status == Status::Ready || player_status == Status::Initial)
			{
				if(window.hasFocus() && handle_chess_place())
				{
					if(player_status == Status::Initial)
						chess = Chess::Black;

					sf::Packet packet;
					packet << cursor_position;
					send(socket, packet);
					player_status = Status::Wait;
					handle_over(cursor_position);
				}
			}

			if(player_status == Status::Wait || player_status == Status::Initial)
			{
				sf::Packet packet;
				if(receive(socket, packet))
				{
					if(player_status == Status::Initial)
						chess = Chess::White;

					sf::Vector2i position;
					packet >> position;

					board.place_chess(position, chess == Chess::Black ? Chess::White : Chess::Black);

					player_status = Status::Ready;
					handle_over(position);
				}
			}

			render();
		}
	}

	void offline()
	{
		while(window.isOpen())
		{
			for(auto event = sf::Event{}; window.pollEvent(event);)
				if(event.type == sf::Event::Closed)
					window.close();

			if(window.hasFocus())
			{
				const auto undo_key = sf::Keyboard::Key::BackSpace;
				if(sf::Keyboard::isKeyPressed(undo_key))
				{
					board.undo();
					chess = chess == Chess::Black ? Chess::White : Chess::Black;
					while(sf::Keyboard::isKeyPressed(undo_key))
						;
				}
				handle_cursor_move();

				if(handle_chess_place())
				{
					handle_over(cursor_position);
					chess = chess == Chess::Black ? Chess::White : Chess::Black;
				}
			}

			render();
		}
	}

	void create_window()
	{
		const sf::Vector2u window_size(static_cast<unsigned>(board.position().x) * 2 + (static_cast<unsigned>(board.size().x) - 1) * 46,
		                               static_cast<unsigned>(board.position().y) * 2 + (static_cast<unsigned>(board.size().y) - 1) * 46);

		window.create(sf::VideoMode{window_size.x, window_size.y}, "Gomoku", sf::Style::Close);
		window.setFramerateLimit(60);
	}

	void reset()
	{
		player_status   = Status::Initial;
		chess           = Chess::Null;
		cursor_position = board.size() / 2;
		board.reset();
	}

	void draw_cursor()
	{
		sf::CircleShape cursor(40 / 2.f, 50);
		cursor.setOrigin(cursor.getRadius(), cursor.getRadius());
		cursor.setFillColor(sf::Color(255 / 2, 255 / 2, 255 / 2, 150));
		cursor.setPosition(board.position().x + cursor_position.x * 46, board.position().y + cursor_position.y * 46);
		window.draw(cursor);
	}

	void handle_over(sf::Vector2i position)
	{
		if(board.is_full())
		{
			window.clear(sf::Color(242, 208, 75));
			board.draw(window);
			window.display();
			sf::sleep(sf::seconds(5.f));

			reset();
			return;
		}

		const auto chesses = board.get_five_in_a_row(position);
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

	void handle_mouse_input()
	{
		if(const auto result = board.window_to_board_position(sf::Mouse::getPosition(window)); result.has_value())
			cursor_position = result.value();
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

	void handle_cursor_move()
	{
		handle_mouse_input();
		handle_keyboard_input();
	}

	bool handle_chess_place()
	{
		static sf::Clock clock;
		if(clock.getElapsedTime() < sf::seconds(0.2f))
			return false;

		if((board.window_to_board_position(sf::Mouse::getPosition(window)).has_value() &&
		    sf::Mouse::isButtonPressed(sf::Mouse::Left)) ||
		   sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
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

	void render()
	{
		window.clear(sf::Color(242, 208, 75));
		board.draw(window);
		draw_cursor();
		window.display();
	}

	static void send(sf::TcpSocket& socket, sf::Packet& packet)
	{
		sf::Socket::Status status;
		do
		{
			status = socket.send(packet);
		} while(status == sf::Socket::Status::Partial);
		if(status == sf::Socket::Status::Done)
			return;

		if(status == sf::Socket::Disconnected)
			throw std::runtime_error("the network connection has been lost");
		throw std::runtime_error("unknown network error");
	}

	static bool receive(sf::TcpSocket& socket, sf::Packet& packet)
	{
		sf::Socket::Status status;
		do
		{
			status = socket.receive(packet);
		} while(status == sf::Socket::Status::Partial);
		if(status == sf::Socket::Status::NotReady)
			return false;
		else if(status == sf::Socket::Status::Done)
			return true;

		if(status == sf::Socket::Disconnected)
			throw std::runtime_error("the network connection has been lost");
		throw std::runtime_error("unknown network error");
	}

	Status           player_status;
	Chess            chess;
	sf::Vector2i     cursor_position;
	Board            board;
	sf::RenderWindow window;
};
