#pragma once

#include <string>
#include <unordered_map>

namespace roblox_transpiler {
  class TypeMapper {
  public:
    static std::string mapCppToLuau(const std::string &cppType);
    static bool isBuiltinType(const std::string &type);
    static bool isRobloxType(const std::string &type);

  private:
    static const std::unordered_map<std::string, std::string> typeMap_;
    static const std::unordered_map<std::string, std::string> robloxTypeMap_;
  };
} // namespace roblox_transpiler
