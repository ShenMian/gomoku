#pragma once

#include <SFML/Graphics.hpp>
#include <cassert>
#include <optional>
#include <tuple>

enum class Chess
{
	Null,
	Black,
	White,
	Green
};

class Board
{
public:
	Board(sf::Vector2i size = {15, 15}) : size_(size), max_steps_(size.x * size.y) { reset(); }

	void draw(sf::RenderWindow& window)
	{
		sf::RectangleShape horizontal_line(sf::Vector2f((size_.x - 1) * chess_offset_, line_thickness_));
		horizontal_line.setOrigin(0, horizontal_line.getSize().y / 2);
		horizontal_line.setFillColor(sf::Color::Black);
		for(int y = 1; y <= size_.y; y++)
		{
			horizontal_line.setPosition(chess_offset_, y * chess_offset_);
			window.draw(horizontal_line);
		}

		sf::RectangleShape vertical_line(sf::Vector2f(line_thickness_, (size_.y - 1) * chess_offset_));
		vertical_line.setOrigin(vertical_line.getSize().x / 2, 0);
		vertical_line.setFillColor(sf::Color::Black);
		for(int x = 1; x <= size_.x; x++)
		{
			vertical_line.setPosition(x * chess_offset_, chess_offset_);
			window.draw(vertical_line);
		}

		draw_stars(window);

		for(int y = 0; y < board.size(); y++)
			for(int x = 0; x < board[0].size(); x++)
				if(board[x][y] != Chess::Null)
					draw_chess(window, {x, y}, board[x][y]);

		draw_mark(window);
	}

	void place_chess(sf::Vector2i position, Chess chess)
	{
		board[position.x][position.y] = chess;
		last_place_position_          = position;
		steps_++;
	}

	bool is_full() const noexcept { return steps_ == max_steps_; }

	std::optional<std::vector<sf::Vector2i>> get_five_chesses_in_a_row(sf::Vector2i position)
	{
		std::vector<sf::Vector2i> chesses;
		const Chess               chess = board[position.x][position.y];

		for(int x = std::max(position.x - 4, 0); x < size_.x; x++)
		{
			if(board[x][position.y] == chess)
				chesses.emplace_back(x, position.y);
			else
				chesses.clear();
			if(chesses.size() == 5)
				return chesses;
		}
		chesses.clear();

		for(int y = std::max(position.y - 4, 0); y < size_.y; y++)
		{
			if(board[position.x][y] == chess)
				chesses.emplace_back(position.x, y);
			else
				chesses.clear();
			if(chesses.size() == 5)
				return chesses;
		}
		chesses.clear();

		{
			int x = position.x, y = position.y;
			while(x > 0 && y > 0)
				x--, y--;
			for(; x < size_.x && y < size_.y; x++, y++)
			{
				if(board[x][y] == chess)
					chesses.emplace_back(x, y);
				else
					chesses.clear();
				if(chesses.size() == 5)
					return chesses;
			}
			chesses.clear();
		}

		{
			int x = position.x, y = position.y;
			while(x < size_.x - 1 && y > 0)
				x++, y--;
			for(; x >= 0 && y < size_.y; x--, y++)
			{
				if(board[x][y] == chess)
					chesses.emplace_back(x, y);
				else
					chesses.clear();
				if(chesses.size() == 5)
					return chesses;
			}
		}

		return std::nullopt;
	}

	Chess get_chess(sf::Vector2i position) const { return board[position.x][position.y]; }

	void reset()
	{
		board.clear();
		board.resize(size_.x);
		for(auto& row : board)
			row.resize(size_.y);

		steps_               = 1;
		last_place_position_ = {-9, -9};
	}

	const sf::Vector2i& size() const noexcept { return size_; }

private:
	void draw_mark(sf::RenderWindow& window)
	{
		sf::CircleShape mark(chess_diameter_ / 4.f / 2.f, 3);
		mark.setOrigin(mark.getRadius(), mark.getRadius());
		mark.setPosition(chess_offset_ + last_place_position_.x * chess_offset_,
		                 chess_offset_ + last_place_position_.y * chess_offset_);
		mark.setFillColor(sf::Color::Red);
		window.draw(mark);
	}

	void draw_chess(sf::RenderWindow& window, sf::Vector2i position, Chess chess)
	{
		sf::CircleShape chess_shape(chess_diameter_ / 2.f, 50);
		chess_shape.setOrigin(chess_shape.getRadius(), chess_shape.getRadius());
		chess_shape.setPosition(chess_offset_ + position.x * chess_offset_, chess_offset_ + position.y * chess_offset_);

		switch(chess)
		{
		case Chess::White:
			chess_shape.setFillColor(sf::Color(230, 230, 230));
			break;

		case Chess::Black:
			chess_shape.setFillColor(sf::Color::Black);
			break;

		case Chess::Green:
			chess_shape.setFillColor(sf::Color::Green);
			break;

		default:
			assert(false);
		}

		window.draw(chess_shape);
	}

	void draw_stars(sf::RenderWindow& window)
	{
		sf::CircleShape star(5.f, 10);
		star.setOrigin(star.getRadius(), star.getRadius());
		star.setFillColor(sf::Color::Black);

		star.setPosition(3 * chess_offset_ + chess_offset_, 3 * chess_offset_ + chess_offset_);
		window.draw(star);

		star.setPosition((size_.x - 4) * chess_offset_ + chess_offset_, 3 * chess_offset_ + chess_offset_);
		window.draw(star);

		star.setPosition(3 * chess_offset_ + chess_offset_, (size_.y - 4) * chess_offset_ + chess_offset_);
		window.draw(star);

		star.setPosition((size_.x - 4) * chess_offset_ + chess_offset_, (size_.y - 4) * chess_offset_ + chess_offset_);
		window.draw(star);

		star.setPosition((size_.x - 1) / 2.f * chess_offset_ + chess_offset_,
		                 (size_.y - 1) / 2.f * chess_offset_ + chess_offset_);
		window.draw(star);
	}

	std::vector<std::vector<Chess>> board;

	const sf::Vector2i size_;
	const float        line_thickness_ = 2.f;
	const int          max_steps_;

	const float chess_diameter_ = 40;
	const float chess_spacing_  = 6;
	const float chess_offset_   = chess_diameter_ + chess_spacing_;

	int          steps_;
	sf::Vector2i last_place_position_;
};
