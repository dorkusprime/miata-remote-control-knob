#pragma once
#include "sony_output.h"

namespace knob {
struct FlickSettings {
  bool enabled = true;
  uint32_t firstMin = 1, firstMax = 6, secondMin = 1, secondMax = 6, thirdMin = 1;
  uint32_t gapMs = 250, windowMs = 700, quietMs = 120, steps = 10;
};

// Runtime tuning only. Parse into a copy so malformed commands cannot partially
// update settings or suspend outputs. Ranges bound arithmetic and detection time.
inline bool parseFlickSetting(const char *text, FlickSettings &settings) {
  FlickSettings next = settings;
  if (!strcmp(text, "on")) next.enabled = true;
  else if (!strcmp(text, "off")) next.enabled = false;
  else {
    const char *space = strchr(text, ' ');
    if (!space || space == text || space - text > 11) return false;
    char key[12] = {};
    memcpy(key, text, space - text);
    uint32_t values[2] = {};
    unsigned count = 0;
    const char *p = space + 1;
    while (*p) {
      if (count == 2 || *p < '0' || *p > '9') return false;
      uint32_t value = 0;
      do {
        value = value * 10 + static_cast<unsigned>(*p++ - '0');
        if (value > 5000) return false;
      } while (*p >= '0' && *p <= '9');
      values[count++] = value;
      if (!*p) break;
      if (*p++ != ' ' || !*p) return false;
    }
    if (!count) return false;
    const uint32_t a = values[0], b = values[1];
    if (count == 2) {
      if (a < 1 || b < a || b > 100) return false;
      if (!strcmp(key, "first")) { next.firstMin = a; next.firstMax = b; }
      else if (!strcmp(key, "second")) { next.secondMin = a; next.secondMax = b; }
      else return false;
    } else {
      if (!strcmp(key, "third")) { if (a < 1 || a > 100) return false; next.thirdMin = a; }
      else if (!strcmp(key, "steps")) { if (a < 1 || a > 100) return false; next.steps = a; }
      else {
        if (a < 10 || a > 5000) return false;
        if (!strcmp(key, "gap")) next.gapMs = a;
        else if (!strcmp(key, "window")) next.windowMs = a;
        else if (!strcmp(key, "quiet")) next.quietMs = a;
        else return false;
      }
    }
  }
  settings = next;
  return true;
}

class BulkVolume {
 public:
  const FlickSettings &settings() const { return settings_; }
  void configure(const FlickSettings &settings, SonyOutput &output, uint32_t now) {
    interrupt(output, now); settings_ = settings;
  }
  // Fault/maintenance reset follows output.release(); no hardware operation here.
  void reset() { run_ = 0; bulk_ = recognized_ = tail_ = false; }
  void interrupt(SonyOutput &output, uint32_t now) {
    if (bulk_) output.cancelVolume(now);
    reset();
  }
  bool finishingFlick(const Event &event, uint32_t now) const {
    return bulk_ && now - lastAt_ < settings_.quietMs &&
        ((direction_ > 0 && event.action == Action::VolumeUp) ||
         (direction_ < 0 && event.action == Action::VolumeDown));
  }
  bool consumedTail() const { return tail_; }
  bool recognized() const { return recognized_; }
  int64_t correction() const { return correction_; }
  int64_t sentDuring() const { return sentDuring_; }
  int direction() const { return direction_; }

  bool submit(const Event &event, uint32_t now, SonyOutput &output) {
    recognized_ = tail_ = false;
    if (!output.enabled() || output.learning()) { reset(); return false; }
    const bool volume = event.action == Action::VolumeUp || event.action == Action::VolumeDown;
    if (!volume) { interrupt(output, now); return output.submit(event, now); }
    if (!event.steps || event.steps > 65536) return false;
    const int direction = event.action == Action::VolumeUp ? 1 : -1;
    if (!settings_.enabled) return output.submit(event, now);

    if (bulk_) {
      if (direction == direction_ && now - lastAt_ < settings_.quietMs) {
        lastAt_ = now; tail_ = true; return true;  // Tail of final flick, including extra detents in later samples.
      }
      interrupt(output, now);  // Opposite direction, or fresh turn after quiet.
    }
    if (run_ && (now - lastAt_ > settings_.gapMs || now - beganAt_ > settings_.windowMs)) run_ = 0;
    if (!run_) start(direction, event.steps, now, output);
    else if (direction == direction_) count_ += event.steps;
    else {
      const uint32_t minimum = run_ == 1 ? settings_.firstMin : settings_.secondMin;
      const uint32_t maximum = run_ == 1 ? settings_.firstMax : settings_.secondMax;
      if (run_ < 3 && count_ >= minimum && count_ <= maximum) {
        ++run_; direction_ = direction; count_ = event.steps;
      } else start(direction, event.steps, now, output);
    }
    lastAt_ = now;
    if (run_ == 3 && count_ >= settings_.thirdMin) {
      // Snapshot includes the original queue's intended position. All outstanding
      // volume is now replaced by the target; already-started pulses finish.
      const int64_t target = baseline_ + direction * static_cast<int64_t>(settings_.steps);
      const int64_t remaining = target - output.volumeSent();
      output.cancelVolume(now);
      sentDuring_ = output.volumeSent() - beganSent_;
      correction_ = remaining;
      run_ = 0; bulk_ = recognized_ = true;
      if (!remaining) return true;
      const int64_t magnitude = remaining > 0 ? remaining : -remaining;
      if (magnitude > 65536) { bulk_ = false; return false; }
      const uint32_t steps = static_cast<uint32_t>(magnitude);
      const bool accepted = output.submit({remaining > 0 ? Action::VolumeUp : Action::VolumeDown, steps}, now);
      if (!accepted) bulk_ = false;
      return accepted;
    }
    // Oversized first/second run can never qualify. Keep its ordinary behavior;
    // the next direction begins a new candidate instead of slicing a large spin.
    return output.submit(event, now);
  }
 private:
  void start(int direction, uint32_t steps, uint32_t now, const SonyOutput &output) {
    run_ = 1; direction_ = direction; count_ = steps; beganAt_ = now;
    baseline_ = output.projectedVolume(); beganSent_ = output.volumeSent();
  }
  FlickSettings settings_;
  uint8_t run_ = 0;
  int direction_ = 0;
  uint32_t count_ = 0, beganAt_ = 0, lastAt_ = 0;
  int64_t baseline_ = 0, beganSent_ = 0;
  int64_t correction_ = 0, sentDuring_ = 0;
  bool bulk_ = false, recognized_ = false, tail_ = false;
};
}  // namespace knob
