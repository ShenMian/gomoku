#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <optional>

const float chess_diameter = 40;
const float chess_spacing = 6;
const float board_line_thickness = 2.f;
const float chess_offset = chess_diameter + chess_spacing;

enum class Chess {
	Null,
	Black,
	White,
	Green
};

std::vector<std::vector<Chess>> board;

void draw_chess(sf::RenderWindow& window, sf::Vector2i position, Chess chess)
{
	sf::CircleShape chess_shape(chess_diameter / 2.f, 30);
	chess_shape.setOrigin(chess_diameter / 2.f, chess_diameter / 2.f);
	chess_shape.setPosition(chess_offset + position.x * chess_offset, chess_offset + position.y * chess_offset);

	switch (chess)
	{
	case Chess::White:
		chess_shape.setFillColor(sf::Color::White);
		break;

	case Chess::Black:
		chess_shape.setFillColor(sf::Color::Black);
		break;

	case Chess::Green:
		chess_shape.setFillColor(sf::Color::Green);
		break;
	}

	window.draw(chess_shape);
}

void draw_board(sf::RenderWindow& window)
{
	for (int y = 1; y <= 15; y++)
	{
		sf::RectangleShape line(sf::Vector2f(14.f * chess_offset, board_line_thickness));
		line.setPosition(chess_offset, y * chess_offset);
		line.setFillColor(sf::Color::Black);
		window.draw(line);
	}

	for (int x = 1; x <= 15; x++)
	{
		sf::RectangleShape line(sf::Vector2f(board_line_thickness, 14.f * chess_offset));
		line.setPosition(x * chess_offset, chess_offset);
		line.setFillColor(sf::Color::Black);
		window.draw(line);
	}

	for (size_t y = 0; y < board.size(); y++)
		for (size_t x = 0; x < board[0].size(); x++)
			if (board[x][y] != Chess::Null)
				draw_chess(window, sf::Vector2i(x, y), board[x][y]);
}

std::optional<std::vector<sf::Vector2i>> get_five_chesses_in_a_row(sf::Vector2i position)
{
	std::vector<sf::Vector2i> chesses;
	const Chess chess = board[position.x][position.y];

	for (int x = std::max(position.x - 4, 0); x < 15; x++)
	{
		if (board[x][position.y] == chess)
			chesses.emplace_back(x, position.y);
		if (chesses.size() == 5)
			return chesses;
	}
	chesses.clear();

	for (int y = std::max(position.y - 4, 0); y < 15; y++)
	{
		if (board[position.x][y] == chess)
			chesses.emplace_back(position.x, y);
		if (chesses.size() == 5)
			return chesses;
	}
	chesses.clear();

	{
		int x = position.x, y = position.y;
		while (x > 0 && y > 0)
			x--, y--;
		for (; x < 15 && y < 15; x++, y++)
		{
			if (board[x][y] == chess)
				chesses.emplace_back(x, y);
			if (chesses.size() == 5)
				return chesses;
		}
		chesses.clear();
	}

	{
		int x = position.x, y = position.y;
		while (x < 14 && y > 0)
			x++, y--;
		for (; x >= 0 && y < 15; x--, y++)
		{
			if (board[x][y] == chess)
				chesses.emplace_back(x, y);
			if (chesses.size() == 5)
				return chesses;
		}
	}

	return std::nullopt;
}

int main()
{
	const unsigned int size = chess_offset * (14 + 2);
	auto window = sf::RenderWindow{ { size, size }, "Gomoku" };
	window.setSize({ size, size });
	window.setFramerateLimit(144);

	int index = 0;

	board.resize(15);
	for (auto& row : board)
		row.resize(15);

	while (window.isOpen())
	{
		for (auto event = sf::Event{}; window.pollEvent(event);)
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			sf::Vector2i position = sf::Mouse::getPosition(window);
			position.x = ((position.x - chess_offset) + (chess_offset / 2.f)) / chess_offset;
			position.y = ((position.y - chess_offset) + (chess_offset / 2.f)) / chess_offset;

			if (position.x < 0 || position.x >= 15 || position.y < 0 || position.y >= 15)
				continue;

			if (board[position.x][position.y] != Chess::Null)
				continue;

			const Chess chess = index % 2 == 0 ? Chess::Black : Chess::White;
			board[position.x][position.y] = chess;
			index++;

			auto chesses = get_five_chesses_in_a_row(position);
			if (chesses.has_value())
			{
				while (true)
				{
					for (auto& position : chesses.value())
						board[position.x][position.y] = Chess::Green;

					window.clear(sf::Color(242, 208, 75));
					draw_board(window);
					window.display();
					sf::sleep(sf::seconds(0.5f));

					for (auto& position : chesses.value())
						board[position.x][position.y] = chess;

					window.clear(sf::Color(242, 208, 75));
					draw_board(window);
					window.display();
					sf::sleep(sf::seconds(0.5f));
				}
			}

			sf::sleep(sf::seconds(0.2f));
		}

		window.clear(sf::Color(242, 208, 75));
		draw_board(window);
		window.display();
	}
}
