#include "board.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>

enum class Status { Ready, Wait, Initial };

Status status = Status::Initial;
Chess chess = Chess::Null;

Board board;

sf::Vector2i normalize(const sf::Vector2i &v) {
  const float norm = sqrt((v.x * v.x) + (v.y * v.y));
  if (norm == 0)
    return v;
  return sf::Vector2i(v.x / norm, v.y / norm);
}

void reset() {
  status = Status::Initial;
  chess = Chess::Null;
  board.reset();
}

void place_chess(sf::RenderWindow &window, sf::Vector2i position, Chess chess) {
  board.place_chess(position, chess);

  if (board.is_full()) {
    sf::sleep(sf::seconds(5.f));
    reset();
    return;
  }

  const auto chesses = board.get_five_chesses_in_a_row(position);
  if (!chesses.has_value())
    return;

  for (int i = 0; i < 10; i++) {
    for (const auto &position : chesses.value())
      board.place_chess(position, i % 2 == 0 ? Chess::Green : chess);

    window.clear(sf::Color(242, 208, 75));
    board.draw(window);
    window.display();
    sf::sleep(sf::seconds(0.5f));
  }

  reset();
}

int main() {
  std::cout << R"(
  _____                __       
 / ___/__  __ _  ___  / /____ __
/ (_ / _ \/  ' \/ _ \/  '_/ // /
\___/\___/_/_/_/\___/_/\_\\_,_/ 
                                
          1. client
          2. server

)";
  std::string choice;
  std::getline(std::cin, choice);

  sf::TcpSocket socket;

  if (choice == "1") {
    std::cout << "host ip: ";
    std::string ip;
    std::cin >> ip;
    while (socket.connect(ip, 1234) != sf::Socket::Status::Done)
      std::cout << "retrying...\n";
  } else if (choice == "2") {
    std::cout << "local ip: " << sf::IpAddress::getLocalAddress() << "\n";
    std::cout << "waiting for connections...\n";
    sf::TcpListener listener;
    listener.listen(1234);
    assert(listener.accept(socket) == sf::Socket::Status::Done);
  } else {
    return 1;
  }
  socket.setBlocking(false);

  const sf::Vector2u size(736u, 736u);
  auto window = sf::RenderWindow{{size.x, size.y}, "Gomoku"};
  window.setSize(size);
  window.setFramerateLimit(60);

  reset();

  while (window.isOpen()) {
    for (auto event = sf::Event{}; window.pollEvent(event);) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    if (status == Status::Ready || status == Status::Initial) {
      if (window.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2i position = sf::Mouse::getPosition(window);

        position.x = static_cast<int>((position.x - 46) + (46 / 2.f));
        position.y = static_cast<int>((position.y - 46) + (46 / 2.f));
        if (position.x < 0 || position.y < 0)
          continue;

        position.x /= static_cast<int>(46);
        position.y /= static_cast<int>(46);
        if (position.x >= board.size().x || position.y >= board.size().y)
          continue;

        if (board.get_chess(position) != Chess::Null)
          continue;

        if (chess == Chess::Null)
          chess = Chess::Black;

        sf::Packet packet;
        packet.append(&position, sizeof(position));
        assert(socket.send(packet) == sf::Socket::Status::Done);
        status = Status::Wait;
        place_chess(window, position, chess);
      }
    }

    if (status == Status::Wait || status == Status::Initial) {
      sf::Packet packet;
      socket.receive(packet);
      if (!packet.endOfPacket()) {
        const sf::Vector2i position =
            *static_cast<const sf::Vector2i *>(packet.getData());
        assert(packet.getDataSize() == sizeof(sf::Vector2i));
        if (chess == Chess::Null)
          chess = Chess::White;
        status = Status::Ready;
        place_chess(window, position,
                    chess == Chess::Black ? Chess::White : Chess::Black);
      }
    }

    window.clear(sf::Color(242, 208, 75));
    board.draw(window);
    window.display();
  }
}
