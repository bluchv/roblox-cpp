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
  LuauCodeGen() : indent_level(0), function_params(0) {};

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

  void writefn(const std::string &functionName) {
    function_params = 0;
    write("local function " + functionName + "(");
  }

  void writeFnParam(const std::string &paramName, const std::string &paramType) {
    const std::string generatedType = LuauTypeGenerator::generateType(paramType);
    if (function_params > 0) {
      write(", " + paramName + (!generatedType.empty() ? ": " + generatedType : ""));
    } else {
      write(paramName + (!generatedType.empty() ? ": " + generatedType : ""));
    }
    function_params++;
  }

  void finishFnDecl(const std::string &returnType) {
    const std::string generatedType = LuauTypeGenerator::generateType(returnType);
    if (!generatedType.empty()) {
      write("): " + generatedType);
    } else {
      write(")");
    }
    write("\n");
    function_params = 0;
    increaseIndent();
  }

  void write_eof() {
    decreaseIndent();
    writeln("end");
  }

  void writeVariable(const std::string &str, const std::string &varType) {
    write("local " + str);

    auto generatedType = LuauTypeGenerator::generateType(varType);
    if (!generatedType.empty()) {
      write(": " + generatedType + " = ");
    } else {
      write(" = ");
    }
  }

  void writeToFile(const std::string &fileName) {
    std::ofstream file("tests/" + fileName);
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
  int function_params;
};


#endif // LUAUCODEGEN_H
