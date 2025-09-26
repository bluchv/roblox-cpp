#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace roblox_transpiler {
  // Forward declaration
  class LuauCodegen;

  using Value = std::variant<std::string, int, double, bool>;

  class LuauNode {
  public:
    virtual ~LuauNode() = default;

    void addChild(std::unique_ptr<LuauNode> child);
    virtual void render(LuauCodegen &codegen) const;

    void setParent(LuauNode *parent) { parent_ = parent; }
    LuauNode *getParent() const { return parent_; }

    const std::vector<std::unique_ptr<LuauNode>> &getChildren() const { return children_; }

  protected:
    std::vector<std::unique_ptr<LuauNode>> children_;
    LuauNode *parent_ = nullptr;
  };
} // namespace roblox_transpiler
