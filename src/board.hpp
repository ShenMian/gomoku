// Copyright 2023 ShenMian
// License(Apache-2.0)

#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

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
	Board(const sf::Vector2i& size = {15, 15}) : size_(size) { reset(); }

	/**
	 * @brief 在指定位置下棋.
	 *
	 * @param position 落子位置.
	 * @param chess    棋子类型.
	 */
	void place(const sf::Vector2i& position, Chess chess)
	{
		board_[position.x][position.y] = chess;
		if(chess != Chess::Black && chess != Chess::White)
			return;
		histories_.push_back(position);
	}

	/**
	 * @brief 悔棋.
	 */
	void undo()
	{
		if(histories_.empty())
			return;
		place(histories_.back(), Chess::Null);
		histories_.pop_back();
	}

	/**
	 * @brief 棋盘是否已被下满.
	 */
	bool is_full() const noexcept { return histories_.size() == size_.x * size_.y; }

	/**
	 * @brief 返回连成一线的五子.
	 *
	 * @param position 最后落子位置.
	 *
	 * @return 若存在则返回连成一线的五子, 否则返回空.
	 */
	std::optional<std::vector<sf::Vector2i>> get_five_in_a_row(const sf::Vector2i& position) const
	{
		const Chess chess = board_[position.x][position.y];

		const sf::Vector2i directions[8] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}};
		for(int direction_index = 0; direction_index < 8; direction_index += 2)
		{
			std::vector<sf::Vector2i> chesses;

			auto add_chesses_on_direction = [&](const sf::Vector2i& direction) {
				for(int i = 1; i <= 4; i++)
				{
					const sf::Vector2i pos = position + direction * i;
					if(pos.x < 0 || pos.x >= size_.x || pos.y < 0 || pos.y >= size_.y)
						break;

					if(board_[pos.x][pos.y] == chess)
						chesses.emplace_back(pos.x, pos.y);
					else
						break;
				}
			};

			add_chesses_on_direction(directions[direction_index]);
			add_chesses_on_direction(directions[direction_index + 1]);

			if(chesses.size() >= 4)
			{
				chesses.resize(4);
				chesses.push_back(position);
				return chesses;
			}
		}
		return std::nullopt;
	}

	/**
	 * @brief 将棋盘绘制在指定的窗口中.
	 *
	 * @param window 窗口.
	 */
	void draw(sf::RenderWindow& window) const
	{
		draw_board(window);
		draw_chesses(window);
		draw_mark(window);
	}

	/**
	 * @brief 重置棋盘.
	 */
	void reset()
	{
		board_.clear();
		board_.resize(size_.x);
		for(auto& row : board_)
			row.resize(size_.y);

		histories_.clear();
	}

	/**
	 * @brief 获取指定位置的棋子.
	 *
	 * @param position 要获取棋子的位置.
	 *
	 * @return 返回棋子.
	 */
	Chess get_chess(const sf::Vector2i& position) const { return board_[position.x][position.y]; }

	/**
	 * @brief 将窗口坐标转为棋盘坐标.
	 *
	 * @return 返回对应的棋盘坐标, 若坐标在棋盘外则返回空.
	 */
	std::optional<sf::Vector2i> window_to_board_position(sf::Vector2i position) const
	{
		position.x = static_cast<int>((position.x - position_.x) + (chess_offset_ / 2.f));
		position.y = static_cast<int>((position.y - position_.y) + (chess_offset_ / 2.f));
		if(position.x < 0 || position.y < 0)
			return std::nullopt;

		position.x /= static_cast<int>(chess_offset_);
		position.y /= static_cast<int>(chess_offset_);
		if(position.x >= size_.x || position.y >= size_.y)
			return std::nullopt;

		return position;
	}

	/**
	 * @brief 将棋盘坐标转为窗口坐标.
	 *
	 * @return 返回对应的窗口坐标.
	 */
	sf::Vector2f board_to_window_position(const sf::Vector2f& position) const noexcept
	{
		return {position_.x + position.x * chess_offset_, position_.y + position.y * chess_offset_};
	}

	const auto& position() const noexcept { return position_; }
	const auto& size() const noexcept { return size_; }

private:
	/**
	 * @brief 绘制棋盘.
	 */
	void draw_board(sf::RenderWindow& window) const
	{
		const float line_thickness = 2.f;

		sf::RectangleShape board_shape({(size_.x - 1) * chess_offset_, (size_.y - 1) * chess_offset_});
		board_shape.setFillColor(sf::Color(242, 208, 75));
		board_shape.setPosition(position_);
		window.draw(board_shape);

		sf::RectangleShape horizontal_line({(size_.x - 1) * chess_offset_, line_thickness});
		horizontal_line.setOrigin(0, horizontal_line.getSize().y / 2);
		horizontal_line.setFillColor(sf::Color::Black);
		for(int y = 0; y < size_.y; y++)
		{
			horizontal_line.setPosition(board_to_window_position({0.f, static_cast<float>(y)}));
			window.draw(horizontal_line);
		}

		sf::RectangleShape vertical_line({line_thickness, (size_.y - 1) * chess_offset_});
		vertical_line.setOrigin(vertical_line.getSize().x / 2, 0);
		vertical_line.setFillColor(sf::Color::Black);
		for(int x = 0; x < size_.x; x++)
		{
			vertical_line.setPosition(board_to_window_position({static_cast<float>(x), 0.f}));
			window.draw(vertical_line);
		}

		draw_stars(window);
	}

	/**
	 * @brief 绘制天元和星.
	 */
	void draw_stars(sf::RenderWindow& window) const
	{
		sf::CircleShape star(5.f, 10);
		star.setOrigin(star.getRadius(), star.getRadius());
		star.setFillColor(sf::Color::Black);

		// 绘制星
		star.setPosition(board_to_window_position({3.f, 3.f}));
		window.draw(star);

		star.setPosition(board_to_window_position({size_.x - 4.f, 3.f}));
		window.draw(star);

		star.setPosition(board_to_window_position({3.f, size_.y - 4.f}));
		window.draw(star);

		star.setPosition(board_to_window_position({size_.x - 4.f, size_.y - 4.f}));
		window.draw(star);

		// 绘制天元
		star.setPosition(board_to_window_position({(size_.x - 1.f) / 2.f, (size_.y - 1.f) / 2.f}));
		window.draw(star);
	}

	void draw_chesses(sf::RenderWindow& window) const
	{
		for(int y = 0; y < board_.size(); y++)
			for(int x = 0; x < board_[0].size(); x++)
				if(board_[x][y] != Chess::Null)
					draw_chess(window, {x, y}, board_[x][y]);
	}

	/**
	 * @brief 绘制棋子.
	 *
	 * @param window   要绘制的窗口.
	 * @param position 棋子的位置.
	 * @param chess    棋子.
	 */
	void draw_chess(sf::RenderWindow& window, const sf::Vector2i& position, Chess chess) const
	{
		sf::CircleShape chess_shape(chess_diameter_ / 2.f, 50);
		chess_shape.setOrigin(chess_shape.getRadius(), chess_shape.getRadius());
		chess_shape.setPosition(board_to_window_position({static_cast<float>(position.x), static_cast<float>(position.y)}));

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
			throw std::logic_error("");
		}

		window.draw(chess_shape);
	}

	/**
	 * @brief 绘制最后落子标记.
	 *
	 * @param window 要绘制的窗口.
	 */
	void draw_mark(sf::RenderWindow& window) const
	{
		if(histories_.empty())
			return;
		if(histories_.back().x < 0 || histories_.back().y < 0)
			return;
		sf::CircleShape mark(chess_diameter_ / 4.f / 2.f, 3);
		mark.setOrigin(mark.getRadius(), mark.getRadius());
		mark.setPosition(board_to_window_position(sf::Vector2f(histories_.back())));
		mark.setFillColor(sf::Color::Red);
		window.draw(mark);
	}

	std::vector<std::vector<Chess>> board_;
	std::vector<sf::Vector2i>       histories_;

	const float chess_diameter_ = 40.f;
	const float chess_spacing_  = 6.f;
	const float chess_offset_   = chess_diameter_ + chess_spacing_;

	const sf::Vector2i size_;
	const sf::Vector2f position_ = {chess_offset_ + 10.f, chess_offset_ + 10.f};
};
