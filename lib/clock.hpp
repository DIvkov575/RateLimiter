#pragma once

#include <chrono>

class Clock {
  public:
  using time_point = std::chrono::system_clock::time_point;
  using duration   = std::chrono::system_clock::duration;

  virtual ~Clock() = default;
  virtual time_point now() = 0;
};


class SystemClock : public Clock {
  public:
  time_point now() override { return std::chrono::system_clock::now(); }
};


class FakeClock : public Clock {
  time_point current;
  duration step_size;

  public:
  explicit FakeClock(duration _step = std::chrono::seconds(1), time_point _start = {}):
    current(_start),
    step_size(_step)
  {}

  time_point now() override { return current; }

  void step(int n = 1) { current += n * step_size; }
  void advance(duration d) { current += d; }
};
