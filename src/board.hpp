// Copyright 2023-2025 ShenMian
// License(Apache-2.0)

#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>

enum class Piece { Empty, Black, White, Green };

class Board {
  public:
    Board(const sf::Vector2i& size = {15, 15}) : size_(size) {
        reset();
    }

    /**
	 * @brief 在指定位置下棋.
	 *
	 * @param position 落子位置.
	 * @param piece    棋子类型.
	 */
    auto place(const sf::Vector2i& position, Piece piece) -> void {
        board_[position.x][position.y] = piece;
        if (piece != Piece::Black && piece != Piece::White) {
            return;
        }
        histories_.push_back(position);
    }

    /**
	 * @brief 悔棋.
	 */
    auto undo() -> void {
        if (histories_.empty()) {
            return;
        }
        place(histories_.back(), Piece::Empty);
        histories_.pop_back();
    }

    /**
	 * @brief 棋盘是否已被下满.
	 */
    auto is_full() const noexcept -> bool {
        return histories_.size() == size_.x * size_.y;
    }

    /**
	 * @brief 返回连成一线的五子.
	 *
	 * @param position 最后落子位置.
	 *
	 * @return 若存在则返回连成一线的五子, 否则返回空.
	 */
    auto get_five_in_a_row() const -> std::optional<std::vector<sf::Vector2i>> {
        const auto last_position = histories_.back();
        const Piece piece = board_[last_position.x][last_position.y];

        const sf::Vector2i directions[8] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
            {-1, -1},
            {1, 1},
            {-1, 1},
            {1, -1}
        };
        for (size_t direction_index = 0; direction_index < 8;
             direction_index += 2) {
            std::vector<sf::Vector2i> pieces;

            auto add_pieces_on_direction = [&](const sf::Vector2i& direction) {
                for (int i = 1; i <= 4; i++) {
                    const sf::Vector2i pos = last_position + direction * i;
                    if (pos.x < 0 || pos.x >= size_.x || pos.y < 0
                        || pos.y >= size_.y) {
                        break;
                    }

                    if (board_[pos.x][pos.y] == piece) {
                        pieces.emplace_back(pos.x, pos.y);
                    } else {
                        break;
                    }
                }
            };

            add_pieces_on_direction(directions[direction_index]);
            add_pieces_on_direction(directions[direction_index + 1]);

            if (pieces.size() >= 4) {
                pieces.resize(4);
                pieces.push_back(last_position);
                return pieces;
            }
        }
        return std::nullopt;
    }

    /**
	 * @brief 将棋盘绘制在指定的窗口中.
	 *
	 * @param window 窗口.
	 */
    auto draw(sf::RenderWindow& window) const -> void {
        draw_board(window);
        draw_pieces(window);
        draw_mark(window);
    }

    /**
	 * @brief 重置棋盘.
	 */
    auto reset() -> void {
        board_.clear();
        board_.resize(size_.x);
        for (auto& row : board_) {
            row.resize(size_.y);
        }

        histories_.clear();
    }

    /**
	 * @brief 获取指定位置的棋子.
	 *
	 * @param position 要获取棋子的位置.
	 *
	 * @return 返回棋子.
	 */
    auto get_piece(const sf::Vector2i& position) const -> Piece {
        return board_[position.x][position.y];
    }

    /**
	 * @brief 将窗口坐标转为棋盘坐标.
	 *
	 * @return 返回对应的棋盘坐标, 若坐标在棋盘外则返回空.
	 */
    auto window_to_board_position(sf::Vector2i position) const
        -> std::optional<sf::Vector2i> {
        position.x = static_cast<int>(
            (position.x - position_.x) + (piece_offset_ / 2.f)
        );
        position.y = static_cast<int>(
            (position.y - position_.y) + (piece_offset_ / 2.f)
        );
        if (position.x < 0 || position.y < 0) {
            return std::nullopt;
        }

        position.x /= static_cast<int>(piece_offset_);
        position.y /= static_cast<int>(piece_offset_);
        if (position.x >= size_.x || position.y >= size_.y) {
            return std::nullopt;
        }

        return position;
    }

    /**
	 * @brief 将棋盘坐标转为窗口坐标.
	 *
	 * @return 返回对应的窗口坐标.
	 */
    auto board_to_window_position(const sf::Vector2f& position) const noexcept
        -> sf::Vector2f {
        return position_ + position * piece_offset_;
    }

    const auto& position() const noexcept {
        return position_;
    }

    const auto& size() const noexcept {
        return size_;
    }

  private:
    /**
	 * @brief 绘制棋盘.
	 */
    auto draw_board(sf::RenderWindow& window) const -> void {
        const float line_thickness = 2.f;

        sf::RectangleShape board_shape(
            {(size_.x - 1) * piece_offset_, (size_.y - 1) * piece_offset_}
        );
        board_shape.setFillColor(sf::Color(242, 208, 75));
        board_shape.setPosition(position_);
        window.draw(board_shape);

        sf::RectangleShape horizontal_line(
            {(size_.x - 1) * piece_offset_, line_thickness}
        );
        horizontal_line.setOrigin({0, horizontal_line.getSize().y / 2});
        horizontal_line.setFillColor(sf::Color::Black);
        for (int y = 0; y < size_.y; y++) {
            horizontal_line.setPosition(
                board_to_window_position({0.f, static_cast<float>(y)})
            );
            window.draw(horizontal_line);
        }

        sf::RectangleShape vertical_line(
            {line_thickness, (size_.y - 1) * piece_offset_}
        );
        vertical_line.setOrigin({vertical_line.getSize().x / 2, 0});
        vertical_line.setFillColor(sf::Color::Black);
        for (int x = 0; x < size_.x; x++) {
            vertical_line.setPosition(
                board_to_window_position({static_cast<float>(x), 0.f})
            );
            window.draw(vertical_line);
        }

        draw_stars(window);
    }

    /**
	 * @brief 绘制天元和星.
	 */
    auto draw_stars(sf::RenderWindow& window) const -> void {
        sf::CircleShape star(5.f, 10);
        star.setOrigin({star.getRadius(), star.getRadius()});
        star.setFillColor(sf::Color::Black);

        // 绘制星
        star.setPosition(board_to_window_position({3.f, 3.f}));
        window.draw(star);

        star.setPosition(board_to_window_position({size_.x - 4.f, 3.f}));
        window.draw(star);

        star.setPosition(board_to_window_position({3.f, size_.y - 4.f}));
        window.draw(star);

        star.setPosition(board_to_window_position({size_.x - 4.f, size_.y - 4.f}
        ));
        window.draw(star);

        // 绘制天元
        star.setPosition(board_to_window_position(
            {(size_.x - 1.f) / 2.f, (size_.y - 1.f) / 2.f}
        ));
        window.draw(star);
    }

    auto draw_pieces(sf::RenderWindow& window) const -> void {
        for (size_t y = 0; y < board_.size(); y++) {
            for (size_t x = 0; x < board_[0].size(); x++) {
                if (board_[x][y] != Piece::Empty) {
                    draw_piece(window, sf::Vector2i(x, y), board_[x][y]);
                }
            }
        }
    }

    /**
	 * @brief 绘制棋子.
	 *
	 * @param window   要绘制的窗口.
	 * @param position 棋子的位置.
	 * @param piece    棋子.
	 */
    auto draw_piece(
        sf::RenderWindow& window,
        const sf::Vector2i& position,
        Piece piece
    ) const -> void {
        sf::CircleShape piece_shape(piece_diameter_ / 2.f, 50);
        piece_shape.setOrigin({piece_shape.getRadius(), piece_shape.getRadius()}
        );
        piece_shape.setPosition(board_to_window_position(sf::Vector2f(position))
        );

        switch (piece) {
            case Piece::White:
                piece_shape.setFillColor(sf::Color(230, 230, 230));
                break;

            case Piece::Black:
                piece_shape.setFillColor(sf::Color::Black);
                break;

            case Piece::Green:
                piece_shape.setFillColor(sf::Color::Green);
                break;

            default:
                throw std::logic_error("");
        }

        window.draw(piece_shape);
    }

    /**
	 * @brief 绘制最后落子标记.
	 *
	 * @param window 要绘制的窗口.
	 */
    auto draw_mark(sf::RenderWindow& window) const -> void {
        if (histories_.empty()) {
            return;
        }
        if (histories_.back().x < 0 || histories_.back().y < 0) {
            return;
        }
        sf::CircleShape mark(piece_diameter_ / 4.f / 2.f, 3);
        mark.setOrigin({mark.getRadius(), mark.getRadius()});
        mark.setPosition(board_to_window_position(sf::Vector2f(histories_.back()
        )));
        mark.setFillColor(sf::Color::Red);
        window.draw(mark);
    }

    std::vector<std::vector<Piece>> board_;
    std::vector<sf::Vector2i> histories_;

    const float piece_diameter_ = 40.f;
    const float piece_spacing_ = 6.f;
    const float piece_offset_ = piece_diameter_ + piece_spacing_;

    const sf::Vector2i size_;
    const sf::Vector2f position_ = {piece_offset_ + 10.f, piece_offset_ + 10.f};
};
