#pragma once

#include<atomic>
#include<thread>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "clock.hpp"

class RateLimiter {
  public:
  virtual ~RateLimiter() = default;
  virtual int request() = 0;
};


class NaiveTokenBucket: RateLimiter {
  private:
  std::atomic<int> bucket;
  size_t size;
  size_t refill_rate; // per second
  std::atomic<bool> running{true};
  std::thread filler;

  public:
    NaiveTokenBucket(size_t _size, size_t _refill_rate): 
      bucket(_size),
      size(_size),
      refill_rate(_refill_rate)
  {
      filler = std::thread([this](){
            const auto period = std::chrono::duration<double>(1.0 / static_cast<double>(refill_rate));
            while (running) {
              std::this_thread::sleep_for(period);
              if (static_cast<size_t>(bucket.load()) < size) ++bucket;
            }
          });
    }

    ~NaiveTokenBucket() {
      running = false;
      filler.join();
      // if (filler.joinable()) filler.join();
    }
    
    int request() {
      int old = bucket.load();
      while (old > 0) {
        if (bucket.compare_exchange_strong(old, old - 1)) {
          return 0;
        }
      }
      return -1;
    }

};
 
using namespace std::chrono;

class TokenBucketSingle: RateLimiter {
  private:
  int tokens;
  Clock& clock;
  Clock::time_point last_request;
  size_t size;
  size_t refill_rate;

  public:
    TokenBucketSingle(size_t _size, size_t _refill_rate, Clock& _clock):
      tokens(_size),
      clock(_clock),
      size(_size),
      refill_rate(_refill_rate) // tokens per secon
  {
    last_request = clock.now();
  }
    
    int request() {
      Clock::time_point now = clock.now();
      tokens = std::max<double>(
          size,
          tokens + (static_cast<duration<double>>(now - last_request).count())*refill_rate
      );
      if (tokens > 0) {
        tokens -= 1;
        return 0;
      } else {
        return -1;
      }
    }

};



class TokenBucketMulti: RateLimiter {
  private:
  int tokens;
  Clock& clock;
  Clock::time_point last_request;
  size_t size;
  size_t refill_rate;

  public:
    TokenBucketMulti(size_t _size, size_t _refill_rate, Clock& _clock):
      tokens(_size),
      clock(_clock),
      size(_size),
      refill_rate(_refill_rate) // tokens per secon
  {
    last_request = clock.now();
    // const auto period = std::chrono::duration<double>(1.0 / static_cast<double>(refill_rate));
  }
    
    int request() { // broken
      Clock::time_point now = clock.now();
      if (tokens > 0) {
        tokens -= 1;
      }
      tokens = std::max<double>(
          size,
          tokens + (static_cast<duration<double>>(now - last_request).count())*refill_rate
      );
      return 0;
    }

};



