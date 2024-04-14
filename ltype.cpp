#include <string>
#include <variant>

#include "ltype.hpp"

std::string literalToString(const Literal& l) {
  std::string s;

  static_assert(2 == std::variant_size_v<Literal>);
  switch (l.index()) {
    case 0:
      s = std::get<std::string>(l);
      break;
    case 1:
      s = std::to_string(std::get<double>(l));
      break;
  }

  return s;
}

Ltype literalToLtype(const Literal& l) {
  Ltype value;

  static_assert(2 == std::variant_size_v<Literal>);
  switch (l.index()) {
    case 0:
      value = Lstring(std::get<std::string>(l));
      break;
    case 1:
      value = std::get<double>(l);
      break;
  }

  return value;
}

std::string valueToString(const Ltype& l) {
  std::string s;
  std::size_t pos;

  static_assert(8 == std::variant_size_v<Ltype>);
  switch (l.index()) {
    case 0:
      s = std::string(std::get<Lstring>(l));
      break;
    case 1:
      s = std::to_string(std::get<double>(l));
      pos = s.find_last_not_of('0');
      if (s[pos] == '.')
        pos--;
      s = s.substr(0, pos + 1);
      break;
    case 2:
      if (std::get<bool>(l) == false)
        s = std::string("false");
      else
        s = std::string("true");
      break;
    case 3:
      s = std::string("nil");
      break;
    case 4:
      [[fallthrough]];
    case 5:
      s = std::string("function");
      break;
    case 6:
      s = std::string("object");
      break;
    case 7:
      s = std::string("class");
      break;
  }

  return s;
}

std::string typeToString(const Ltype& l) {
  static const char* t[] = {"string",   "number",   "bool",   "nil",
                            "function", "function", "object", "class"};
  static_assert(sizeof(t) / sizeof(t[0]) == std::variant_size_v<Ltype>);

  return std::string(t[l.index()]);
}

Lstring::Lstring(std::string&& str) : str(std::move(str)) {}

Lstring::Lstring(const std::string& str) : str(str) {}

Lstring& Lstring::operator=(const Lstring& rhs) {
  str = rhs.str;
  return *this;
}

Lstring& Lstring::operator=(Lstring&& rhs) {
  str = std::move(rhs.str);
  return *this;
}

Lstring::operator std::string() const {
  return str;
}

Lstring Lstring::operator+(const Lstring& rhs) const {
  return Lstring(str + rhs.str);
}

bool Lstring::operator==(const Lstring& rhs) const {
  return str == rhs.str;
}
