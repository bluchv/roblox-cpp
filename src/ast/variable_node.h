#pragma once

#include <string>
#include "luau_node.h"

namespace roblox_transpiler {
  class VariableNode : public LuauNode {
  public:
    VariableNode(std::string name, Value value, std::string type = "");

    void render(LuauCodegen &codegen) const override;

    const std::string &getName() const { return name_; }
    const Value &getValue() const { return value_; }
    const std::string &getType() const { return type_; }

  private:
    std::string name_;
    Value value_;
    std::string type_;
  };

  class FunctionCallNode : public LuauNode {
  public:
    explicit FunctionCallNode(std::string functionName);

    void addArgument(const Value &arg);
    void addArguments(const std::vector<Value> &args);

    void render(LuauCodegen &codegen) const override;

    const std::string &getFunctionName() const { return functionName_; }
    const std::vector<Value> &getArguments() const { return arguments_; }

  private:
    std::string functionName_;
    std::vector<Value> arguments_;
  };
} // namespace roblox_transpiler
