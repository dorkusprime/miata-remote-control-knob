#pragma once
#include "gesture_engine.h"

namespace knob {
struct Color {
  uint8_t red, green, blue;
  constexpr bool operator==(const Color &other) const {
    return red == other.red && green == other.green && blue == other.blue;
  }
};

// Full-intensity colors for underglow beneath an opaque knob. No global dimmer.
constexpr Color kIdleBlue{0, 0, 255};
constexpr Color kHeldPurple{255, 0, 255};
constexpr Color kVolumeWhite{255, 255, 255};
constexpr Color kClickGreen{0, 255, 0};
constexpr Color kCameraAmber{255, 96, 0};
constexpr Color kOff{0, 0, 0};

class LedFeedback {
 public:
  void clear() { duration_ = 0; learning_ = flashing_ = false; }
  void setLearning(bool on, uint32_t now) {
    learning_ = on; flash(on ? kCameraAmber : kIdleBlue, now, 3);
  }
  void learningCommand(uint32_t now) { flash(kClickGreen, now, 2); }

  void onGesture(const Event &event, uint32_t now) {
    if (learning_) return;
    switch (event.action) {
      case Action::VolumeUp:
      case Action::VolumeDown: pulse(kVolumeWhite, now, 120); break;
      case Action::PlayPause:
      case Action::MuteAtt: pulse(kClickGreen, now, 180); break;
      case Action::CameraOn:
      case Action::CameraOff: pulse(kCameraAmber, now, 200); break;
      // Held track turns already have the observed-button purple indication.
      case Action::NextTrack:
      case Action::PreviousTrack:
      case Action::LongPressIgnored:
      case Action::LearningToggle: break;
    }
  }

  Color colorAt(uint32_t now, bool buttonHeld, bool connected) const {
    if (!connected) return kOff;
    const uint32_t age = now - started_;
    if (duration_ && age < duration_)
      return flashing_ && (age / 150) % 2 ? kOff : pulseColor_;
    return buttonHeld ? kHeldPurple : (learning_ ? kCameraAmber : kIdleBlue);
  }

 private:
  void pulse(Color color, uint32_t now, uint32_t duration) {
    pulseColor_ = color; started_ = now; duration_ = duration; flashing_ = false;
  }
  void flash(Color color, uint32_t now, uint8_t count) {
    pulse(color, now, static_cast<uint32_t>(count) * 300); flashing_ = true;
  }
  bool learning_ = false, flashing_ = false;
  Color pulseColor_ = kOff;
  uint32_t started_ = 0, duration_ = 0;
};
}  // namespace knob
