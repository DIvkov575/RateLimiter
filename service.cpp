#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "lib/config.hpp"
#include "lib/network.hpp"
#include "lib/queue.hpp"

using MessageType = std::string;

int main() {
  Config cfg = Config::from_file("config.json");   // throws on bad config

  myqueue::NaiveQueue<MessageType> queue;
  Network<MessageType, myqueue::NaiveQueue<MessageType>> net(cfg, queue);
  net.start_polling();

  while (true) {
    if (auto msg = queue.consume()) {
      std::cout << *msg << std::flush;
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}
