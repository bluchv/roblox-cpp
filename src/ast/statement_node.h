#pragma once

#include <string>
#include "luau_node.h"

namespace roblox_transpiler {
  class AssignmentNode : public LuauNode {
  public:
    AssignmentNode(std::string variable, Value value);

    void render(LuauCodegen &codegen) const override;

    const std::string &getVariable() const { return variable_; }
    const Value &getValue() const { return value_; }

  private:
    std::string variable_;
    Value value_;
  };

  class ReturnNode : public LuauNode {
  public:
    explicit ReturnNode(Value value = std::string(""));

    void render(LuauCodegen &codegen) const override;

  private:
    Value value_;
    bool hasValue_;
  };
} // namespace roblox_transpiler
