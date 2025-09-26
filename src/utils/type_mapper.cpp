#include "type_mapper.h"
#include <algorithm>

namespace roblox_transpiler {
  const std::unordered_map<std::string, std::string> TypeMapper::typeMap_ = {
      // Basic types
      {"int", "number"},
      {"float", "number"},
      {"double", "number"},
      {"bool", "boolean"},
      {"string", "string"},
      {"std::string", "string"},
      {"char", "string"},
      {"void", ""},

      // Pointer types
      {"int*", "number"},
      {"float*", "number"},
      {"double*", "number"},
      {"char*", "string"},
      {"const char*", "string"},

      // Common C++ types
      {"size_t", "number"},
      {"uint32_t", "number"},
      {"int32_t", "number"},
      {"uint64_t", "number"},
      {"int64_t", "number"},
  };

  const std::unordered_map<std::string, std::string> TypeMapper::robloxTypeMap_ = {
      {"Player", "Player"},       {"Instance", "Instance"},     {"Part", "Part"},       {"Model", "Model"},
      {"Workspace", "Workspace"}, {"Vector3", "Vector3"},       {"Vector2", "Vector2"}, {"CFrame", "CFrame"},
      {"Color3", "Color3"},       {"BrickColor", "BrickColor"},
  };

  std::string TypeMapper::mapCppToLuau(const std::string &cppType) {
    if (cppType.empty()) {
      return "";
    }

    // Remove const and reference qualifiers
    std::string cleanType = cppType;
    if (cleanType.starts_with("const ")) {
      cleanType = cleanType.substr(6);
    }
    if (cleanType.ends_with("&")) {
      cleanType = cleanType.substr(0, cleanType.length() - 1);
    }
    if (cleanType.ends_with(" ")) {
      cleanType = cleanType.substr(0, cleanType.length() - 1);
    }

    // Check built-in types first
    if (auto it = typeMap_.find(cleanType); it != typeMap_.end()) {
      return it->second;
    }

    // Check Roblox types
    if (auto it = robloxTypeMap_.find(cleanType); it != robloxTypeMap_.end()) {
      return it->second;
    }

    // If no mapping found, return as-is (might be a custom type)
    return cleanType;
  }

  bool TypeMapper::isBuiltinType(const std::string &type) { return typeMap_.find(type) != typeMap_.end(); }

  bool TypeMapper::isRobloxType(const std::string &type) { return robloxTypeMap_.find(type) != robloxTypeMap_.end(); }
} // namespace roblox_transpiler
