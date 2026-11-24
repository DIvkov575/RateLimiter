#pragma once

#include <cctype>
#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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
