#include "../firmware/controller/usb_maintenance.h"
#include <cassert>
#include <iostream>
using knob::BootRequest;
int main() {
  { BootRequest b; assert(!b.pending() && !b.ready(100000, false)); }
  { BootRequest b; b.request(100); assert(b.pending());
    assert(!b.ready(199, false)); assert(b.ready(200, false));
    assert(!b.ready(200, true)); assert(!b.ready(1099, true));
    assert(b.ready(1100, true)); }
  { BootRequest b; b.request(0xfffffff0u);
    assert(!b.ready(83, false)); assert(b.ready(84, false));
    assert(!b.ready(983, true)); assert(b.ready(984, true)); }
  std::cout << "3 USB maintenance scenarios passed\n";
}
