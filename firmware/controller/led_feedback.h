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
  void clear() { duration_ = 0; }

  void onGesture(const Event &event, uint32_t now) {
    switch (event.action) {
      case Action::VolumeUp:
      case Action::VolumeDown: pulse(kVolumeWhite, now, 120); break;
      case Action::MuteAtt: pulse(kClickGreen, now, 180); break;
      case Action::CameraOn:
      case Action::CameraOff: pulse(kCameraAmber, now, 200); break;
      // Held track turns already have the observed-button purple indication.
      case Action::NextTrack:
      case Action::PreviousTrack:
      case Action::LongPressIgnored: break;
    }
  }

  Color colorAt(uint32_t now, bool buttonHeld, bool connected) const {
    if (!connected) return kOff;
    if (duration_ && static_cast<uint32_t>(now - started_) < duration_) return pulseColor_;
    return buttonHeld ? kHeldPurple : kIdleBlue;
  }

 private:
  void pulse(Color color, uint32_t now, uint32_t duration) {
    pulseColor_ = color; started_ = now; duration_ = duration;
  }
  Color pulseColor_ = kOff;
  uint32_t started_ = 0, duration_ = 0;
};
}  // namespace knob
