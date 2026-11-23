#include<iostream>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <cassert>
#include <map>
#include <netdb.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <variant>
#include <vector>
#include <poll.h>
#include <format>

using namespace std;

namespace json {

  struct Value;
  using Array  = std::vector<Value>;
  using Object = std::map<std::string, Value>;

  struct Value {
    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> v;

    template <typename T> const T& as() const { return std::get<T>(v); }
    const Value& operator[](const std::string& k) const { return as<Object>().at(k); }
    bool contains(const std::string& k) const { return std::holds_alternative<Object>(v) && as<Object>().count(k); }
  };

  struct Parser {
    std::string s;
    size_t i = 0;

    [[noreturn]] void fail() { throw std::runtime_error("json: parse error at offset " + std::to_string(i)); }

    char next() {   // skip whitespace, peek
      while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
      if (i >= s.size()) fail();
      return s[i];
    }
    bool eat(char c) { if (next() != c) return false; ++i; return true; }
    void expect(char c) { if (!eat(c)) fail(); }

    Value value() {
      if (eat('{')) {
        Object o;
        if (eat('}')) return {o};
        do { std::string k = str(); expect(':'); o[k] = value(); } while (eat(','));
        expect('}');
        return {o};
      }
      if (eat('[')) {
        Array a;
        if (eat(']')) return {a};
        do a.push_back(value()); while (eat(','));
        expect(']');
        return {a};
      }
      if (next() == '"') return {str()};
      if (s.compare(i, 4, "true") == 0)  { i += 4; return {true}; }
      if (s.compare(i, 5, "false") == 0) { i += 5; return {false}; }
      if (s.compare(i, 4, "null") == 0)  { i += 4; return {nullptr}; }

      char* end;
      double d = std::strtod(s.c_str() + i, &end);
      if (end == s.c_str() + i) fail();
      i = end - s.c_str();
      return {d};
    }

    std::string str() {   // no \u escapes
      expect('"');
      std::string out, from = "ntrbf\"\\/", to = "\n\t\r\b\f\"\\/";
      for (char c; (c = s.at(i++)) != '"'; out += c) {
        if (c != '\\') continue;
        size_t k = from.find(s.at(i++));
        if (k == std::string::npos) fail();
        c = to[k];
      }
      return out;
    }
  };

  inline Value parse(std::string s) {
    Parser p{std::move(s)};
    Value v = p.value();
    if (p.s.find_first_not_of(" \t\r\n", p.i) != std::string::npos) p.fail();
    return v;
  }

}

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


template<typename T>
class MsgQueue<T> {
  virtual void publish(T&&) = 0;
  virtual T&& consume() = 0;
};




int main() {
  Config cfg = Config::from_file("config.json");   // throws on bad config
  // port 8080 

  addrinfo hints{}, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;
  getaddrinfo(nullptr, std::to_string(cfg.port).c_str(), &hints, &res);

  int lds = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (::bind(lds, res->ai_addr, res->ai_addrlen) < 0) { perror("bind"); return 1; }
  if (listen(lds, 1) < 0) { perror("listen"); return 1; }
  freeaddrinfo(res);

  std::cout << format("listening on port {}", cfg.port) << endl;


  // assume one message per connection
  vector<pollfd> fds;
  fds.push_back({lds, POLLIN, 0});
  while (poll(fds.data(), fds.size(), 100) >= 0) {
    assert(fds[0].events & POLLIN != 0); // is this idiomatic?
                                         //
    if (fds[0].revents & POLLIN) {
      int fd = accept(fds[0].fd, nullptr, nullptr);
      if (fd >= 0) {
        fds.push_back({fd, POLLIN, 0});
        printf("client %d connected (%zu total)\n", fd, fds.size() - 1);
      }
    }

    for (size_t i = fds.size() - 1; i >= 1; --i) {
      if (!(fds[i].revents & (POLLIN | POLLHUP | POLLERR))) continue;

      char buf[1024];
      ssize_t n = read(fds[i].fd, buf, sizeof buf);}
    }


  // string message{};
  // while (true) {
  //   char buf[1024];
  //   ssize_t n = read(cfd, buf, sizeof(buf));
  //   if (n <= 0) break;   // 0 = peer closed, <0 = error
  //   std::cout.write(buf, n) << std::flush;
  //
  // }
  // close(cfd);
}
