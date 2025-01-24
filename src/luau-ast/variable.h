//
// Created by bluec on 1/24/2025.
//

#ifndef VARIABLE_H
#define VARIABLE_H

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
    writer.writeln(output + ";");
  }

private:
  ValueType value;
  std::string name;
};

#endif // VARIABLE_H
