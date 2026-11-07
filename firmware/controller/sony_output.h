#pragma once
#include <stdint.h>
#include <string.h>
#include "gesture_engine.h"

namespace knob {
// Six user-learned Sony codes. Mute 230 was accepted through a serial probe;
// the remaining five codes were also verified with their physical gestures.
struct CommandSetting { Action action; const char *name; uint8_t code; };
constexpr CommandSetting kDefaultCommands[] = {
  {Action::VolumeUp, "volume_up", 255}, {Action::VolumeDown, "volume_down", 0},
  {Action::PlayPause, "play_pause", 238}, {Action::MuteAtt, "mute_att", 230}, {Action::NextTrack, "next_track", 248},
  {Action::PreviousTrack, "previous_track", 215}
};
constexpr uint8_t kCommandCount = sizeof(kDefaultCommands) / sizeof(kDefaultCommands[0]);
inline int commandIndex(Action action) {
  for (uint8_t i = 0; i < kCommandCount; ++i)
    if (kDefaultCommands[i].action == action) return i;
  return -1;
}
inline bool sonyCode(Action action, uint8_t &code) {
  const int index = commandIndex(action);
  if (index < 0) return false;
  code = kDefaultCommands[index].code;
  return true;
}
inline bool parsePotCode(const char *digits, uint8_t &code) {
  if (!*digits) return false;
  unsigned value = 0;
  for (const char *p = digits; *p; ++p) {
    if (*p < '0' || *p > '9') return false;
    value = value * 10 + static_cast<unsigned>(*p - '0');
    if (value > 255) return false;
  }
  code = static_cast<uint8_t>(value); return true;
}
// Parse the payload after "set "; reject partial names, extra tokens and overflow.
inline bool parseCodeSetting(const char *text, Action &action, uint8_t &code) {
  for (const auto &entry : kDefaultCommands) {
    const size_t length = strlen(entry.name);
    if (strncmp(text, entry.name, length) || text[length] != ' ') continue;
    if (!parsePotCode(text + length + 1, code)) return false;
    action = entry.action; return true;
  }
  return false;
}

// Parse the payload after "timing "; no trailing tokens or implicit clamping.
inline bool parsePulseTiming(const char *text, uint8_t &press, uint8_t &gap) {
  const char *space = strchr(text, ' ');
  if (!space || space == text || space - text > 3) return false;
  char first[4] = {};
  memcpy(first, text, space - text);
  return parsePotCode(first, press) && parsePotCode(space + 1, gap) &&
         press >= 5 && press <= 250 && gap >= 5 && gap <= 250;
}

// Hardware callbacks keep timing/queue tests independent of Arduino.
class SonyOutput {
 public:
  using Pot = void (*)(uint8_t);
  using Switch = void (*)(bool);
  SonyOutput(Pot pot, Switch relay, Switch camera)
      : pot_(pot), relay_(relay), camera_(camera) {
    for (uint8_t i = 0; i < kCommandCount; ++i) codes_[i] = kDefaultCommands[i].code;
  }
  static constexpr uint32_t kSettleMs = 1, kPulseMs = 55, kGapMs = 55;
  static constexpr uint32_t kMaxAgeMs = 750, kLearnMs = 6000;
  static constexpr uint8_t kCapacity = 8;

  void enable() { release(); enabled_ = true; }
  void release() {
    enabled_ = learning_ = activeVolume_ = false;
    pulseMs_ = normalPulseMs_;
    size_ = head_ = 0;
    phase_ = Phase::Idle;
    relayOn_ = cameraOn_ = false;
    relay_(false); camera_(false);
  }
  bool codeFor(Action action, uint8_t &code) const {
    const int index = commandIndex(action);
    if (index < 0) return false;
    code = codes_[index]; return true;
  }
  bool setCode(Action action, unsigned value) {
    const int index = commandIndex(action);
    if (index < 0 || value > 255) return false;
    const bool wasEnabled = enabled_;
    release();  // Discard old queued codes and disconnect before reconfiguration.
    codes_[index] = static_cast<uint8_t>(value);
    enabled_ = wasEnabled;
    return true;
  }
  bool setTiming(unsigned press, unsigned gap) {
    if (press < 5 || press > 250 || gap < 5 || gap > 250) return false;
    const bool wasEnabled = enabled_;
    release();
    normalPulseMs_ = pulseMs_ = press; normalGapMs_ = gap;
    enabled_ = wasEnabled;
    return true;
  }
  uint32_t pressMs() const { return normalPulseMs_; }
  uint32_t gapMs() const { return normalGapMs_; }
  bool learn(Action action, uint32_t now) {
    uint8_t code;
    if (!enabled_ || !codeFor(action, code)) return false;
    return learnCode(code, now);
  }
  bool learnCode(unsigned value, uint32_t now) {
    if (!enabled_ || value > 255) return false;
    const uint8_t code = static_cast<uint8_t>(value);
    release();
    enabled_ = learning_ = true;
    pulseMs_ = kLearnMs;
    pot_(code); code_ = code; activeAt_ = at_ = now;
    phase_ = Phase::Settle;
    return true;
  }
  bool learning() const { return learning_; }
  bool submit(const Event &event, uint32_t now) {
    if (!enabled_ || learning_ || !event.steps) return false;
    if (event.action == Action::CameraOn || event.action == Action::CameraOff) {
      cameraOn_ = event.action == Action::CameraOn;
      camera_(cameraOn_);
      return true;
    }
    uint8_t code;
    if (!codeFor(event.action, code)) return false;
    if (event.steps > 65536 || event.steps > UINT32_MAX - pending()) return false;
    const bool volume = event.action == Action::VolumeUp || event.action == Action::VolumeDown;
    // Compress consecutive same-direction volume into a counted run: a quick
    // spin consumes one slot, retaining every detent rather than eight only.
    if (volume) {
      if (size_) {
        Pending &tail = queue_[(head_ + size_ - 1) % kCapacity];
        if (tail.volume && tail.code == code) { tail.steps += event.steps; return true; }
      }
      if (size_ == kCapacity) return false;
      queue_[(head_ + size_) % kCapacity] = {code, now, event.steps, true};
      ++size_;
    } else {
      if (event.steps > static_cast<uint32_t>(kCapacity - size_)) return false;
      for (uint32_t i = 0; i < event.steps; ++i) {
        queue_[(head_ + size_) % kCapacity] = {code, now, 1, false};
        ++size_;
      }
    }
    return true;
  }
  void tick(uint32_t now) {
    if (!enabled_) return;
    if (phase_ == Phase::Settle && now - at_ >= kSettleMs) {
      if (!activeVolume_ && now - activeAt_ >= kMaxAgeMs) {
        if (learning_) { release(); return; }
        phase_ = Phase::Gap; at_ = now; return;
      }
      relay_(true); relayOn_ = true;
      phase_ = Phase::Pulse; at_ = now;
    } else if (phase_ == Phase::Pulse && now - at_ >= pulseMs_) {
      if (learning_) { release(); return; }
      relay_(false); relayOn_ = false;
      phase_ = Phase::Gap; at_ = now;
    } else if (phase_ == Phase::Gap && now - at_ >= normalGapMs_) {
      phase_ = Phase::Idle;
    }
    if (phase_ != Phase::Idle) return;
    while (size_) {
      const Pending item = queue_[head_];
      if (!item.volume && now - item.at >= kMaxAgeMs) {
        head_ = (head_ + 1) % kCapacity; --size_; continue;
      }
      if (--queue_[head_].steps == 0) { head_ = (head_ + 1) % kCapacity; --size_; }
      relay_(false); relayOn_ = false;
      pot_(item.code); code_ = item.code; activeAt_ = item.at; activeVolume_ = item.volume;
      phase_ = Phase::Settle; at_ = now;
      break;
    }
  }
  bool enabled() const { return enabled_; }
  bool relayOn() const { return relayOn_; }
  bool cameraOn() const { return cameraOn_; }
  uint8_t code() const { return code_; }
  uint32_t pending() const {
    uint32_t count = 0;
    for (uint8_t i = 0; i < size_; ++i) count += queue_[(head_ + i) % kCapacity].steps;
    return count;
  }
 private:
  enum class Phase { Idle, Settle, Pulse, Gap };
  struct Pending { uint8_t code; uint32_t at, steps; bool volume; };
  Pot pot_;
  Switch relay_, camera_;
  Pending queue_[kCapacity] = {};
  uint8_t codes_[kCommandCount] = {};
  uint8_t size_ = 0, head_ = 0, code_ = 128;
  bool activeVolume_ = false, enabled_ = false, learning_ = false, relayOn_ = false, cameraOn_ = false;
  Phase phase_ = Phase::Idle;
  uint32_t at_ = 0, activeAt_ = 0, normalPulseMs_ = kPulseMs, normalGapMs_ = kGapMs, pulseMs_ = kPulseMs;
};
}  // namespace knob
