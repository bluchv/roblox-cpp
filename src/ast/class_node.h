#pragma once
#include <memory>
#include <vector>
#include "../codegen/luau_codegen.h"
#include "luau_node.h"

namespace roblox_transpiler {
  class ClassNode : public LuauNode {
  public:
    ClassNode(const std::string &name);

    void addMethod(std::unique_ptr<LuauNode> method);
    void addMember(const std::string &name, const std::string &type);
    void addConstructor(std::unique_ptr<LuauNode> constructor);
    void render(LuauCodegen &codegen) const override;

    const std::string &getName() const { return className_; }

  private:
    std::string className_;
    std::vector<std::pair<std::string, std::string>> members_; // name, type pairs
    std::vector<std::unique_ptr<LuauNode>> methods_;
    std::unique_ptr<LuauNode> constructor_;
  };
} // namespace roblox_transpiler
