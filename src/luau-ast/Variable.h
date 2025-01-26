//
// Created by bluec on 1/24/2025.
//

#ifndef VARIABLE_H
#define VARIABLE_H

#include <functional>
#include <type_traits>
#include <variant>
#include "LuauNode.h"

class VariableNode : public LuauNode {
public:
  VariableNode(std::string _name, ValueType _value) : LuauNode(), value(_value), name(_name) {}
  void render(LuauCodeGen &writer) override {
    std::string output = "local " + name;
    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::string>) {
            output += std::format(": string = \"{}\"", arg);
          } else if constexpr (std::is_same_v<T, bool>) {
            output += std::format(": boolean = {}", arg ? "true" : "false");
          } else { // int, double, etc.
            output += std::format(": number = {}", arg);
          }
        },
        value);
    writer.writeln(output);
  }

private:
  ValueType value;
  std::string name;
};

class VariableFunctionNode : public LuauNode {
public:
  VariableFunctionNode(std::string _name, std::string _valueId) :
      name(std::move(_name)), valueID(std::move(_valueId)), args({}) {}
  void render(LuauCodeGen &writer) override {
    std::string output = std::format("local {} = {}(", name, valueID);

    int argIndex = 0;
    for (auto &arg: args) {
      if (argIndex > 0) {
        std::visit([&](auto &&visitedArg) { output += std::format(", {}", visitedArg); }, arg);
      } else {
        std::visit([&](auto &&visitedArg) { output += visitedArg; }, arg);
      }
      argIndex++;
    }

    output += ")";
    writer.writeln(output);
  }

  void addArgument(ValueType _valueId) { args.push_back(_valueId); }
  void addArguments(std::vector<ValueType> _arguments) {
    for (auto &arg: _arguments) {
      addArgument(arg);
    }
  }

private:
  std::string name;
  std::string valueID;
  std::vector<ValueType> args;
};

#endif // VARIABLE_H
