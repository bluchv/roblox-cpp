#include "luau_codegen.h"
#include <format>
#include <sstream>
#include "../ast/function_node.h"
#include "../utils/type_mapper.h"

namespace roblox_transpiler {
  void LuauCodegen::write(const std::string &text) { output_ << text; }

  void LuauCodegen::writeLine(const std::string &text) {
    writeIndented(text);
    newLine();
  }

  void LuauCodegen::writeIndented(const std::string &text) {
    indent();
    write(text);
  }

  void LuauCodegen::newLine() { output_ << '\n'; }

  void LuauCodegen::increaseIndent() { ++indentLevel_; }

  void LuauCodegen::decreaseIndent() {
    if (indentLevel_ > 0) {
      --indentLevel_;
    }
  }

  void LuauCodegen::indent() {
    for (int i = 0; i < indentLevel_; ++i) {
      output_ << INDENT_STRING;
    }
  }

  void LuauCodegen::writeFunction(const std::string &name, const std::vector<Parameter> &params,
                                  const std::string &returnType) {
    writeIndented("local function " + name + "(");

    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        write(", ");
      write(params[i].name);
      // Skip type annotations for now
      // if (!params[i].type.empty()) {
      //     write(": " + formatType(params[i].type));
      // }
    }

    write(")");
    // Skip return type annotation for now
    // if (!returnType.empty()) {
    //     write(": " + formatType(returnType));
    // }
    newLine();
  }

  void LuauCodegen::writeClassMethod(const std::string &className, const std::string &methodName,
                                     const std::vector<Parameter> &params, const std::string &returnType) {
    writeIndented("function " + className + ":" + methodName + "(");

    for (size_t i = 0; i < params.size(); ++i) {
      if (i > 0)
        write(", ");
      write(params[i].name);
    }

    write(")");
    newLine();
  }

  void LuauCodegen::writeVariable(const std::string &name, const Value &value, const std::string &type) {
    writeIndented("local " + name);
    // Skip type annotations for now
    // if (!type.empty()) {
    //     write(": " + formatType(type));
    // }
    write(" = " + valueToString(value));
    newLine();
  }

  void LuauCodegen::writeFunctionCall(const std::string &name, const std::vector<Value> &args) {
    writeIndented(name + "(");
    for (size_t i = 0; i < args.size(); ++i) {
      if (i > 0)
        write(", ");
      write(valueToString(args[i]));
    }
    write(")");
    newLine();
  }

  void LuauCodegen::writeComment(const std::string &comment) { writeLine("-- " + comment); }

  void LuauCodegen::writeAssignment(const std::string &variable, const Value &value) {
    writeIndented(variable + " = " + valueToString(value));
    newLine();
  }

  void LuauCodegen::writeReturn(const Value &value) {
    writeIndented("return " + valueToString(value));
    newLine();
  }

  void LuauCodegen::writeReturn() {
    writeIndented("return");
    newLine();
  }

  void LuauCodegen::writeToFile(const std::string &filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
      file << output_.str();
      file.close();
    }
  }

  void LuauCodegen::writeToConsole() const { std::cout << output_.str(); }

  std::string LuauCodegen::toString() const { return output_.str(); }

  void LuauCodegen::clear() {
    output_.str("");
    output_.clear();
    indentLevel_ = 0;
  }

  void LuauCodegen::writeBootstrapper(const std::string &filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
      file << "-- Roblox C++ Transpiler Bootstrapper\n";
      file << "-- This script loads and executes C++ code that has been transpiled to Luau\n\n";
      file << "local Players = game:GetService(\"Players\")\n";
      file << "local RunService = game:GetService(\"RunService\")\n\n";
      file << "-- Execute the transpiled code\n";
      file << "local function executeTranspiledCode()\n";
      file << "    -- Transpiled code:\n";

      // Insert the transpiled code with proper indentation
      std::stringstream codeStream(output_.str());
      std::string line;
      while (std::getline(codeStream, line)) {
        file << "    " << line << "\n";
      }

      file << "end\n\n";
      file << "-- Main bootstrapper function\n";
      file << "local function bootstrap()\n";
      file << "    print(\"Roblox C++ Transpiler: Starting program execution...\")\n\n";
      file << "    -- Execute the transpiled code\n";
      file << "    local success, result = pcall(executeTranspiledCode)\n\n";
      file << "    if not success then\n";
      file << "        error(\"Failed to load transpiled code: \" .. tostring(result))\n";
      file << "        return\n";
      file << "    end\n\n";
      file << "    -- Try to call main function if it exists\n";
      file << "    if _G.main and type(_G.main) == \"function\" then\n";
      file << "        print(\"Roblox C++ Transpiler: Calling main function...\")\n";
      file << "        local mainSuccess, mainResult = pcall(_G.main)\n\n";
      file << "        if not mainSuccess then\n";
      file << "            error(\"Main function failed: \" .. tostring(mainResult))\n";
      file << "        else\n";
      file << "            print(\"Roblox C++ Transpiler: Main function completed with result:\", mainResult)\n";
      file << "        end\n";
      file << "    else\n";
      file << "        print(\"Roblox C++ Transpiler: No main function found, continuing...\")\n";
      file << "    end\n\n";
      file << "    print(\"Roblox C++ Transpiler: Program execution completed.\")\n";
      file << "end\n\n";
      file << "-- Start the bootstrapper when script loads\n";
      file << "bootstrap()\n";
      file.close();
    }
  }

  std::string LuauCodegen::valueToString(const Value &value) const {
    return std::visit(
        [](const auto &v) -> std::string {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, std::string>) {
            // Check for special string literal prefix
            if (v.find("STRING_LITERAL:") == 0) {
              std::string stringValue = v.substr(15); // Remove "STRING_LITERAL:" prefix
              return "\"" + stringValue + "\"";
            }
            // Check for special expression prefix (for binary operations)
            if (v.find("EXPRESSION:") == 0) {
              std::string expressionValue = v.substr(11); // Remove "EXPRESSION:" prefix
              return expressionValue; // Return expression without quotes
            }
            // Check for special function call prefix
            if (v.find("FUNCTION_CALL:") == 0) {
              std::string functionName = v.substr(14); // Remove "FUNCTION_CALL:" prefix
              return functionName + "()";
            }
            // Check for function call with arguments
            if (v.find("FUNCTION_CALL_WITH_ARGS:") == 0) {
              std::string remaining = v.substr(24); // Remove "FUNCTION_CALL_WITH_ARGS:" prefix (24 chars)
              size_t firstColon = remaining.find(':');
              if (firstColon == std::string::npos) {
                return remaining + "()"; // No arguments
              }

              std::string functionName = remaining.substr(0, firstColon);
              std::string result = functionName + "(";
              std::string argsStr = remaining.substr(firstColon + 1);

              // Split arguments by colon and add them
              size_t start = 0;
              size_t pos = 0;
              bool first = true;
              while ((pos = argsStr.find(':', start)) != std::string::npos) {
                if (!first)
                  result += ", ";
                result += argsStr.substr(start, pos - start);
                start = pos + 1;
                first = false;
              }
              if (start < argsStr.length()) {
                if (!first)
                  result += ", ";
                result += argsStr.substr(start);
              }

              return result + ")";
            }
            // Check if it's a variable reference (no quotes for variable names)
            // Only treat as variable if it's a simple identifier
            if (v == "nil" || v == "true" || v == "false" ||
                (v.length() > 0 && (std::islower(v[0]) || v[0] == '_') &&
                 v.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") ==
                     std::string::npos)) {
              return v; // Return as-is for variable references
            }
            return "\"" + v + "\"";
          } else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
          } else if constexpr (std::is_same_v<T, double>) {
            // Format doubles with minimal decimal places
            std::ostringstream oss;
            oss << v;
            std::string result = oss.str();
            // Remove trailing zeros and decimal point if not needed
            if (result.find('.') != std::string::npos) {
              result = result.substr(0, result.find_last_not_of('0') + 1);
              if (result.back() == '.') {
                result += "0";
              }
            }
            return result;
          } else {
            return std::to_string(v);
          }
        },
        value);
  }

  std::string LuauCodegen::formatType(const std::string &cppType) const { return TypeMapper::mapCppToLuau(cppType); }
} // namespace roblox_transpiler
