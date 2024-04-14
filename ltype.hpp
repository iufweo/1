#pragma once
#include <memory>
#include <string>
#include <variant>

#include "interp_func_fwd.hpp"

// nil
class Lnil {};

class Lstring {
 private:
  std::string str;

 public:
  Lstring() = default;
  Lstring(const Lstring& rhs) = default;
  Lstring(Lstring&& rhs) = default;
  Lstring& operator=(const Lstring& rhs);
  Lstring& operator=(Lstring&& rhs);
  explicit Lstring(const std::string& str);
  explicit Lstring(std::string&& str);
  explicit operator std::string() const;
  Lstring operator+(const Lstring& rhs) const;
  bool operator==(const Lstring& rhs) const;
  ~Lstring() = default;
};

using Literal = std::variant<std::string, double>;
using FunPtr = Func*;
using InstPtr = Linstance*;
using ClassPtr = Lclass*;
using LfunPtr = Lfunc*;
using Ltype = std::
    variant<Lstring, double, bool, Lnil, FunPtr, LfunPtr, InstPtr, ClassPtr>;

std::string literalToString(const Literal& l);
Ltype literalToLtype(const Literal& l);
std::string valueToString(const Ltype& l);
std::string typeToString(const Ltype& l);
