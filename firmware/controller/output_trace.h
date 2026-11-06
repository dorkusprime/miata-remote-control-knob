#pragma once
#include <stdint.h>

namespace knob {
struct OutputEdge {
  uint32_t at, inputAge;
  uint8_t code, pending;
  bool on;
};
// Diagnostic storage only. Recording never performs USB I/O or delays output.
class OutputTrace {
 public:
  static constexpr uint8_t kCapacity = 32;
  void record(const OutputEdge &edge) {
    if (edge.on == lastOn_) return;
    lastOn_ = edge.on;
    if (size_ == kCapacity) { ++lost_; return; }
    edges_[(head_ + size_) % kCapacity] = edge;
    ++size_;
  }
  const OutputEdge *front() const { return size_ ? &edges_[head_] : nullptr; }
  void pop() { if (size_) { head_ = (head_ + 1) % kCapacity; --size_; } }
  void discard() { head_ = size_ = 0; lost_ = 0; } // Keep actual relay state.
  uint32_t lost() const { return lost_; }
 private:
  OutputEdge edges_[kCapacity] = {};
  uint32_t lost_ = 0;
  uint8_t head_ = 0, size_ = 0;
  bool lastOn_ = false;
};
}
