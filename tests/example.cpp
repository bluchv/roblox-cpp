#include "../include/roblox.h"
#include <iostream>
#include <stdio.h>

using namespace Roblox::Services;
using namespace Roblox;

int test = 5;
const char *testVar2 = "Hello c++!";

void something() {
  for (int i = 0; i < 50; i++) { }
}

int get_number() {
  int y = 20;
  return y;
}

int main() {
  int x = get_number();
  return 0;
}
