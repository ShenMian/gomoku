# Gomoku

[![CodeFactor](https://www.codefactor.io/repository/github/shenmian/gomoku/badge)](https://www.codefactor.io/repository/github/shenmian/gomoku)

A simple gomoku, using [SFML] framework, supports mouse/keyboard input and LAN connection.  

## Rules

The rules are **free-style** and there are no forbidden moves. Players can decide the order of play in each round.  

## Modes

- Online: one person acts as a server and the other as a client to play games online.
- Offline: playing games in turns with two players in an offline environment.

## Online mode

1. Player A selects the server under the online mode, which will display Player A's local IP address.
2. Player B selects the client under the online mode and enters Player A's local IP address.
3. The game begins and players decide the order of play themselves. The first player is black and the second player is white.

## Screenshot

<p align="center"><img src="docs/screenshot.png" width=50% height=50%></p>

[SFML]: https://github.com/SFML/SFML
