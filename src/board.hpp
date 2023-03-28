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
	Board(sf::Vector2i size = {4, 4}) : size_(size), max_steps_(size.x * size.y) { reset(); }

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
	 * @brief 在指定位置下棋.
	 *
	 * @param position 落子位置.
	 * @param chess    棋子类型.
	 */
	void place_chess(sf::Vector2i position, Chess chess)
	{
		board[position.x][position.y] = chess;
		if(chess != Chess::Black && chess != Chess::White)
			return;
		last_place_position_ = position;
		steps_++;
	}

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
	 * @brief 棋盘是否已被下满.
	 */
	bool is_full() const noexcept { return steps_ == max_steps_; }

	/**
	 * @brief 返回连成一线的五子.
	 *
	 * @param position 最后落子位置.
	 *
	 * @return 若存在则返回连成一线的五子, 否则返回空.
	 */
	std::optional<std::vector<sf::Vector2i>> get_five_chesses_in_a_row(sf::Vector2i position)
	{
		const Chess chess = board[position.x][position.y];

		sf::Vector2i directions[8] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}};
		for(int i = 0; i < 8; i += 2)
		{
			std::vector<sf::Vector2i> chesses;

			auto add_chesses_on_direction = [&](sf::Vector2i direction) {
				for(int j = 1; j <= 4; j++)
				{
					const sf::Vector2i pos = position + direction * j;
					if(pos.x < 0 || pos.x >= size_.x || pos.y < 0 || pos.y >= size_.y)
						break;

					if(board[pos.x][pos.y] == chess)
						chesses.emplace_back(pos.x, pos.y);
					else
						break;
				}
			};

			add_chesses_on_direction(directions[i]);
			add_chesses_on_direction(directions[i + 1]);

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
	 * @brief 获取指定位置的棋子.
	 *
	 * @param position 要获取棋子的位置.
	 *
	 * @return 返回棋子.
	 */
	Chess get_chess(sf::Vector2i position) const { return board[position.x][position.y]; }

	/**
	 * @brief 重置棋盘.
	 */
	void reset()
	{
		board.clear();
		board.resize(size_.x);
		for(auto& row : board)
			row.resize(size_.y);

		steps_               = 0;
		last_place_position_ = {-1, -1};
	}

	const auto& position() const noexcept { return position_; }
	const auto& size() const noexcept { return size_; }

private:
	/**
	 * @brief 绘制棋盘.
	 */
	void draw_board(sf::RenderWindow& window) const
	{
		sf::RectangleShape board_shape({(size_.x - 1) * chess_offset_, (size_.y - 1) * chess_offset_});
		board_shape.setFillColor(sf::Color(242, 208, 75));
		board_shape.setPosition(position_);
		window.draw(board_shape);

		sf::RectangleShape horizontal_line(sf::Vector2f((size_.x - 1) * chess_offset_, line_thickness_));
		horizontal_line.setOrigin(0, horizontal_line.getSize().y / 2);
		horizontal_line.setFillColor(sf::Color::Black);
		for(int y = 0; y < size_.y; y++)
		{
			horizontal_line.setPosition(position_.x, y * chess_offset_ + position_.y);
			window.draw(horizontal_line);
		}

		sf::RectangleShape vertical_line(sf::Vector2f(line_thickness_, (size_.y - 1) * chess_offset_));
		vertical_line.setOrigin(vertical_line.getSize().x / 2, 0);
		vertical_line.setFillColor(sf::Color::Black);
		for(int x = 0; x < size_.x; x++)
		{
			vertical_line.setPosition(x * chess_offset_ + position_.x, position_.y);
			window.draw(vertical_line);
		}

		draw_stars(window);
	}

	void draw_chesses(sf::RenderWindow& window) const
	{
		for(int y = 0; y < board.size(); y++)
			for(int x = 0; x < board[0].size(); x++)
				if(board[x][y] != Chess::Null)
					draw_chess(window, {x, y}, board[x][y]);
	}

	/**
	 * @brief 绘制最后落子标记.
	 * 
	 * @param window 要绘制的窗口.
	 */
	void draw_mark(sf::RenderWindow& window) const
	{
		if(last_place_position_.x < 0 || last_place_position_.y < 0)
			return;
		sf::CircleShape mark(chess_diameter_ / 4.f / 2.f, 3);
		mark.setOrigin(mark.getRadius(), mark.getRadius());
		mark.setPosition(position_.x + last_place_position_.x * chess_offset_,
		                 position_.y + last_place_position_.y * chess_offset_);
		mark.setFillColor(sf::Color::Red);
		window.draw(mark);
	}

	/**
	 * @brief 绘制棋子.
	 *
	 * @param window   要绘制的窗口.
	 * @param position 棋子的位置.
	 * @param chess    棋子.
	 */
	void draw_chess(sf::RenderWindow& window, sf::Vector2i position, Chess chess) const
	{
		sf::CircleShape chess_shape(chess_diameter_ / 2.f, 50);
		chess_shape.setOrigin(chess_shape.getRadius(), chess_shape.getRadius());
		chess_shape.setPosition(position_.x + position.x * chess_offset_, position_.y + position.y * chess_offset_);

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
	 * @brief 绘制天元和星.
	 */
	void draw_stars(sf::RenderWindow& window) const
	{
		sf::CircleShape star(5.f, 10);
		star.setOrigin(star.getRadius(), star.getRadius());
		star.setFillColor(sf::Color::Black);

		// 绘制星
		star.setPosition(3 * chess_offset_ + position_.x, 3 * chess_offset_ + position_.y);
		window.draw(star);

		star.setPosition((size_.x - 4) * chess_offset_ + position_.x, 3 * chess_offset_ + position_.y);
		window.draw(star);

		star.setPosition(3 * chess_offset_ + position_.x, (size_.y - 4) * chess_offset_ + position_.y);
		window.draw(star);

		star.setPosition((size_.x - 4) * chess_offset_ + position_.x, (size_.y - 4) * chess_offset_ + position_.y);
		window.draw(star);

		// 绘制天元
		star.setPosition((size_.x - 1) / 2.f * chess_offset_ + position_.x,
		                 (size_.y - 1) / 2.f * chess_offset_ + position_.y);
		window.draw(star);
	}

	std::vector<std::vector<Chess>> board;

	const float chess_diameter_ = 40.f;
	const float chess_spacing_  = 6.f;
	const float chess_offset_   = chess_diameter_ + chess_spacing_;

	const sf::Vector2i size_;
	const sf::Vector2f position_ = {chess_offset_ + 10.f, chess_offset_ + 10.f};

	const float  line_thickness_ = 2.f;
	const int    max_steps_;
	int          steps_;
	sf::Vector2i last_place_position_;
};
