#pragma once
#include <stdint.h>

namespace knob {
enum class Action { VolumeUp, VolumeDown, MuteAtt, NextTrack, PreviousTrack,
                    CameraOn, CameraOff, LongPressIgnored };
struct Event { Action action; uint32_t steps; };
struct Timing {
  uint32_t debounceMs = 30;
  uint32_t doubleClickMs = 300;
  uint32_t longPressMs = 700;
};

// No Arduino or hardware dependencies. All outputs are intentions, never GPIO.
class Gestures {
 public:
  using Sink = void (*)(const Event &, void *);
  explicit Gestures(Sink sink, void *context = nullptr, Timing timing = {})
      : sink_(sink), context_(context), timing_(timing) {}

  void begin(uint32_t now, uint16_t count, bool down) {
    cancel();
    initialized_ = true;
    lastCount_ = count;
    rawDown_ = stableDown_ = down;
    rawChangedAt_ = pressedAt_ = now;
    armed_ = !down;  // Boot/reconnect with button held: wait for release.
  }

  void cancel() {
    if (camera_) emit(Action::CameraOff);
    camera_ = false;
    initialized_ = pendingClick_ = secondClick_ = usedPress_ = false;
    deferredDelta_ = 0;
  }

  bool cameraRequested() const { return camera_; }
  bool buttonHeld() const { return initialized_ && stableDown_; }

  void update(uint32_t now, uint16_t count, bool down) {
    if (!initialized_) { begin(now, count, down); return; }
    const uint16_t wrapped = static_cast<uint16_t>(count - lastCount_);
    const int32_t delta = wrapped <= 32767 ? wrapped : static_cast<int32_t>(wrapped) - 65536;
    lastCount_ = count;
    if (down != rawDown_) { rawDown_ = down; rawChangedAt_ = now; }

    if (!armed_) {
      if (rawDown_ != stableDown_ && elapsed(now, rawChangedAt_) >= timing_.debounceMs)
        stableDown_ = rawDown_;
      if (!rawDown_ && !stableDown_) armed_ = true;
      return;
    }

    // Preserve detents during debounce. On release they belong to the held turn;
    // on press they belong to the newly pressed gesture.
    if (rawDown_ != stableDown_) {
      deferredDelta_ += delta;
      if (elapsed(now, rawChangedAt_) >= timing_.debounceMs) {
        if (rawDown_) {
          stableDown_ = true;
          press(rawChangedAt_);
          turn(deferredDelta_, true);
        } else {
          turn(deferredDelta_, true);
          stableDown_ = false;
          release(rawChangedAt_);
        }
        deferredDelta_ = 0;
      }
    } else {
      turn(delta + deferredDelta_, stableDown_);
      deferredDelta_ = 0;
    }
    if (pendingClick_ && !rawDown_ && !stableDown_ &&
        elapsed(now, releasedAt_) >= timing_.doubleClickMs) {
      pendingClick_ = false;
      emit(Action::MuteAtt);
    }
  }

 private:
  static uint32_t elapsed(uint32_t now, uint32_t then) { return now - then; }
  void emit(Action action, uint32_t steps = 1) {
    if (sink_) sink_({action, steps}, context_);
  }
  void press(uint32_t when) {
    secondClick_ = pendingClick_ && elapsed(when, releasedAt_) <= timing_.doubleClickMs;
    if (pendingClick_ && !secondClick_) emit(Action::MuteAtt);
    pendingClick_ = false;
    usedPress_ = false;
    pressedAt_ = when;
  }
  void release(uint32_t when) {
    if (usedPress_) { secondClick_ = false; return; }
    if (elapsed(when, pressedAt_) >= timing_.longPressMs) {
      secondClick_ = false;
      emit(Action::LongPressIgnored);
    } else if (secondClick_) {
      secondClick_ = false;
      camera_ = !camera_;
      emit(camera_ ? Action::CameraOn : Action::CameraOff);
    } else {
      pendingClick_ = true;
      releasedAt_ = when;
    }
  }
  void turn(int32_t delta, bool down) {
    if (!delta) return;
    // Turning consumes a pending click; held turns never also trigger mute/camera.
    pendingClick_ = secondClick_ = false;
    if (down) usedPress_ = true;
    const uint32_t steps = delta > 0 ? delta : -delta;
    emit(down ? (delta > 0 ? Action::NextTrack : Action::PreviousTrack)
              : (delta > 0 ? Action::VolumeUp : Action::VolumeDown), steps);
  }

  Sink sink_;
  void *context_;
  Timing timing_;
  bool initialized_ = false, armed_ = false, rawDown_ = false, stableDown_ = false;
  bool pendingClick_ = false, secondClick_ = false, usedPress_ = false, camera_ = false;
  uint16_t lastCount_ = 0;
  int32_t deferredDelta_ = 0;
  uint32_t rawChangedAt_ = 0, pressedAt_ = 0, releasedAt_ = 0;
};
}  // namespace knob
