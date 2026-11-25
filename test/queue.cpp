#include "../lib/queue.hpp"
#include<cassert>
#include<memory>
#include<thread>
#include<vector>
#include<atomic>




void check_naive_queue() {
  myqueue::NaiveQueue<int> q;
  assert(!q.consume().has_value());
  for (int i = 0; i < 5; ++i) { q.publish(int(i)); }
  for (int i = 0; i < 5; ++i) { assert(q.consume() == i); }
  assert(!q.consume().has_value());
}



void check_naive_queue_move_only() {
  myqueue::NaiveQueue<std::unique_ptr<int>> q;
  q.publish(std::make_unique<int>(7));
  auto item = q.consume();
  assert(item.has_value() && **item == 7);
}



void check_naive_queue_threads() {
  myqueue::NaiveQueue<int> q;
  const int n = 100000;
  std::thread producer([&]() {
    for (int i = 1; i <= n; ++i) { q.publish(int(i)); }
  });

  long long sum = 0;
  int count = 0;
  while (count < n) {
    if (auto item = q.consume()) { sum += *item; ++count; }
  }
  producer.join();

  assert(sum == (long long)n * (n + 1) / 2);
  assert(!q.consume().has_value());
}



void check_spsc_queue() {
  SPSCQueue<int, 4> q;
  assert(!q.consume().has_value());
  for (int i = 0; i < 4; ++i) { assert(q.publish(int(i)) == 0); }
  assert(q.publish(99) == -1);
  for (int i = 0; i < 4; ++i) { assert(q.consume() == i); }
  assert(!q.consume().has_value());
}



void check_spsc_queue_wraparound() {
  SPSCQueue<int, 4> q;
  for (int i = 0; i < 20; ++i) {
    assert(q.publish(int(i)) == 0);
    assert(q.consume() == i);
  }
  assert(!q.consume().has_value());
}



void check_spsc_queue_threads() {
  SPSCQueue<int, 64> q;
  const int n = 100000;
  std::thread producer([&]() {
    for (int i = 0; i < n; ++i) {
      while (q.publish(int(i)) != 0) {}
    }
  });

  int expected = 0;
  while (expected < n) {
    if (auto item = q.consume()) { assert(*item == expected); ++expected; }
  }
  producer.join();

  assert(!q.consume().has_value());
}


void check_spmc_queue() {
  SPMCQueue<int, 4> q;
  assert(!q.consume().has_value());
  for (int i = 0; i < 4; ++i) { assert(q.publish(int(i)) == 0); }
  assert(q.publish(99) == -1);
  for (int i = 0; i < 4; ++i) { assert(q.consume() == i); }
  assert(!q.consume().has_value());
}



void check_spmc_queue_wraparound() {
  SPMCQueue<int, 4> q;
  for (int i = 0; i < 20; ++i) {
    assert(q.publish(int(i)) == 0);
    assert(q.consume() == i);
  }
  assert(!q.consume().has_value());
}



void check_spmc_queue_threads() {
  SPMCQueue<int, 64> q;
  const int n = 100000;
  const int consumers = 4;
  std::vector<std::atomic<int>> seen(n);
  std::atomic<int> total{0};

  std::thread producer([&]() {
    for (int i = 0; i < n; ++i) {
      while (q.publish(int(i)) != 0) {}
    }
  });

  std::vector<std::thread> threads;
  for (int c = 0; c < consumers; ++c) {
    threads.emplace_back([&]() {
      int last = -1;
      while (total.load() < n) {
        if (auto item = q.consume()) {
          assert(*item > last);
          last = *item;
          seen[*item].fetch_add(1);
          total.fetch_add(1);
        }
      }
    });
  }

  producer.join();
  for (auto& t : threads) { t.join(); }

  assert(total.load() == n);
  for (int i = 0; i < n; ++i) { assert(seen[i].load() == 1); }
  assert(!q.consume().has_value());
}


int main() {
  check_naive_queue();
  check_naive_queue_move_only();
  check_naive_queue_threads();
  check_spsc_queue();
  check_spsc_queue_wraparound();
  check_spsc_queue_threads();
  check_spmc_queue();
  check_spmc_queue_wraparound();
  check_spmc_queue_threads();
}
