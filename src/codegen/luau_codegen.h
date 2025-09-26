#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace roblox_transpiler {
  using Value = std::variant<std::string, int, double, bool>;

  // Forward declaration
  struct Parameter;

  class LuauCodegen {
  public:
    LuauCodegen() = default;

    // Basic writing methods
    void write(const std::string &text);
    void writeLine(const std::string &text);
    void writeIndented(const std::string &text);
    void newLine();

    // Indentation management
    void increaseIndent();
    void decreaseIndent();
    void indent();

    // High-level constructs
    void writeFunction(const std::string &name, const std::vector<Parameter> &params,
                       const std::string &returnType = "");
    void writeClassMethod(const std::string &className, const std::string &methodName,
                          const std::vector<Parameter> &params, const std::string &returnType = "");
    void writeVariable(const std::string &name, const Value &value, const std::string &type = "");
    void writeFunctionCall(const std::string &name, const std::vector<Value> &args);
    void writeComment(const std::string &comment);
    void writeAssignment(const std::string &variable, const Value &value);
    void writeReturn(const Value &value);
    void writeReturn(); // Return without value

    // Output methods
    void writeToFile(const std::string &filename) const;
    void writeToConsole() const;
    std::string toString() const;
    void clear();

    // Bootstrapper generation
    void writeBootstrapper(const std::string &filename) const;

  private:
    std::stringstream output_;
    int indentLevel_ = 0;
    static constexpr const char *INDENT_STRING = "    "; // 4 spaces

    std::string valueToString(const Value &value) const;
    std::string formatType(const std::string &cppType) const;
  };
} // namespace roblox_transpiler
