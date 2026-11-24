#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace myqueue {

template<typename T>
class Queue {
  public:
  virtual ~Queue() = default;
  virtual void publish(T&&) = 0;
  virtual std::optional<T> consume() = 0;
};

template<typename T>
class NaiveQueue : public Queue<T> {
  std::mutex m;
  std::deque<T> deq;

  public:
  NaiveQueue() = default;

  void publish(T&& item) override {
    std::lock_guard<std::mutex> lock(m);
    deq.push_back(std::move(item));
  }

  std::optional<T> consume() override {
    std::lock_guard<std::mutex> lock(m);
    if (deq.empty()) return std::nullopt;
    T item = std::move(deq.front());
    deq.pop_front();
    return item;
  }
};

}
