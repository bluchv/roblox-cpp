//
// Created by bluec on 1/22/2025.
//

#ifndef LUAUCODEGEN_H
#define LUAUCODEGEN_H

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include "type-gen/LuauTypeGenerator.h"

class LuauCodeGen {
public:
  LuauCodeGen() : indent_level(0) {};

  void indent() {
    for (int i = 0; i < indent_level; i++) {
      output << "  ";
    }
  }

  void increaseIndent() { indent_level++; }
  void decreaseIndent() { indent_level--; }

  void write(const std::string &str) { output << str; }
  void writeln(const std::string &str) {
    indent();
    output << str;
    output << "\n";
  }

  void writefn(const std::string &functionName, const std::string &returnType) {
    auto generatedReturnType = LuauTypeGenerator::generateType(returnType);
    if (!generatedReturnType.empty()) {
      writeln("local function " + functionName + "(): " + generatedReturnType);
    } else {
      writeln("local function " + functionName + "()");
    }
    increaseIndent();
  }

  void write_eof() {
    decreaseIndent();
    writeln("end");
  }

  void writeVariable(const std::string &str, const std::string &varType) {
    write("local " + str);

    auto generatedType = LuauTypeGenerator::generateType(str);
    if (!generatedType.empty()) {
      write(": " + generatedType + " = ");
    } else {
      write(" = ");
    }
  }

  void writeToFile(const std::string &fileName) {
    std::ofstream file(fileName);
    file << output.str();
    file.close();
  }

  void writeToConsole() const { std::cout << output.str(); };

  void removeTrailingComma() {
    if (std::string currentOutput = output.str();
        currentOutput.size() >= 2 && currentOutput.substr(currentOutput.size() - 2) == ", ") {
      output.str(currentOutput.substr(0, currentOutput.size() - 2));
    }
  }

private:
  std::stringstream output;
  int indent_level;
};


#endif // LUAUCODEGEN_H
