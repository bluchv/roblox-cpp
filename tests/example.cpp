#include <iostream>
#include <stdio.h>
#include "../include/roblox.h"

using namespace Roblox::Services;
using namespace Roblox;

int test = 5;
const char *testVar2 = "Hello c++!";

void something() {
  for (int i = 0; i < 50; i++) { }
  std::cout << "Hi" << std::endl;
}

int get_number() {
  int y = 20;
  return y;
}

void pointerTest(int var) { }

int main() {
  //  int x = get_number();
  return 0;
}
