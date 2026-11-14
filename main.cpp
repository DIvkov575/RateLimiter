#include<atomic>
#include<thread>
#include <chrono>
#include <iostream>


using namespace std;


class RateLimiter {
  public:
  virtual ~RateLimiter() = default;
  virtual int request() = 0;
};


class TokenBucket: RateLimiter {
  private:
  std::atomic<int> bucket;
  size_t size;
  size_t refill_rate; // per second
  std::atomic<bool> running{true};
  std::thread filler;

  public:
    TokenBucket(size_t _size, size_t _refill_rate): 
      bucket(_size),
      size(_size),
      refill_rate(_refill_rate)
  {
      filler = std::thread([this](){
            const auto period = chrono::duration<double>(1.0 / static_cast<double>(refill_rate));
            while (running) {
              std::this_thread::sleep_for(period);
              if (static_cast<size_t>(bucket.load()) < size) ++bucket;
            }
          });
    }

    ~TokenBucket() {
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


  private:


};




int main() {
  TokenBucket rl(100, 1);

  auto start = chrono::steady_clock::now();
  volatile int total1 = 0;
  for (size_t i = 0; i < 1<<30; ++i) {
    total1 += rl.request();
  }
  auto end = chrono::steady_clock::now();
  auto us = chrono::duration_cast<chrono::microseconds>(end - start).count();

  cout << us << " " << total1 << endl;



  start = chrono::steady_clock::now();
  volatile int total2 = 0;
  for (size_t i = 0; i < 1<<30; ++i) {
    total2 += 0;
  }
  end = chrono::steady_clock::now();
  us = chrono::duration_cast<chrono::microseconds>(end - start).count();

  cout << us << " " << total2<< endl;


}




