//
// Created by bluec on 1/24/2025.
//

#ifndef TEST_NODE_H
#define TEST_NODE_H

#include <iostream>
#include "LuauNode.h"

class TestNode : public LuauNode {
public:
  void render(LuauCodeGen &writer) override { std::cout << "Render from testnode!" << std::endl; }
};

#endif // TEST_NODE_H
