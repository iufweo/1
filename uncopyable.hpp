#pragma once

class Uncopyable {
 public:
  Uncopyable() = default;

  Uncopyable(const Uncopyable& rhs) = delete;
  Uncopyable(Uncopyable&& rhs) = delete;
  Uncopyable& operator=(const Uncopyable& rhs) = delete;
  Uncopyable& operator=(Uncopyable&& rhs) = delete;
};
