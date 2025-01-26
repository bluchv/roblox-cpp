//
// Created by bluec on 1/24/2025.
//

#ifndef FUNCTION_H
#define FUNCTION_H

#include <utility>
#include <vector>
#include "LuauNode.h"

class FunctionNode : public LuauNode {
public:
  explicit FunctionNode(std::string functionName) : LuauNode(), name(std::move(functionName)) {};

  void render(LuauCodeGen &writer) override {
    // Render args
    writer.indent();
    writer.write("local function " + name + "(");

    int amountOfArgs = std::size(args);
    int argIndex = 0;
    for (const auto &arg: args) {
      if (argIndex > 0) {
        writer.write(", " + arg);
      } else {
        writer.write(arg);
      }
      argIndex++;
    }

    writer.write(")\n");
    writer.increaseIndent();

    for (auto bodyNode: children) {
      bodyNode->render(writer);
    }

    writer.decreaseIndent();
    writer.indent();
    writer.write("end\n");
  }

  void addArg(std::string argName) { args.push_back(std::move(argName)); }
  void addArgs(std::vector<std::string> argNames) {
    for (const auto &argName: argNames) {
      addArg(argName);
    }
  }

private:
  std::string name;
  std::vector<std::string> args;
};

#endif // FUNCTION_H
