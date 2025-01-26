//
// Created by bluec on 1/24/2025.
//

#ifndef LUAU_AST_GENERATOR_H
#define LUAU_AST_GENERATOR_H

#include <array>
#include <functional>
#include <vector>
#include "Function.h"
#include "LuauNode.h"
#include "Variable.h"

class LuauAstGenerator {
public:
  LuauAstGenerator() {
    /* ! Direct-node testing ! */
    // LuauNode root;
    // LuauCodeGen codeGen;
    // FunctionNode testFunctionNode("test");
    //
    // VariableNode stringVariable("string", "teasdst");
    // VariableNode intVariable("int", 15500);
    // VariableNode boolVariable("bool", false);
    // VariableFunctionNode testfunctionVariableNode("ihateprogramming", "kill");
    //
    // root.addChild(&testfunctionVariableNode);
    // testFunctionNode.addChild(&stringVariable);
    // testFunctionNode.addChild(&intVariable);
    // testFunctionNode.addChild(&boolVariable);
    //
    // root.addChild(&testFunctionNode);
    // root.render(codeGen);
    // codeGen.writeToConsole();
  };

  void push_node(bool node) { nodes.push_back(node); }

private:
  std::vector<bool> nodes = {};
};


#endif // LUAU_AST_GENERATOR_H
