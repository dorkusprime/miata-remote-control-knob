#pragma once
#include <stdint.h>
namespace knob {
// Let the host finish the command and close its CDC handle before rebooting.
// A terminal user can also request boot without closing: bounded fallback 1s.
class BootRequest {
 public:
  void request(uint32_t now) { pending_ = true; at_ = now; }
  bool pending() const { return pending_; }
  bool ready(uint32_t now, bool serialOpen) const {
    const uint32_t elapsed = now - at_;
    return pending_ && elapsed >= 100 && (!serialOpen || elapsed >= 1000);
  }
 private:
  bool pending_ = false;
  uint32_t at_ = 0;
};
}  // namespace knob
