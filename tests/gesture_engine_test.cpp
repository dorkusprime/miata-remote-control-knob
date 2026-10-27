#include "../firmware/controller/gesture_engine.h"
#include <cassert>
#include <iostream>
#include <vector>
using knob::Action;
struct Test {
  std::vector<knob::Event> events;
  knob::Gestures engine;
  Test(uint32_t now = 0, uint16_t count = 0, bool down = false)
      : engine([](const knob::Event &e, void *ctx) {
          static_cast<Test *>(ctx)->events.push_back(e);
        }, this) { engine.begin(now, count, down); }
  void tick(uint32_t now, uint16_t count = 0, bool down = false) { engine.update(now, count, down); }
  void press(uint32_t now, uint16_t count = 0) { tick(now, count, true); tick(now + 30, count, true); }
  void release(uint32_t now, uint16_t count = 0) { tick(now, count, false); tick(now + 30, count, false); }
  void tap(uint32_t now) { press(now); release(now + 80); }
  void expect(std::initializer_list<Action> actions) {
    assert(events.size() == actions.size());
    size_t i = 0; for (Action action : actions) assert(events[i++].action == action);
  }
};

int main() {
  { Test t; t.tick(10, 3); t.tick(20, 1); t.expect({Action::VolumeUp, Action::VolumeDown});
    assert(t.events[0].steps == 3 && t.events[1].steps == 2); }
  { Test t; t.tap(100); t.tick(479); t.expect({}); t.tick(480); t.expect({Action::MuteAtt});
    t.tick(1000); t.expect({Action::MuteAtt}); }
  { Test t; t.tap(100); t.tap(300); t.tick(1000); t.expect({Action::CameraOn});
    assert(t.engine.cameraRequested()); t.tap(1100); t.tap(1300); t.expect({Action::CameraOn, Action::CameraOff}); }
  { Test t; t.tap(100); t.press(500); t.release(580); t.tick(880);
    t.expect({Action::MuteAtt, Action::MuteAtt}); }
  { Test t; t.tap(100); t.press(480); t.release(560); t.tick(1000);
    t.expect({Action::CameraOn}); }  // second press at exact 300ms boundary
  { Test t; t.press(100); t.tick(150, 2, true); t.tick(180, 1, true); t.release(200, 1); t.tick(1000, 1);
    t.expect({Action::NextTrack, Action::PreviousTrack}); assert(t.events[0].steps == 2); }
  { Test t; t.tick(100, 0, true); t.tick(110, 1, true); t.tick(130, 1, true); t.release(200, 1); t.tick(1000, 1);
    t.expect({Action::NextTrack}); }  // rotation during press debounce
  { Test t; t.press(100); t.tick(200, 0, false); t.tick(210, 1, false); t.tick(230, 1, false); t.tick(1000, 1);
    t.expect({Action::NextTrack}); }  // rotation during release debounce belongs to held turn
  { Test t; t.tick(100, 0, true); t.tick(110); t.tick(120, 0, true); t.tick(130); t.tick(1000); t.expect({}); }
  { Test t; t.press(100); t.release(800); t.tick(1500); t.expect({Action::LongPressIgnored}); }
  { Test t; t.tap(100); t.press(300); t.release(1000); t.tick(1500); t.expect({Action::LongPressIgnored}); }
  { Test t; t.tap(100); t.tick(250, 1); t.tick(1000, 1); t.expect({Action::VolumeUp}); }
  { Test t; t.tap(100); t.press(300); t.tick(350, 1, true); t.release(400, 1); t.tick(1000, 1);
    t.expect({Action::NextTrack}); }  // second click becomes held turn, never camera
  { Test t(0, 65535); t.tick(10, 0); t.tick(20, 65535); t.expect({Action::VolumeUp, Action::VolumeDown});
    assert(t.events[0].steps == 1 && t.events[1].steps == 1); }
  { Test t(0, 32767); t.tick(10, 32768); t.expect({Action::VolumeUp}); assert(t.events[0].steps == 1); }
  { Test t(0, 1234, true); t.tick(10, 1235, true); t.release(100, 1235); t.tick(500, 1235); t.expect({});
    t.tick(510, 1236); t.expect({Action::VolumeUp}); }
  { Test t; t.tap(100); t.engine.cancel(); t.engine.begin(300, 100, false); t.tick(1000, 100); t.expect({}); }
  { Test t; t.tap(100); t.tap(300); t.engine.cancel(); t.expect({Action::CameraOn, Action::CameraOff});
    assert(!t.engine.cameraRequested()); }
  { Test t(0xFFFFFF00u); t.press(0xFFFFFF60u); t.release(0xFFFFFFE0u); t.tick(267);
    t.expect({}); t.tick(268); t.expect({Action::MuteAtt}); }  // millis wrap
  std::cout << "19 gesture scenarios passed\n";
}
