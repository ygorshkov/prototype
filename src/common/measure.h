#pragma once

#include <chrono>
#include <string>
#include <atomic>

namespace prototype {
  
class Measure;

struct Measurer {
  using Duration = std::chrono::high_resolution_clock::duration;

  explicit Measurer(const std::string& _name) : name{_name} {}

  const std::string name;
  uint64_t count() const { return count_; }

  uint64_t avg_ns() const {
    if (!count_) return 0;
    return total_ / count_;
  }

  uint64_t total_ns() const {
    return total_;
  }

private:
  void add(uint64_t ns) {
    total_ += ns;
    ++count_;
  }

  std::atomic<uint64_t> total_ = 0;
  std::atomic<uint64_t> count_ = 0;
  
  friend class Measure;
};

class Measure {
public:
  explicit Measure(Measurer* measurer) : measurer_{measurer} { if (measurer_) start(); }
  ~Measure() { if (measurer_) stop(); }

private:
  void start() {
    started_at_ = std::chrono::high_resolution_clock::now();
  }

  void stop() {
    auto duration = std::chrono::high_resolution_clock::now() - started_at_;
    measurer_->add(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
  }

  Measurer* measurer_;
  std::chrono::high_resolution_clock::time_point started_at_ = {};
};

}

