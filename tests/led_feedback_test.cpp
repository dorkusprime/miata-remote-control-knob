#include "../firmware/controller/led_feedback.h"
#include <cassert>
#include <iostream>
using namespace knob;

int main() {
  { LedFeedback led;
    assert(led.colorAt(0, false, true) == Color({0, 0, 255}));
    assert(led.colorAt(1, true, true) == kHeldPurple);
    assert(led.colorAt(2, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::VolumeUp, 3}, 100);
    assert(led.colorAt(219, false, true) == kVolumeWhite);
    assert(led.colorAt(220, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::VolumeDown, 1}, 100);
    led.onGesture({Action::VolumeDown, 1}, 200);
    assert(led.colorAt(250, false, true) == kVolumeWhite);
    assert(led.colorAt(320, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::MuteAtt, 1}, 100);
    assert(led.colorAt(279, false, true) == kClickGreen);
    assert(led.colorAt(280, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::CameraOn, 1}, 100);
    assert(led.colorAt(299, false, true) == kCameraAmber);
    assert(led.colorAt(300, false, true) == kIdleBlue);
    assert(led.colorAt(10000, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::CameraOff, 1}, 100);
    assert(led.colorAt(150, false, true) == kCameraAmber);
    assert(led.colorAt(300, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::MuteAtt, 1}, 100);
    assert(led.colorAt(120, true, true) == kClickGreen);
    assert(led.colorAt(280, true, true) == kHeldPurple); }
  { LedFeedback led; led.onGesture({Action::CameraOn, 1}, 100);
    assert(led.colorAt(150, false, false) == kOff);
    led.clear(); assert(led.colorAt(160, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::VolumeUp, 1}, 0xFFFFFFE0u);
    assert(led.colorAt(50, false, true) == kVolumeWhite);
    assert(led.colorAt(88, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::NextTrack, 1}, 100);
    assert(led.colorAt(110, true, true) == kHeldPurple);
    led.onGesture({Action::PreviousTrack, 1}, 120);
    assert(led.colorAt(130, false, true) == kIdleBlue);
    led.onGesture({Action::LongPressIgnored, 1}, 140);
    assert(led.colorAt(150, false, true) == kIdleBlue); }
  { LedFeedback led; led.setLearning(true, 100);
    assert(led.colorAt(100, false, true) == kCameraAmber);
    assert(led.colorAt(250, false, true) == kOff);
    assert(led.colorAt(400, false, true) == kCameraAmber);
    assert(led.colorAt(850, false, true) == kOff);
    assert(led.colorAt(1000, false, true) == kCameraAmber);
    led.learningCommand(1100);
    assert(led.colorAt(1100, false, true) == kClickGreen);
    assert(led.colorAt(1250, false, true) == kOff);
    assert(led.colorAt(1400, false, true) == kClickGreen);
    assert(led.colorAt(1700, false, true) == kCameraAmber);
    led.setLearning(false, 1800);
    assert(led.colorAt(1800, false, true) == kIdleBlue);
    assert(led.colorAt(1950, false, true) == kOff);
    assert(led.colorAt(2700, false, true) == kIdleBlue); }
  { LedFeedback led; led.setLearning(true, 0xfffffff0u);
    assert(led.colorAt(134, false, true) == kOff);
    led.clear(); assert(led.colorAt(200, false, true) == kIdleBlue); }
  { LedFeedback led; led.onGesture({Action::PlayPause, 1}, 100);
    assert(led.colorAt(279, false, true) == kClickGreen);
    assert(led.colorAt(280, false, true) == kIdleBlue); }
  std::cout << "13 LED feedback scenarios passed\n";
}
