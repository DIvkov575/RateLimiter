#pragma once

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.hpp"

struct RateLimit {
  int requests  = 100;
  int window_ms = 60000;
};

struct Config {
  int port = 8080;
  std::vector<std::string> downstream;
  RateLimit rate_limit;

  static Config from_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open " + path);
    json::Value j = json::parse({std::istreambuf_iterator<char>(f), {}});

    auto num = [](const json::Value& o, const char* k, int def) {
      return o.contains(k) ? static_cast<int>(o[k].as<double>()) : def;
    };

    Config c;
    c.port = num(j, "port", c.port);
    if (j.contains("downstream"))
      for (const auto& d : j["downstream"].as<json::Array>())
        c.downstream.push_back(d.as<std::string>());
    if (j.contains("rate_limit")) {
      const auto& rl = j["rate_limit"];
      c.rate_limit = {num(rl, "requests", c.rate_limit.requests), num(rl, "window_ms", c.rate_limit.window_ms)};
    }
    return c;
  }
};
