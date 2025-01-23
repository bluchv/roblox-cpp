//
// Created by bluec on 1/17/2025.
//
#pragma once

#include <string>
#include <vector>

class Player {
public:
  Player() : Name(nullptr) {}
  ~Player() { delete[] Name; }

  char *Name;
};

namespace Roblox {
  namespace Services {
    class Players {
    public:
      static std::vector<Player *> GetPlayers() { return {}; }
    };
  } // namespace Services

  inline void print(const std::string &msg) {
    // Print
  };
} // namespace Roblox
