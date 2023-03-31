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
			chess_ = Chess::Black;

			offline();
		}
		else
			throw std::runtime_error("invalid option");
	}

private:
	void online(sf::TcpSocket& socket)
	{
		while(window_.isOpen())
		{
			handle_window_event();

			if(window_.hasFocus())
				handle_cursor_move();

			if(status_ == Status::Ready || status_ == Status::Initial)
			{
				if(window_.hasFocus() && handle_chess_place())
				{
					if(status_ == Status::Initial)
						chess_ = Chess::Black;

					sf::Packet packet;
					packet << cursor_position_;
					send(socket, packet);
					status_ = Status::Wait;
					handle_over(cursor_position_);
				}
			}

			if(status_ == Status::Wait || status_ == Status::Initial)
			{
				sf::Packet packet;
				if(receive(socket, packet))
				{
					if(status_ == Status::Initial)
						chess_ = Chess::White;

					sf::Vector2i position;
					packet >> position;

					board_.place(position, chess_ == Chess::Black ? Chess::White : Chess::Black);

					status_ = Status::Ready;
					handle_over(position);
				}
			}

			render();
		}
	}

	void offline()
	{
		while(window_.isOpen())
		{
			handle_window_event();

			if(window_.hasFocus())
			{
				const auto undo_key = sf::Keyboard::Key::BackSpace;
				if(sf::Keyboard::isKeyPressed(undo_key))
				{
					board_.undo();
					chess_ = chess_ == Chess::Black ? Chess::White : Chess::Black;
					while(sf::Keyboard::isKeyPressed(undo_key))
						;
				}
				handle_cursor_move();

				if(handle_chess_place())
				{
					handle_over(cursor_position_);
					chess_ = chess_ == Chess::Black ? Chess::White : Chess::Black;
				}
			}

			render();
		}
	}

	void create_window()
	{
		const sf::Vector2u window_size(static_cast<unsigned>(board_.position().x) * 2 + (static_cast<unsigned>(board_.size().x) - 1) * 46,
		                               static_cast<unsigned>(board_.position().y) * 2 + (static_cast<unsigned>(board_.size().y) - 1) * 46);

		window_.create(sf::VideoMode{window_size.x, window_size.y}, "Gomoku", sf::Style::Close);
		window_.setFramerateLimit(60);
	}

	void reset()
	{
		status_          = Status::Initial;
		chess_           = Chess::Null;
		cursor_position_ = board_.size() / 2;
		board_.reset();
	}

	void draw_cursor()
	{
		sf::CircleShape cursor(40 / 2.f, 50);
		cursor.setOrigin(cursor.getRadius(), cursor.getRadius());
		cursor.setFillColor(sf::Color(255 / 2, 255 / 2, 255 / 2, 150));
		cursor.setPosition(board_.board_to_window_position(sf::Vector2f(cursor_position_)));
		window_.draw(cursor);
	}

	void handle_window_event()
	{
		for(auto event = sf::Event{}; window_.pollEvent(event);)
			if(event.type == sf::Event::Closed)
				window_.close();
	}

	void handle_over(const sf::Vector2i& position)
	{
		if(board_.is_full())
		{
			window_.clear(sf::Color(242, 208, 75));
			board_.draw(window_);
			window_.display();
			sf::sleep(sf::seconds(5.f));

			reset();
			return;
		}

		const auto chesses = board_.get_five_in_a_row(position);
		if(!chesses.has_value())
			return;

		const auto winner_chess = board_.get_chess(position);

		for(int i = 0; i < 10; i++)
		{
			for(const auto& pos : chesses.value())
				board_.place(pos, i % 2 == 0 ? Chess::Green : winner_chess);
			board_.place(position, i % 2 == 0 ? Chess::Green : winner_chess);

			window_.clear(sf::Color(242, 208, 75));
			board_.draw(window_);
			window_.display();
			sf::sleep(sf::seconds(0.5f));
		}

		reset();
	}

	void handle_mouse_input()
	{
		if(const auto result = board_.window_to_board_position(sf::Mouse::getPosition(window_)); result.has_value())
			cursor_position_ = result.value();
	}

	void handle_keyboard_input()
	{
		static sf::Clock clock;
		if(clock.getElapsedTime() < sf::seconds(0.2f))
			return;

		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			cursor_position_.y--;
			clock.restart();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			cursor_position_.y++;
			clock.restart();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		{
			cursor_position_.x--;
			clock.restart();
		}
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			cursor_position_.x++;
			clock.restart();
		}

		cursor_position_.x = std::clamp(cursor_position_.x, 0, board_.size().x - 1);
		cursor_position_.y = std::clamp(cursor_position_.y, 0, board_.size().y - 1);
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

		if((board_.window_to_board_position(sf::Mouse::getPosition(window_)).has_value() &&
		    sf::Mouse::isButtonPressed(sf::Mouse::Left)) ||
		   sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			clock.restart();

			if(board_.get_chess(cursor_position_) != Chess::Null)
				return false;

			if(chess_ == Chess::Null)
				chess_ = Chess::Black;

			board_.place(cursor_position_, chess_);

			return true;
		}
		return false;
	}

	void render()
	{
		window_.clear(sf::Color(242, 208, 75));
		board_.draw(window_);
		draw_cursor();
		window_.display();
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
		if(status == sf::Socket::Status::Done)
			return true;

		if(status == sf::Socket::Disconnected)
			throw std::runtime_error("the network connection has been lost");
		throw std::runtime_error("unknown network error");
	}

	Status           status_;
	Chess            chess_;
	sf::Vector2i     cursor_position_;
	Board            board_;
	sf::RenderWindow window_;
};
