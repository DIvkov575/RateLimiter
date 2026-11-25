#pragma once

#include <deque>
#include <mutex>
#include <optional>
#include <vector>
#include <utility>

namespace myqueue {

template<typename T>
class Queue {
  public:
  virtual ~Queue() = default;
  virtual int publish(T&&) = 0;
  virtual std::optional<T> consume() = 0;
};


template<typename T>
class NaiveQueue : public Queue<T> {
  std::mutex m;
  std::deque<T> deq;

  public:
  NaiveQueue() = default;

  int publish(T&& item) override {
    std::lock_guard<std::mutex> lock(m);
    deq.push_back(std::move(item));
    return 0;
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

template<typename T, size_t  N>
class SPSCQueue : public myqueue::Queue<T> {
  T arr[N];
  std::atomic<size_t> front_{0};
  std::atomic<size_t> back_{0};


  public:
  SPSCQueue() { }

  int publish(T&& item) override {
    size_t front = front_.load();    
    if (front - back_ >= N) {
      return -1;
    }

    arr[front%N] = std::forward<T>(item);
    front_.fetch_add(1);
    return 0;
  }

  std::optional<T> consume() override {
    size_t back= back_.load();    
    if (front_ == back) return std::nullopt;

    T item = std::move(arr[back%N]);
    back_.fetch_add(1); // can be cheaper
    return item;

  }
};




