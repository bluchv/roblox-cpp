//
// Created by bluec on 1/24/2025.
//

#ifndef LUAU_NODE_H
#define LUAU_NODE_H

#include <any>
#include <array>
#include <string>
#include <variant>
#include <vector>
#include "../luau-code-gen/LuauCodeGen.h"

class LuauNode {
public:
  using ValueType = std::variant<std::string, int, double, bool>;
  virtual ~LuauNode() = default;
  LuauNode *parent = nullptr;

  template<typename T>
  void addChild(T *node) {
    static_assert(std::is_base_of_v<LuauNode, T>, "Child node must be derived from luau_node");
    children.push_back(node);
    node->parent = this;
  };

  virtual void render(LuauCodeGen &writer) {
    for (LuauNode *child: children) {
      child->render(writer);
    }
  }

protected:
  std::vector<LuauNode *> children = {};
};

#endif // LUAU_NODE_H
