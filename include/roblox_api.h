#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Roblox {
  class Player {
  public:
    Player(const std::string &name) : name_(name) {}
    const std::string &getName() const { return name_; }
    void setName(const std::string &name) { name_ = name; }
    void Kick(const std::string &reason) {}

  private:
    std::string name_;
  };

  namespace Services {
    class Players {
    public:
      static std::vector<std::shared_ptr<Player>> getPlayers() { return {}; }

      static std::shared_ptr<Player> getLocalPlayer() { return std::make_shared<Player>("LocalPlayer"); }
    };

    class Workspace {
    public:
      static void print(const std::string &message) {
        // Implementation for Roblox print
      }
    };
  } // namespace Services

  // Global functions that map to Luau
  inline void print(const std::string &message) { Services::Workspace::print(message); }
} // namespace Roblox
