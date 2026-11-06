#include "../firmware/controller/sony_output.h"
#include <cassert>
#include <iostream>
#include <vector>
using namespace knob;
bool relay = false, camera = false;
std::vector<int> codes;
void pot(uint8_t code) { assert(!relay); codes.push_back(code); }
void setRelay(bool on) { relay = on; }
void setCamera(bool on) { camera = on; }
SonyOutput makeOutput() { codes.clear(); relay = camera = false; return SonyOutput(pot, setRelay, setCamera); }
int main() {
  { auto out = makeOutput(); assert(!out.submit({Action::VolumeUp, 1}, 0));
    out.tick(100); assert(codes.empty() && !relay); }
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::VolumeUp, 1}, 100)); out.tick(100);
    assert(codes.back() == 64 && !relay);
    out.tick(101); assert(relay); out.tick(155); assert(relay);
    out.tick(156); assert(!relay); out.tick(10000); assert(!relay); }
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::PreviousTrack, 2}, 0));
    out.tick(0); out.tick(1); out.tick(56); assert(!relay && codes.size() == 1);
    out.tick(110); assert(codes.size() == 1); out.tick(111);
    assert(codes.size() == 2 && codes.back() == 192 && !relay);
    out.tick(112); assert(relay); }
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::NextTrack, 8}, 0));
    assert(!out.submit({Action::VolumeDown, 1}, 0));
    assert(out.pending() == 8); out.tick(750); assert(codes.empty());
    assert(out.pending() == 0 && !relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeUp, 1}, 0);
    out.tick(0); out.tick(800); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeUp, 8}, 0);
    out.tick(0); out.tick(1); out.submit({Action::CameraOn, 1}, 2);
    assert(relay && camera); out.release();
    assert(!relay && !camera && !out.enabled() && out.pending() == 0);
    out.enable(); out.tick(100); assert(!relay && codes.size() == 1); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::CameraOn, 1}, 0);
    assert(camera); out.submit({Action::MuteAtt, 1}, 0); out.tick(0); out.tick(1);
    assert(camera && relay); out.submit({Action::CameraOff, 1}, 2);
    assert(!camera && relay); out.tick(56); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeDown, 1}, 0xfffffff0u);
    out.tick(0xfffffff0u); out.tick(0xfffffff1u); assert(relay);
    out.tick(40); assert(!relay && codes.back() == 96); }
  { uint8_t code = 0;
    assert(sonyCode(Action::MuteAtt, code) && code == 128);
    assert(sonyCode(Action::NextTrack, code) && code == 160);
    assert(!sonyCode(Action::CameraOn, code));
    auto out = makeOutput(); out.enable();
    assert(!out.submit({Action::LongPressIgnored, 1}, 0));
    assert(!out.submit({Action::VolumeUp, 0}, 0));
    assert(!out.submit({Action::VolumeUp, 0xffffffffu}, 0)); }
  { auto out = makeOutput(); out.enable();
    Gestures gestures([](const Event &e, void *ctx) {
      static_cast<SonyOutput *>(ctx)->submit(e, 100);
    }, &out);
    gestures.begin(0, 0, false); gestures.update(100, 1, false);
    out.tick(100); out.tick(101); assert(relay && codes.back() == 64);
    out.release(); gestures.cancel(); out.tick(102); assert(!relay && !camera); }
  { auto out = makeOutput(); out.enable();
    out.submit({Action::CameraOn, 1}, 0); out.submit({Action::NextTrack, 3}, 0);
    assert(out.learn(Action::VolumeUp, 100));
    assert(!camera && !relay && out.pending() == 0 && out.learning());
    assert(!out.submit({Action::VolumeDown, 1}, 101));
    assert(!out.submit({Action::CameraOn, 1}, 101));
    out.tick(100); assert(!relay && codes.back() == 64);
    out.tick(101); assert(relay);
    out.tick(3101); assert(relay);
    out.tick(6100); assert(relay);
    out.tick(6101); assert(!relay && !out.learning() && !out.enabled());
    out.enable(); out.submit({Action::MuteAtt, 1}, 6200);
    out.tick(6200); out.tick(6201); assert(relay);
    out.tick(6256); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.learn(Action::VolumeDown, 0xfffffff0u);
    out.tick(0xfffffff1u); assert(relay && codes.back() == 96);
    out.tick(2985); assert(relay);
    out.tick(5985); assert(!relay && !out.enabled()); }
  { auto out = makeOutput(); out.enable(); out.learn(Action::MuteAtt, 0);
    out.tick(1); assert(relay); out.release();
    assert(!relay && !camera && !out.learning());
    out.enable(); assert(!out.learn(Action::CameraOn, 10));
    out.learn(Action::NextTrack, 10); out.tick(800);
    assert(!relay && !out.enabled()); }
  std::cout << "13 Sony output scenarios passed\n";
}
