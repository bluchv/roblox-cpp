#include "statement_node.h"
#include "../codegen/luau_codegen.h"

namespace roblox_transpiler {
  AssignmentNode::AssignmentNode(std::string variable, Value value) :
      variable_(std::move(variable)), value_(std::move(value)) {}

  void AssignmentNode::render(LuauCodegen &codegen) const { codegen.writeAssignment(variable_, value_); }

  ReturnNode::ReturnNode(Value value) :
      value_(std::move(value)),
      hasValue_(!std::holds_alternative<std::string>(value_) || !std::get<std::string>(value_).empty()) {}

  void ReturnNode::render(LuauCodegen &codegen) const {
    if (hasValue_) {
      codegen.writeReturn(value_);
    } else {
      codegen.writeReturn();
    }
  }
} // namespace roblox_transpiler
