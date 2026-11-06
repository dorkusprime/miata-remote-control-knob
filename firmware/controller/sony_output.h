#pragma once
#include <stdint.h>
#include "gesture_engine.h"

namespace knob {
// Sony learns these five distinct resistances. Only 64/128/192 have been
// measured so far; 96/160 are interpolated candidates, not receiver calibration.
inline bool sonyCode(Action action, uint8_t &code) {
  switch (action) {
    case Action::VolumeUp: code = 64; return true;
    case Action::VolumeDown: code = 96; return true;
    case Action::MuteAtt: code = 128; return true;
    case Action::NextTrack: code = 160; return true;
    case Action::PreviousTrack: code = 192; return true;
    default: return false;
  }
}

// Hardware callbacks keep timing/queue tests independent of Arduino.
class SonyOutput {
 public:
  using Pot = void (*)(uint8_t);
  using Switch = void (*)(bool);
  SonyOutput(Pot pot, Switch relay, Switch camera)
      : pot_(pot), relay_(relay), camera_(camera) {}
  static constexpr uint32_t kSettleMs = 1, kPulseMs = 55, kGapMs = 55;
  static constexpr uint32_t kMaxAgeMs = 750;
  static constexpr uint8_t kCapacity = 8;

  void enable() { release(); enabled_ = true; }
  void release() {
    enabled_ = false;
    size_ = head_ = 0;
    phase_ = Phase::Idle;
    relayOn_ = cameraOn_ = false;
    relay_(false); camera_(false);
  }
  bool submit(const Event &event, uint32_t now) {
    if (!enabled_ || !event.steps) return false;
    if (event.action == Action::CameraOn || event.action == Action::CameraOff) {
      cameraOn_ = event.action == Action::CameraOn;
      camera_(cameraOn_);
      return true;
    }
    uint8_t code;
    if (!sonyCode(event.action, code)) return false;
    // Bound backlog; reject the whole event if it cannot fit. Never replay
    // old detents indefinitely or allocate memory in the input callback.
    if (event.steps > static_cast<uint32_t>(kCapacity - size_)) return false;
    for (uint32_t i = 0; i < event.steps; ++i) {
      queue_[(head_ + size_) % kCapacity] = {code, now};
      ++size_;
    }
    return true;
  }
  void tick(uint32_t now) {
    if (!enabled_) return;
    if (phase_ == Phase::Settle && now - at_ >= kSettleMs) {
      if (now - activeAt_ >= kMaxAgeMs) {
        phase_ = Phase::Gap; at_ = now; return;
      }
      relay_(true); relayOn_ = true;
      phase_ = Phase::Pulse; at_ = now;
    } else if (phase_ == Phase::Pulse && now - at_ >= kPulseMs) {
      relay_(false); relayOn_ = false;
      phase_ = Phase::Gap; at_ = now;
    } else if (phase_ == Phase::Gap && now - at_ >= kGapMs) {
      phase_ = Phase::Idle;
    }
    if (phase_ != Phase::Idle) return;
    while (size_) {
      const Pending item = queue_[head_];
      head_ = (head_ + 1) % kCapacity; --size_;
      if (now - item.at >= kMaxAgeMs) continue;
      relay_(false); relayOn_ = false;
      pot_(item.code); code_ = item.code; activeAt_ = item.at;
      phase_ = Phase::Settle; at_ = now;
      break;
    }
  }
  bool enabled() const { return enabled_; }
  bool relayOn() const { return relayOn_; }
  bool cameraOn() const { return cameraOn_; }
  uint8_t code() const { return code_; }
  uint8_t pending() const { return size_; }
 private:
  enum class Phase { Idle, Settle, Pulse, Gap };
  struct Pending { uint8_t code; uint32_t at; };
  Pot pot_;
  Switch relay_, camera_;
  Pending queue_[kCapacity] = {};
  uint8_t size_ = 0, head_ = 0, code_ = 128;
  bool enabled_ = false, relayOn_ = false, cameraOn_ = false;
  Phase phase_ = Phase::Idle;
  uint32_t at_ = 0, activeAt_ = 0;
};
}  // namespace knob
