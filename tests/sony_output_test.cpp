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
    assert(codes.back() == 255 && !relay);
    out.tick(101); assert(relay); out.tick(155); assert(relay);
    out.tick(156); assert(!relay); out.tick(10000); assert(!relay); }
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::VolumeDown, 1}, 0));
    out.tick(0); out.tick(1); out.tick(56); assert(!relay && codes.size() == 1);
    out.tick(111); assert(codes.size() == 1);
    assert(out.submit({Action::VolumeDown, 1}, 200));
    out.tick(200); out.tick(201); assert(relay && codes.size() == 2);
    out.tick(256); assert(!relay); } // isolated detents remain discrete taps
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::NextTrack, 8}, 0));
    assert(!out.submit({Action::PreviousTrack, 1}, 0));
    assert(out.pending() == 8); out.tick(750); assert(codes.empty());
    assert(out.pending() == 0 && !relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::NextTrack, 1}, 0);
    out.tick(0); out.tick(800); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeUp, 8}, 0);
    out.tick(0); out.tick(1); out.submit({Action::CameraOn, 1}, 2);
    assert(relay && camera); out.release();
    assert(!relay && !camera && !out.enabled() && out.pending() == 0);
    out.enable(); out.tick(100); assert(!relay && codes.size() == 1); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::CameraOn, 1}, 0);
    assert(camera); out.submit({Action::VolumeUp, 1}, 0); out.tick(0); out.tick(1);
    assert(camera && relay); out.submit({Action::CameraOff, 1}, 2);
    assert(!camera && relay); out.tick(56); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeDown, 1}, 0xfffffff0u);
    out.tick(0xfffffff0u); out.tick(0xfffffff1u); assert(relay);
    out.tick(40); assert(!relay && codes.back() == 0); }
  { uint8_t code = 0;
    assert(sonyCode(Action::VolumeUp, code) && code == 255);
    assert(sonyCode(Action::VolumeDown, code) && code == 0);
    assert(sonyCode(Action::MuteAtt, code) && code == 230);
    assert(sonyCode(Action::PlayPause, code) && code == 238);
    assert(sonyCode(Action::NextTrack, code) && code == 248);
    assert(sonyCode(Action::PreviousTrack, code) && code == 215);
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
    out.tick(100); out.tick(101); assert(relay && codes.back() == 255);
    out.release(); gestures.cancel(); out.tick(102); assert(!relay && !camera); }
  { auto out = makeOutput(); out.enable();
    out.submit({Action::CameraOn, 1}, 0); out.submit({Action::VolumeDown, 3}, 0);
    assert(out.learn(Action::VolumeUp, 100));
    assert(!camera && !relay && out.pending() == 0 && out.learning());
    assert(!out.submit({Action::VolumeDown, 1}, 101));
    assert(!out.submit({Action::CameraOn, 1}, 101));
    out.tick(100); assert(!relay && codes.back() == 255);
    out.tick(101); assert(relay);
    out.tick(3101); assert(relay);
    out.tick(6100); assert(relay);
    out.tick(6101); assert(!relay && !out.learning() && !out.enabled());
    out.enable(); out.submit({Action::VolumeUp, 1}, 6200);
    out.tick(6200); out.tick(6201); assert(relay);
    out.tick(6256); assert(!relay); }
  { auto out = makeOutput(); out.enable(); out.learn(Action::VolumeDown, 0xfffffff0u);
    out.tick(0xfffffff1u); assert(relay && codes.back() == 0);
    out.tick(2985); assert(relay);
    out.tick(5985); assert(!relay && !out.enabled()); }
  { auto out = makeOutput(); out.enable(); out.learn(Action::VolumeUp, 0);
    out.tick(1); assert(relay); out.release();
    assert(!relay && !camera && !out.learning());
    out.enable(); assert(!out.learn(Action::CameraOn, 10));
    out.learn(Action::VolumeDown, 10); out.tick(800);
    assert(!relay && !out.enabled()); }
  { auto out = makeOutput(); out.enable();
    out.submit({Action::VolumeUp, 3}, 0); out.tick(0); out.tick(1); assert(relay);
    assert(out.setCode(Action::VolumeUp, 240));
    assert(!relay && !camera && out.pending() == 0 && out.enabled());
    out.submit({Action::VolumeUp, 1}, 10); out.tick(10); assert(codes.back() == 240);
    assert(out.learn(Action::VolumeUp, 20) && codes.back() == 240);
    assert(!out.setCode(Action::VolumeUp, 256));
    assert(!out.setCode(Action::CameraOn, 4));
    uint8_t code = 0; assert(out.codeFor(Action::VolumeUp, code) && code == 240);
    SonyOutput fresh(pot, setRelay, setCamera);
    assert(fresh.codeFor(Action::VolumeUp, code) && code == 255); }
  { for (const auto &entry : kDefaultCommands) {
      Action action; uint8_t code = 0;
      const std::string text = std::string(entry.name) + " 123";
      assert(parseCodeSetting(text.c_str(), action, code) && action == entry.action && code == 123);
    }
    Action action; uint8_t code = 0;
    for (const char *bad : {"", "mute_att", "mute_att ", "mute_att -1", "mute_att 256",
                           "mute_att 99999999999999999999", "mute_att 2 extra", "mute_att 1x", "camera_on 2"})
      assert(!parseCodeSetting(bad, action, code));
    assert(parseCodeSetting("volume_up 0", action, code) && code == 0);
    assert(parseCodeSetting("volume_up 255", action, code) && code == 255); }
  { for (const auto &entry : kDefaultCommands) {
      auto out = makeOutput(); out.enable(); assert(out.learn(entry.action, 0));
      assert(codes.back() == entry.code); out.tick(1); assert(relay);
      out.tick(6001); assert(!relay && !out.enabled());
    } }
  { auto out = makeOutput();
    assert(!out.learnCode(100, 0)); out.enable();
    assert(!out.learnCode(256, 0)); assert(out.learnCode(230, 0));
    assert(codes.back() == 230); out.tick(1); assert(relay);
    out.tick(6001); assert(!relay);
    uint8_t code; assert(out.codeFor(Action::PlayPause, code) && code == 238); }
  { auto out = makeOutput(); out.enable();
    Gestures gestures([](const Event &e, void *ctx) {
      static_cast<SonyOutput *>(ctx)->submit(e, 1000);
    }, &out);
    gestures.begin(0, 0, false);
    gestures.update(100, 0, true); gestures.update(130, 0, true);
    gestures.update(800, 0, false); gestures.update(830, 0, false);
    out.tick(1000); assert(codes.back() == 230); out.tick(1001); assert(relay);
    out.tick(1056); assert(!relay); } // physical long release drives accepted Mute code
  { auto out = makeOutput(); out.enable();
    const uint32_t samples[] = {1,1,2,4,5,4,3,3,2,2,2,1,1,1,1};
    uint32_t now = 0;
    for (uint32_t steps : samples) {
      assert(out.submit({Action::VolumeUp, steps}, now));
      out.tick(now); now += 10;
    }
    for (; now < 5000; ++now) out.tick(now);
    assert(codes.size() == 10 && out.pending() == 0 && !relay);
    for (int code : codes) assert(code == 255); } // same measured fast spin becomes exactly one ten-step coarse burst
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::VolumeDown, 20}, 0)); out.tick(0); out.tick(1); assert(relay);
    assert(out.submit({Action::VolumeUp, 10}, 10)); assert(!relay);
    out.tick(64); assert(!relay); out.tick(65); out.tick(66); assert(relay);
    out.tick(121); assert(!relay && out.pending() == 9);
    for (uint32_t now = 122; now < 1500; ++now) out.tick(now);
    assert(codes.size() == 11 && codes[0] == 0);
    for (unsigned i = 1; i < codes.size(); ++i) assert(codes[i] == 255); } // old direction cancelled
  { auto out = makeOutput(); out.enable();
    out.submit({Action::VolumeUp, 100}, 0); out.tick(0); out.tick(1); assert(relay);
    assert(out.setTiming(25, 25)); assert(!relay && !out.pending() && out.enabled());
    assert(out.pressMs() == 25 && out.gapMs() == 25);
    out.submit({Action::VolumeDown, 1}, 100);
    out.tick(100); out.tick(101); assert(relay);
    out.tick(125); assert(relay); out.tick(126); assert(!relay);
    out.tick(150); assert(codes.size() == 2);
    assert(out.submit({Action::VolumeDown, 1}, 151)); out.tick(151); assert(codes.size() == 3);
    out.tick(152); assert(relay);
    assert(!out.setTiming(0, 25)); assert(!out.setTiming(25, 251));
    assert(out.learn(Action::MuteAtt, 200)); out.tick(201); out.tick(6200); assert(relay);
    out.tick(6201); assert(!relay); } // tuning does not shorten learning holds
  { uint8_t press, gap;
    assert(parsePulseTiming("25 25", press, gap) && press == 25 && gap == 25);
    assert(parsePulseTiming("5 250", press, gap));
    for (const char *bad : {"", "4 25", "25 251", "25", "25 25 extra", "-1 25", "25  25", "999999 25"})
      assert(!parsePulseTiming(bad, press, gap)); }
  { auto out = makeOutput(); out.enable();
    assert(out.submit({Action::NextTrack, 8}, 0));
    assert(!out.submit({Action::VolumeUp, 10}, 10));
    assert(out.pending() == 8); out.release(); assert(!out.pending()); }
  { auto out = makeOutput(); out.enable(); out.setTiming(55, 10, 10);
    out.submit({Action::VolumeUp, 1}, 0); out.tick(0); out.tick(1); assert(relay);
    out.submit({Action::VolumeUp, 1}, 10); assert(out.volumeBoosting() && out.pending() == 9);
    out.tick(56); assert(!relay); out.tick(66); out.tick(67); assert(relay);
    for (uint32_t now = 68; now < 700; ++now) out.tick(now);
    assert(!relay && !out.pending() && codes.size() == 10); } // first tap included, bounded burst
  { auto out = makeOutput(); out.enable();
    out.submit({Action::VolumeDown, 5}, 0); out.tick(0); out.tick(1);
    out.submit({Action::MuteAtt, 1}, 10); assert(!relay && out.pending() == 1);
    out.tick(65); out.tick(66); assert(relay && codes.back() == 230);
    for (uint32_t now = 67; now < 2000; ++now) out.tick(now);
    assert(codes.size() == 2 && !out.pending()); } // deliberate audio command cancels remaining sweep
  { auto out = makeOutput(); out.enable();
    out.submit({Action::VolumeDown, 17}, 0); out.tick(100); out.tick(101); assert(relay);
    for (uint32_t now = 102; now < 2000; ++now) out.tick(now);
    assert(codes.size() == 10 && !out.pending()); } // a coarse request is not an idle-expiring hold
  { auto out = makeOutput(); out.enable(); out.setTiming(55, 10, 3);
    out.submit({Action::VolumeDown, 5}, 0xfffffff0u); out.tick(0xfffffff0u); out.tick(0xfffffff1u);
    out.tick(40); assert(!relay); out.tick(50); out.tick(51); assert(relay);
    out.tick(106); assert(!relay); } // burst timing across timestamp rollover
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeUp, 20}, 0);
    out.tick(0); out.tick(1); assert(out.volumeBoosting()); out.release();
    assert(!relay && !out.volumeBoosting() && !out.pending());
    out.enable(); out.tick(1000); assert(codes.size() == 1); }
  { auto out = makeOutput(); out.enable(); out.submit({Action::VolumeUp, 10}, 0);
    out.tick(0); out.tick(1); out.submit({Action::VolumeUp, 1}, 10);
    assert(out.learn(Action::VolumeDown, 20)); out.tick(21); out.tick(121); assert(relay);
    out.tick(6021); assert(!relay); } // short idle watchdog does not truncate explicit learning holds
  { auto out = makeOutput(); out.enable();
    assert(out.setTiming(55, 10, 14)); assert(out.boostSteps() == 14);
    assert(out.setTiming(55, 20)); assert(out.boostSteps() == 14);
    assert(!out.setTiming(55, 10, 21)); assert(out.boostSteps() == 14 && out.enabled());
    uint8_t press = 0, gap = 0, boost = 14;
    assert(parseTiming("55 10", press, gap, boost) && boost == 14);
    assert(parseTiming("55 10 12", press, gap, boost) && boost == 12);
    for (const char *bad : {"55 10 0", "55 10 21", "55 10 10 10", "55  10", "55 10 ", "55 10 x"})
      assert(!parseTiming(bad, press, gap, boost)); }
  { auto out = makeOutput(); out.enable(); out.setTiming(55, 10, 10);
    out.submit({Action::VolumeUp, 2}, 0);
    for (uint32_t now = 0; now < 1200; ++now) {
      if (now && now % 10 == 0) out.submit({Action::VolumeUp, 5}, now);
      out.tick(now);
      assert(out.pending() <= 10);
    }
    for (uint32_t now = 1200; now < 1400; ++now) out.tick(now);
    assert(!relay && !out.pending()); assert(codes.size() > 10 && codes.size() < 25); }
  std::cout << "31 Sony output scenarios passed\n";
}
