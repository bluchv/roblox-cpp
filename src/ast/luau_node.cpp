#include "luau_node.h"
#include "../codegen/luau_codegen.h"

namespace roblox_transpiler {
  void LuauNode::addChild(std::unique_ptr<LuauNode> child) {
    child->setParent(this);
    children_.push_back(std::move(child));
  }

  void LuauNode::render(LuauCodegen &codegen) const {
    for (const auto &child: children_) {
      child->render(codegen);
    }
  }
} // namespace roblox_transpiler
