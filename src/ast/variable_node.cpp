#include "variable_node.h"
#include "../codegen/luau_codegen.h"
#include "../utils/type_mapper.h"

namespace roblox_transpiler {
  VariableNode::VariableNode(std::string name, Value value, std::string type) :
      name_(std::move(name)), value_(std::move(value)), type_(std::move(type)) {}

  void VariableNode::render(LuauCodegen &codegen) const { codegen.writeVariable(name_, value_, type_); }

  FunctionCallNode::FunctionCallNode(std::string functionName) : functionName_(std::move(functionName)) {}

  void FunctionCallNode::addArgument(const Value &arg) { arguments_.push_back(arg); }

  void FunctionCallNode::addArguments(const std::vector<Value> &args) {
    arguments_.insert(arguments_.end(), args.begin(), args.end());
  }

  void FunctionCallNode::render(LuauCodegen &codegen) const { codegen.writeFunctionCall(functionName_, arguments_); }
} // namespace roblox_transpiler
