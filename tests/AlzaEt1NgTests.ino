#line 2 "AlzaEt1NgTests.ino"

#include <AUnit.h>

using namespace aunit;


test(correct) {
  int x = 1;
  assertEqual(x, 1);
}

void setup() {
}


void loop() {
  TestRunner::run();
}