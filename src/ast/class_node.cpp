#include "class_node.h"

namespace roblox_transpiler {
  ClassNode::ClassNode(const std::string &name) : className_(name), constructor_(nullptr) {}

  void ClassNode::addMethod(std::unique_ptr<LuauNode> method) { methods_.push_back(std::move(method)); }

  void ClassNode::addMember(const std::string &name, const std::string &type) { members_.push_back({name, type}); }

  void ClassNode::addConstructor(std::unique_ptr<LuauNode> constructor) { constructor_ = std::move(constructor); }

  void ClassNode::render(LuauCodegen &codegen) const {
    // Generate Roblox-ts style class wrapper
    codegen.writeComment("Class: " + className_);
    codegen.writeLine("local " + className_);
    codegen.writeLine("do");
    codegen.increaseIndent();

    // Initialize class table with metatable
    codegen.writeLine(className_ + " = setmetatable({}, {");
    codegen.increaseIndent();
    codegen.writeLine("__tostring = function()");
    codegen.increaseIndent();
    codegen.writeLine("return \"" + className_ + "\"");
    codegen.decreaseIndent();
    codegen.writeLine("end,");
    codegen.decreaseIndent();
    codegen.writeLine("})");
    codegen.writeLine(className_ + ".__index = " + className_);
    codegen.newLine();

    // Constructor function
    codegen.writeLine("function " + className_ + ".new(...)");
    codegen.increaseIndent();
    codegen.writeLine("local self = setmetatable({}, " + className_ + ")");

    // Initialize members with default values first
    for (const auto &member: members_) {
      if (member.second == "int" || member.second == "double") {
        codegen.writeLine("self." + member.first + " = 0");
      } else if (member.second == "bool") {
        codegen.writeLine("self." + member.first + " = false");
      } else if (member.second == "string" || member.second == "std::string") {
        codegen.writeLine("self." + member.first + " = \"\"");
      } else {
        codegen.writeLine("self." + member.first + " = nil");
      }
    }

    // If we have a constructor, call the init method
    if (constructor_) {
      codegen.writeLine("self:init(...)");
    }

    codegen.writeLine("return self");
    codegen.decreaseIndent();
    codegen.writeLine("end");
    codegen.newLine();

    // Render constructor as init method if present
    if (constructor_) {
      constructor_->render(codegen);
      codegen.newLine();
    }

    // Render methods
    for (const auto &method: methods_) {
      method->render(codegen);
    }

    // Close the do block
    codegen.decreaseIndent();
    codegen.writeLine("end");
    codegen.newLine();
  }
} // namespace roblox_transpiler
