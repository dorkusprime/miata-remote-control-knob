#include "../firmware/controller/output_trace.h"
#include <cassert>
#include <iostream>
using namespace knob;
int main() {
  { OutputTrace t; t.record({1, 0, 0, 0, false}); assert(!t.front());
    t.record({2, 3, 255, 7, true}); t.record({3, 4, 255, 6, true});
    assert(t.front()->at == 2 && t.front()->pending == 7);
    t.pop(); assert(!t.front()); t.record({57, 58, 255, 6, false});
    assert(t.front()->at == 57 && !t.front()->on); }
  { OutputTrace t;
    for (unsigned i = 0; i < 34; ++i) t.record({i, i, 0, 0, !(i % 2)});
    assert(t.lost() == 2);
    for (unsigned i = 0; i < 32; ++i) { assert(t.front()->at == i); t.pop(); }
    assert(!t.front()); t.record({40, 9, 0, 0, true});
    assert(t.front() && t.front()->at == 40); }
  { OutputTrace t; t.record({0xfffffff0u, 10, 230, 0, true});
    t.discard(); assert(!t.front() && !t.lost());
    t.record({10, 36, 230, 0, true}); assert(!t.front());
    t.record({20, 46, 230, 0, false}); assert(t.front()->at == 20); }
  std::cout << "3 output trace scenarios passed\n";
}
