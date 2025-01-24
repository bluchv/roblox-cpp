//
// Created by bluec on 1/24/2025.
//

#ifndef LUAU_AST_GENERATOR_H
#define LUAU_AST_GENERATOR_H

#include <array>
#include <vector>
#include "LuauNode.h"
#include "function.h"
#include "test_node.h"
#include "variable.h"

class luau_ast_generator {
public:
  luau_ast_generator() {
    LuauNode root;
    LuauCodeGen codeGen;
    FunctionNode testFunctionNode("test");
    testFunctionNode.addArgs({{"test1"}});

    VariableNode stringVariable("string", "teasdst");
    VariableNode intVariable("int", 15500);
    VariableNode boolVariable("bool", false);
    testFunctionNode.addChild(&stringVariable);
    testFunctionNode.addChild(&intVariable);
    testFunctionNode.addChild(&boolVariable);

    root.addChild(&testFunctionNode);
    root.render(codeGen);
    codeGen.writeToConsole();
  };

  void push_node(bool node) { nodes.push_back(node); }

private:
  std::vector<bool> nodes = {};
};


#endif // LUAU_AST_GENERATOR_H
