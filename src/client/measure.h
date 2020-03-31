#pragma once

#include <chrono>
#include <string>

namespace prototype {
  
class Measure;

struct Measurer {
  using Duration = std::chrono::high_resolution_clock::duration;
  
  explicit Measurer(const std::string& _name) : name{_name} {}
  
  const std::string name;
  int64_t count() const { return count_; }
  
  int64_t avg_ns() const {
    if (!count_) return 0;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(total_ / count_).count();
  }
  
  auto total_ns() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(total_).count();
  }
  
private:
  void start() {
    started_at_ = std::chrono::high_resolution_clock::now();
  }
  
  void stop() {
    auto duration = std::chrono::high_resolution_clock::now() - started_at_;
    total_ += duration;
    ++count_;
  }
  
  Duration total_ = {};
  std::chrono::high_resolution_clock::time_point started_at_ = {};
  uint64_t count_ = 0;
  
  friend class Measure;
};

class Measure {
public:
  Measure(Measurer& measurer) : measurer_{measurer} { measurer_.start(); }
  ~Measure() { measurer_.stop(); }
  
private:
  Measurer& measurer_;
};

}

