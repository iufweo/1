#pragma once

struct StructUncopyable {
  StructUncopyable() = default;
  //	virtual
  //	~StructUncopyable() = default;

  StructUncopyable(const StructUncopyable &rhs) = delete;
  StructUncopyable(StructUncopyable &&rhs) = delete;
  StructUncopyable &operator=(const StructUncopyable &rhs) = delete;
  StructUncopyable &operator=(StructUncopyable &&rhs) = delete;
};

class ClassUncopyable {
public:
  ClassUncopyable() = default;
  //	virtual
  //	~ClassUncopyable() = default;

  ClassUncopyable(const ClassUncopyable &rhs) = delete;
  ClassUncopyable(ClassUncopyable &&rhs) = delete;
  ClassUncopyable &operator=(const ClassUncopyable &rhs) = delete;
  ClassUncopyable &operator=(ClassUncopyable &&rhs) = delete;
};
