#include "function_node.h"
#include "../codegen/luau_codegen.h"
#include "../utils/type_mapper.h"

namespace roblox_transpiler {
  FunctionNode::FunctionNode(std::string name, std::string returnType) :
      name_(std::move(name)), returnType_(std::move(returnType)), className_("") {}

  void FunctionNode::addParameter(const std::string &name, const std::string &type) {
    parameters_.emplace_back(name, type);
  }

  void FunctionNode::addParameters(const std::vector<Parameter> &params) {
    parameters_.insert(parameters_.end(), params.begin(), params.end());
  }

  void FunctionNode::setReturnType(const std::string &returnType) { returnType_ = returnType; }

  void FunctionNode::setClassName(const std::string &className) { className_ = className; }

  void FunctionNode::render(LuauCodegen &codegen) const {
    if (!className_.empty()) {
      // This is a class method
      codegen.writeClassMethod(className_, name_, parameters_, returnType_);
    } else {
      // Regular function
      codegen.writeFunction(name_, parameters_, returnType_);
    }
    codegen.increaseIndent();

    // Render function body
    LuauNode::render(codegen);

    codegen.decreaseIndent();
    codegen.writeLine("end");
  }
} // namespace roblox_transpiler
