#pragma once

#include <string>
#include <vector>
#include "luau_node.h"

namespace roblox_transpiler {
  struct Parameter {
    std::string name;
    std::string type;

    Parameter(std::string n, std::string t = "") : name(std::move(n)), type(std::move(t)) {}
  };

  class FunctionNode : public LuauNode {
  public:
    explicit FunctionNode(std::string name, std::string returnType = "");

    void addParameter(const std::string &name, const std::string &type = "");
    void addParameters(const std::vector<Parameter> &params);
    void setReturnType(const std::string &returnType);
    void setClassName(const std::string &className);

    void render(LuauCodegen &codegen) const override;

    const std::string &getName() const { return name_; }
    const std::vector<Parameter> &getParameters() const { return parameters_; }

  private:
    std::string name_;
    std::string returnType_;
    std::vector<Parameter> parameters_;
    std::string className_; // Empty if not a class method
  };
} // namespace roblox_transpiler
