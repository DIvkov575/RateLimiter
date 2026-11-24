#pragma once

#include <atomic>
#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <concepts>

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.hpp"
#include "logger.hpp"
#include "queue.hpp"

template<typename T, std::derived_from<myqueue::Queue<T>> Qtype, typename Log = Logger>
class Network {
  const Config cfg;
  std::unordered_map<int, T> msgs;
  std::vector<pollfd> fds;
  int lds = -1;
  Qtype& message_queue;
  Log logger;

  std::atomic<bool> polling_on{true};
  std::thread poll_thread;

  public:

  Network(const Config& _cfg, Qtype& _msg_queue):
    cfg(_cfg),
    message_queue(_msg_queue)
  {
    if (create_listener() != 0) throw std::runtime_error("cannot create listener");
  }

  ~Network() {
    polling_on = false;
    if (poll_thread.joinable()) poll_thread.join();
    for (pollfd pfd : fds) {
      close(pfd.fd);
    }
  }

  void start_polling() {
    polling_on = false;
    if (poll_thread.joinable()) poll_thread.join();
    polling_on = true;

    poll_thread = std::thread([this]() {
      while (polling_on && poll_once() >= 0) {}
    });
  }


  int create_listener() {
    addrinfo hints{}, *res;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    int rc = getaddrinfo(nullptr, std::to_string(cfg.port).c_str(), &hints, &res);
    if (rc != 0) {
      logger.log("getaddrinfo: {}", gai_strerror(rc));
      return 1;
    }

    lds = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (lds < 0) {
      perror("socket");
      freeaddrinfo(res);
      return 1;
    }
    if (::bind(lds, res->ai_addr, res->ai_addrlen) < 0) {
      perror("bind");
      freeaddrinfo(res);
      return 1;
    }
    freeaddrinfo(res);
    if (listen(lds, 1) < 0) {
      perror("listen");
      return 1;
    }

    fds.push_back({lds, POLLIN, 0});

    logger.log("listening on port {}", cfg.port);
    return 0;
  }

  int poll_once() {
    if (poll(fds.data(), fds.size(), 100) < 0) return -1;
    assert((fds[0].events & POLLIN) != 0);

    if (fds[0].revents & POLLIN) {
      int fd = accept(fds[0].fd, nullptr, nullptr);
      if (fd >= 0) {
        fds.push_back({fd, POLLIN, 0});
        msgs[fd] = T{};
        logger.log("client {} connected ({} total)", fd, fds.size() - 1);
      }
    }

    for (size_t i = fds.size() - 1; i >= 1; --i) {
      int fd = fds[i].fd;
      if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;

      char buf[1024];
      ssize_t n = read(fd, buf, sizeof buf);
      if (n > 0) {
        msgs[fd] += std::string_view(buf, n);
        continue;
      }

      message_queue.publish(std::move(msgs[fd]));
      msgs.erase(fd);
      close(fd);
      fds.erase(fds.begin() + i);
    }
    return 0;
  }
};
