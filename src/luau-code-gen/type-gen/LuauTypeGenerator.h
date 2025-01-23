//
// Created by bluec on 1/22/2025.
//

#ifndef LUAUTYPEGENERATOR_H
#define LUAUTYPEGENERATOR_H

#include <iostream>
#include <string>

class LuauTypeGenerator {
public:
  static std::string generateType(const std::string &inputType) {
    if (inputType == "int") {
      return "number";
    } else if (inputType == "void") {
      return "nil";
    } else if (inputType == "std::string" || inputType == "string" || inputType == "const char *") {
      return "string";
    }
    std::cout << "Unknown type: " << inputType << std::endl;
    return "";
  }
};

#endif // LUAUTYPEGENERATOR_H
