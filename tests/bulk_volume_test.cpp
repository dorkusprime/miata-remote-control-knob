#include "../firmware/controller/bulk_volume.h"
#include <cassert>
#include <iostream>
#include <vector>
using namespace knob;
static bool relay, camera;
static uint8_t code;
static std::vector<int> sent;
void pot(uint8_t n) { assert(!relay); code = n; }
void switchRelay(bool on) { if (on && !relay) sent.push_back(code); relay = on; }
void switchCamera(bool on) { camera = on; }
struct Bench {
  SonyOutput output{pot, switchRelay, switchCamera};
  BulkVolume bulk;
  uint32_t now = 0;
  Bench() { relay = camera = false; sent.clear(); output.enable(); }
  void advance(uint32_t end) { while (now < end) output.tick(++now); }
  bool turn(int delta) {
    return bulk.submit({delta > 0 ? Action::VolumeUp : Action::VolumeDown,
                        static_cast<uint32_t>(delta > 0 ? delta : -delta)}, now, output);
  }
  void drain() { advance(now + 10000); assert(!relay && !output.pending()); }
};
int main() {
  unsigned scenarios = 0;
  // Immediate recognition in both directions; account for pulses sent during flicks.
  for (int direction : {-1, 1}) {
    Bench b;
    assert(b.turn(direction * 2)); b.advance(80);
    assert(b.turn(-direction * 2)); b.advance(150);
    assert(b.turn(direction)); assert(b.bulk.recognized());
    b.drain(); assert(b.output.volumeSent() == direction * 10); ++scenarios;
  }
  { Bench b; b.turn(-3); b.advance(20); b.turn(2); b.advance(40); b.turn(-3);
    assert(b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -10); ++scenarios; }
  // Existing pending ordinary steps remain in the original intended position.
  { Bench b; b.turn(20); b.advance(1000); // 17 sent, remainder still pending
    assert(b.output.projectedVolume() == 20);
    b.turn(-1); b.advance(1040); b.turn(1); b.advance(1080); b.turn(-1);
    assert(b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == 10); ++scenarios; }
  // Zero steps sent: cancel an unstarted settle and replace with exact target.
  { Bench b; b.turn(1); b.output.tick(0); b.turn(-1); b.turn(1);
    assert(b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == 10); ++scenarios; }
  // Current pulse must finish at full width and its original release gap remains.
  { Bench b; b.turn(-1); b.advance(2); assert(relay); b.turn(1); b.turn(-1);
    assert(relay && b.bulk.recognized()); b.advance(30); assert(relay);
    b.advance(32); assert(!relay); b.advance(61); assert(!relay);
    b.drain(); assert(b.output.volumeSent() == -10); ++scenarios; }
  // Tail samples are consumed, not multiplied and not self-interrupting.
  { Bench b; b.turn(1); b.advance(10); b.turn(-1); b.advance(20); b.turn(1);
    for (unsigned t = 30; t <= 300; t += 10) { b.advance(t); b.turn(3); assert(!b.bulk.recognized()); }
    b.drain(); assert(b.output.volumeSent() == 10); ++scenarios; }
  // A fresh same-direction turn after quiet cancels bulk and is an ordinary step.
  { Bench b; b.turn(1); b.advance(10); b.turn(-1); b.advance(20); b.turn(1);
    b.advance(140); const auto before = b.output.volumeSent(); b.turn(1);
    b.drain(); assert(b.output.volumeSent() == before + 1); ++scenarios; }
  // Reversal interrupts immediately even before quiet; an already active pulse finishes.
  { Bench b; b.turn(1); b.advance(10); b.turn(-1); b.advance(20); b.turn(1);
    const auto before = b.output.volumeSent(); b.advance(21); b.turn(-1);
    b.drain(); assert(b.output.volumeSent() == before - 1); ++scenarios; }
  // Incomplete, oversized and expired patterns stay proportional.
  { Bench b; b.turn(-2); b.advance(50); b.turn(2); b.drain(); assert(b.output.volumeSent() == 0); ++scenarios; }
  { Bench b; b.turn(-7); b.advance(50); b.turn(1); b.advance(100); b.turn(-1);
    assert(!b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -7); ++scenarios; }
  { Bench b; b.turn(-1); b.advance(50); b.turn(7); b.advance(100); b.turn(-1);
    assert(!b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == 5); ++scenarios; }
  { Bench b; b.turn(-1); b.advance(251); b.turn(1); b.advance(300); b.turn(-1);
    assert(!b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -1); ++scenarios; }
  { Bench b; auto s = b.bulk.settings(); s.windowMs = 70; b.bulk.configure(s, b.output, 0);
    b.turn(-1); b.advance(40); b.turn(1); b.advance(80); b.turn(-1);
    assert(!b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -1); ++scenarios; }
  // Every detent threshold is live-tunable; third triggers at minimum without waiting.
  { Bench b; auto s = b.bulk.settings(); s.firstMin = s.firstMax = 2;
    s.secondMin = s.secondMax = 3; s.thirdMin = 4; s.steps = 15; b.bulk.configure(s, b.output, 0);
    b.turn(-2); b.advance(40); b.turn(3); b.advance(80); b.turn(-3);
    assert(!b.bulk.recognized()); b.advance(90); b.turn(-1);
    assert(b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -15); ++scenarios; }
  { Bench b; auto s = b.bulk.settings(); s.firstMin = 2; b.bulk.configure(s, b.output, 0);
    b.turn(-1); b.advance(30); b.turn(1); b.advance(60); b.turn(-1);
    assert(!b.bulk.recognized()); b.drain(); assert(b.output.volumeSent() == -1); ++scenarios; }
  // Button/raw press and other actions cancel bulk, but do not clear Camera 1 state.
  { Bench b; b.output.submit({Action::CameraOn, 1}, 0);
    b.turn(1); b.advance(10); b.turn(-1); b.advance(20); b.turn(1);
    auto before = b.output.volumeSent(); b.bulk.interrupt(b.output, b.now); b.drain();
    assert(camera && b.output.volumeSent() == before); ++scenarios; }
  for (Action action : {Action::PlayPause, Action::MuteAtt, Action::NextTrack, Action::PreviousTrack, Action::CameraOff}) {
    Bench b; b.turn(1); b.advance(10); b.turn(-1); b.advance(20); b.turn(1);
    auto before = b.output.volumeSent(); assert(b.bulk.submit({action, 1}, b.now, b.output));
    b.drain(); assert(b.output.volumeSent() == before); ++scenarios;
  }
  { Bench b; auto s = b.bulk.settings(); s.enabled = false; b.bulk.configure(s, b.output, 0);
    b.turn(-1); b.turn(1); b.turn(-1); assert(!b.bulk.recognized());
    b.drain(); assert(b.output.volumeSent() == -1); ++scenarios; }
  // Gesture work may have a net opposite sign; compensate rather than adding ten blindly.
  { Bench b; auto s = b.bulk.settings(); s.gapMs = s.windowMs = 1000;
    b.bulk.configure(s, b.output, 0); b.turn(-1); b.advance(50); b.turn(6); b.advance(450);
    assert(b.output.volumeSent() == 5); b.turn(-1); assert(b.bulk.recognized());
    assert(b.bulk.correction() == -15); b.drain(); assert(b.output.volumeSent() == -10); ++scenarios; }
  // Post-trigger direction interrupts do not accidentally complete an overlapping pattern.
  { Bench b; b.turn(-1); b.advance(10); b.turn(1); b.advance(20); b.turn(-1);
    assert(b.bulk.recognized()); b.advance(30); b.turn(1); assert(!b.bulk.recognized());
    b.advance(40); b.turn(-1); assert(!b.bulk.recognized()); ++scenarios; }
  // Quiet threshold is measured from the last tail detent, not recognition time.
  { Bench b; b.turn(-1); b.advance(10); b.turn(1); b.advance(20); b.turn(-1);
    b.advance(130); b.turn(-4); b.advance(240); b.turn(-4);
    b.drain(); assert(b.output.volumeSent() == -10); ++scenarios; }
  // uint32_t timer rollover does not prevent detection.
  { Bench b; b.now = 0xfffffff0u; b.turn(1); b.output.tick(b.now); b.now += 10;
    b.output.tick(b.now); b.turn(-1); b.now += 10; b.output.tick(b.now); b.turn(1);
    assert(b.bulk.recognized()); for (unsigned i = 0; i < 1000; ++i) b.output.tick(++b.now);
    assert(b.output.volumeSent() == 10); ++scenarios; }
  { Bench b; b.output.release(); assert(!b.turn(1)); assert(b.output.volumeSent() == 0);
    b.output.enable(); b.output.learn(Action::VolumeUp, 0); assert(!b.turn(1)); ++scenarios; }
  // Cancel only volume: unrelated queued commands and camera request survive.
  { Bench b; b.output.submit({Action::CameraOn, 1}, 0);
    b.output.submit({Action::VolumeUp, 3}, 0); b.output.submit({Action::NextTrack, 1}, 0);
    b.output.cancelVolume(0); assert(camera && b.output.pending() == 1); b.drain();
    assert(sent.size() == 1 && sent[0] == 248); ++scenarios; }
  { FlickSettings s;
    for (const char *text : {"first 2 4", "second 3 5", "third 2", "gap 200", "window 600", "quiet 150", "steps 12", "off", "on"})
      assert(parseFlickSetting(text, s));
    assert(s.firstMin == 2 && s.firstMax == 4 && s.secondMin == 3 && s.secondMax == 5 && s.thirdMin == 2 &&
           s.gapMs == 200 && s.windowMs == 600 && s.quietMs == 150 && s.steps == 12 && s.enabled);
    for (const char *text : {"", "first 4 2", "first 0 2", "first 1 101", "third 0", "steps 101", "gap 0", "quiet 5001", "third 1 2", "gap 30 x", "unknown 1", "steps -1", "first 1 2 extra", "steps 9999999999999999999999999", "steps 4294967306", "steps +1", "steps 1 ", "steps  1"})
      assert(!parseFlickSetting(text, s));
    assert(s.steps == 12 && s.firstMin == 2); ++scenarios; }
  std::cout << scenarios << " bulk volume scenarios passed\n";
}
